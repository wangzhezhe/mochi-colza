/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Provider.hpp>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>

namespace tl = thallium;
namespace snt = colza;

static std::string g_address = "na+sm";
static unsigned    g_num_providers = 1;
static int         g_num_threads = 0;
static std::string g_log_level = "info";
static bool        g_use_progress_thread = false;

static void parse_command_line(int argc, char** argv);

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    // Initialize SSG
    int ret = ssg_init();
    if(ret != SSG_SUCCESS) {
        std::cerr << "Could not initialize SSG" << std::endl;
    }

    tl::engine engine(g_address, THALLIUM_SERVER_MODE, g_use_progress_thread, g_num_threads);
    engine.enable_remote_shutdown();

    // Create SSG group
    auto self_addr = static_cast<std::string>(engine.self());
    std::vector<const char*> group_addr_str = { self_addr.c_str() };
    ssg_group_config_t group_config = SSG_GROUP_CONFIG_INITIALIZER;
    ssg_group_id_t gid = ssg_group_create(engine.get_margo_instance(),
                                          "mygroup",
                                          group_addr_str.data(),
                                          1, &group_config,
                                          nullptr, nullptr);

    // Create Mona instance
    mona_instance_t mona = mona_init(g_address.c_str(), NA_TRUE, NULL);

    std::vector<snt::Provider> providers;
    for(unsigned i=0 ; i < g_num_providers; i++) {
        providers.emplace_back(engine, gid, mona, i);
    }
    spdlog::info("Server running at address {}", (std::string)engine.self());
    engine.wait_for_finalize();

    mona_finalize(mona);

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Spawns a Colza daemon", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or protocol (e.g. ofi+tcp)", true,"","string");
        TCLAP::ValueArg<unsigned>    providersArg("n", "num-providers", "Number of providers to spawn (default 1)", false, 1, "int");
        TCLAP::SwitchArg progressThreadArg("p","use-progress-thread","Use a Mercury progress thread", cmd, false);
        TCLAP::ValueArg<int> numThreads("t","num-threads", "Number of threads for RPC handlers", false, 0, "int");
        TCLAP::ValueArg<std::string> logLevel("v","verbose", "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        cmd.add(addressArg);
        cmd.add(providersArg);
        cmd.add(numThreads);
        cmd.add(logLevel);
        cmd.parse(argc, argv);
        g_address = addressArg.getValue();
        g_num_providers = providersArg.getValue();
        g_num_threads = numThreads.getValue();
        g_use_progress_thread = progressThreadArg.getValue();
        g_log_level = logLevel.getValue();
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}
