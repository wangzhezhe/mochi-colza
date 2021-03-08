/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Provider.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <ssg-mpi.h>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>

namespace tl = thallium;

static std::string g_address     = "na+sm";
static int         g_num_threads = 0;
static std::string g_log_level   = "info";
static std::string g_ssg_file    = "";
static std::string g_config_file = "";
static bool        g_join        = false;


static void parse_command_line(int argc, char** argv);

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    // Initialize MPI
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialize SSG
    int ret = ssg_init();
    if(ret != SSG_SUCCESS) {
        std::cerr << "Could not initialize SSG" << std::endl;
        exit(-1);
    }

    tl::engine engine(g_address, THALLIUM_SERVER_MODE, false, 0);
    engine.enable_remote_shutdown();

    ssg_group_id_t gid;
    if(!g_join) {
        // Create SSG group using MPI
        ssg_group_config_t group_config = SSG_GROUP_CONFIG_INITIALIZER;
        group_config.swim_period_length_ms = 1000;
        group_config.swim_suspect_timeout_periods = 3;
        group_config.swim_subgroup_member_count = 1;
        gid = ssg_group_create_mpi(engine.get_margo_instance(),
                                   "mygroup",
                                   MPI_COMM_WORLD,
                                   &group_config,
                                   nullptr, nullptr);
    } else {
        int num_addrs = SSG_ALL_MEMBERS;
        ret = ssg_group_id_load(g_ssg_file.c_str(), &num_addrs, &gid);
        if(ret != SSG_SUCCESS) {
            std::cerr << "Could not load group id from file" << std::endl;
            exit(-1);
        }
        ret = ssg_group_join(engine.get_margo_instance(), gid, nullptr, nullptr);
        if(ret != SSG_SUCCESS) {
            std::cerr << "Could not join SSG group" << std::endl;
            exit(-1);
        }
    }
    engine.push_prefinalize_callback([](){
            spdlog::trace("Finalizing SSG...");
            ssg_finalize();
            spdlog::trace("SSG finalized");
    });

    // Write SSG file
    if(rank == 0 && !g_ssg_file.empty() && !g_join) {
        int ret = ssg_group_id_store(g_ssg_file.c_str(), gid, SSG_ALL_MEMBERS);
        if(ret != SSG_SUCCESS) {
            spdlog::critical("Could not store SSG file {}", g_ssg_file);
            exit(-1);
        }
    }

    // Create Mona instance
    mona_instance_t mona = mona_init_thread(g_address.c_str(), NA_TRUE, NULL, NA_TRUE);

    // Print MoNA address for information
    na_addr_t mona_addr;
    mona_addr_self(mona, &mona_addr);
    std::vector<char> mona_addr_buf(256);
    na_size_t mona_addr_size = 256;
    mona_addr_to_string(mona, mona_addr_buf.data(), &mona_addr_size, mona_addr);
    spdlog::debug("MoNA address is {}", mona_addr_buf.data());
    mona_addr_free(mona, mona_addr);

    // Read config file
    std::string config;
    if(!g_config_file.empty()) {
        std::ifstream t(g_config_file.c_str());
        config = std::string((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());
    }

    // create ESs
    std::vector<tl::managed<tl::xstream>> colza_xstreams;
    tl::managed<tl::pool> managed_colza_pool = tl::pool::create(tl::pool::access::mpmc);
    for(int i=0; i < g_num_threads; i++) {
        colza_xstreams.push_back(
            tl::xstream::create(tl::scheduler::predef::basic_wait, *managed_colza_pool));
    }
    tl::pool colza_pool;
    if(g_num_threads == 0) {
        colza_pool = *managed_colza_pool;
    } else {
        colza_pool = tl::xstream::self().get_main_pools(1)[0];
    }
    engine.push_finalize_callback([&colza_xstreams, &colza_pool](){
        spdlog::trace("Joining Colza xstreams");
        for(auto& es : colza_xstreams) {
            es->make_thread([]() {
                tl::xstream::self().exit();
            }, tl::anonymous());
        }
        usleep(1);
        for(auto& es : colza_xstreams) {
            es->join();
        }
        colza_xstreams.clear();
        spdlog::trace("Colza xstreams joined");
    });

    colza::Provider provider(engine, gid, mona, 0, config, colza_pool);

    spdlog::info("Server running at address {}", (std::string)engine.self());
    engine.wait_for_finalize();

    spdlog::trace("Engine finalized, now finalizing MoNA...");
    mona_finalize(mona);
    spdlog::trace("MoNA finalized");

    MPI_Finalize();

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Spawns a Colza daemon", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or protocol (e.g. ofi+tcp)", true,"","string");
        TCLAP::ValueArg<int> numThreads("t","num-threads", "Number of threads for RPC handlers", false, 0, "int");
        TCLAP::ValueArg<std::string> logLevel("v","verbose",
                "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        TCLAP::ValueArg<std::string> ssgFile("s", "ssg-file", "SSG file name", false, "", "string");
        TCLAP::ValueArg<std::string> configFile("c", "config", "config file name", false, "", "string");
        TCLAP::SwitchArg joinGroup("j","join","Join an existing group rather than create it", false);
        cmd.add(addressArg);
        cmd.add(numThreads);
        cmd.add(logLevel);
        cmd.add(ssgFile);
        cmd.add(configFile);
        cmd.add(joinGroup);
        cmd.parse(argc, argv);
        g_address     = addressArg.getValue();
        g_num_threads = numThreads.getValue();
        g_log_level   = logLevel.getValue();
        g_ssg_file    = ssgFile.getValue();
        g_config_file = configFile.getValue();
        g_join        = joinGroup.getValue();
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}
