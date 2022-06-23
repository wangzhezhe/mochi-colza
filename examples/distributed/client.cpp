/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Client.hpp>
#include <colza/MPIClientCommunicator.hpp>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <ssg.h>
#include <mpi.h>
#ifdef COLZA_ENABLE_DRC
extern "C" {
#include <rdmacred.h>
}
#endif

namespace tl = thallium;

static std::string g_address;
static std::string g_pipeline;
static std::string g_log_level = "info";
static std::string g_ssg_file;
static uint64_t    g_num_iterations = 10;
static uint64_t    g_wait_between_iterations = 2;
static bool        g_no_stage = false;
static bool        g_no_exec = false;

static void parse_command_line(int argc, char** argv);
static uint32_t get_credentials_from_ssg_file();

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    MPI_Init(&argc, &argv);

    colza::MPIClientCommunicator comm(MPI_COMM_WORLD);
    int rank = comm.rank();

    uint32_t cookie = get_credentials_from_ssg_file();

    hg_init_info hii;
    memset(&hii, 0, sizeof(hii));
    std::string cookie_str = std::to_string(cookie);
    if(cookie != 0)
        hii.na_init_info.auth_key = cookie_str.c_str();

    // Initialize the thallium server
    tl::engine engine(g_address, THALLIUM_SERVER_MODE, false, 0, &hii);

    ssg_init();

    double t1, t2;

    try {

        // Initialize a Client
        colza::Client client(engine);

        // Open distributed pipeline from provider 0
        colza::DistributedPipelineHandle pipeline =
            client.makeDistributedPipelineHandle(
                &comm, g_ssg_file, 0, g_pipeline);

        // start iteration
        for(uint64_t iteration = 0; iteration < g_num_iterations; iteration++) {

            spdlog::trace("Calling start({})", iteration);
            t1 = MPI_Wtime();
            pipeline.start(iteration);
            t2 = MPI_Wtime();
            spdlog::trace("Done calling start({}), took {} seconds", iteration, (t2-t1));

            // create some data
            std::vector<double> mydata(32*54);
            for(unsigned i=0; i < 32; i++)
                for(unsigned j=0; j < 54; j++)
                    mydata[i*54+j] = i*j;
            std::vector<size_t> dimensions = { 32, 54 };
            std::vector<int64_t> offsets = { 0, 0 };
            auto type = colza::Type::FLOAT64;

            // stage the data
            if(!g_no_stage) {
                spdlog::trace("Calling stage({})", iteration);
                int32_t result;
                pipeline.stage("mydata", iteration, rank,
                    dimensions, offsets,
                    type, mydata.data(),
                    &result);
                spdlog::trace("Done calling stage({})", iteration);
            }

            // execute the pipeline
            if(!g_no_exec) {
                spdlog::trace("Calling execute({})", iteration);
                pipeline.execute(iteration);
                spdlog::trace("Done calling execute({})", iteration);
            }

            sleep(g_wait_between_iterations);
            // cleanup the pipeline
            spdlog::trace("Calling cleanup({})", iteration);
            pipeline.cleanup(iteration);
            spdlog::trace("Done calling cleanup({})", iteration);

            spdlog::trace("Iteration {} done", iteration);

            sleep(g_wait_between_iterations);
        }
    } catch(const colza::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    spdlog::trace("Finalizing SSG");
    ssg_finalize();

    spdlog::trace("Finalizing engine");
    engine.finalize();

    spdlog::trace("Finalizing MPI");
    MPI_Finalize();

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Colza client", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or protocol", true,"","string");
        TCLAP::ValueArg<std::string> pipelineArg("p","pipeline","Pipeline name", true, "","string");
        TCLAP::ValueArg<std::string> logLevel("v","verbose",
                "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        TCLAP::ValueArg<std::string> ssgFileArg("s","ssg-file","SSG file name", true, "","string");
        TCLAP::ValueArg<unsigned> waitVal("w","wait","Wait time between iterations", false, 2, "int");
        TCLAP::ValueArg<unsigned> numIterations("i","iterations","Number of iterations", false, 10, "int");
        TCLAP::SwitchArg noStage("","no-stage","Do not stage any data", false);
        TCLAP::SwitchArg noExecute("","no-execute","Do not execute the pipeline", false);
        cmd.add(addressArg);
        cmd.add(pipelineArg);
        cmd.add(logLevel);
        cmd.add(ssgFileArg);
        cmd.add(noStage);
        cmd.add(noExecute);
        cmd.add(waitVal);
        cmd.add(numIterations);
        cmd.parse(argc, argv);
        g_address = addressArg.getValue();
        g_pipeline = pipelineArg.getValue();
        g_log_level = logLevel.getValue();
        g_ssg_file = ssgFileArg.getValue();
        g_no_stage = noStage.getValue();
        g_no_exec = noExecute.getValue();
        g_num_iterations = numIterations.getValue();
        g_wait_between_iterations = waitVal.getValue();
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

uint32_t get_credentials_from_ssg_file() {
    uint32_t cookie = 0;
#ifdef COLZA_ENABLE_DRC
    spdlog::trace("Loading SSG group file {} to get DRC information", g_ssg_file);
    int64_t credential_id = -1;
    int ret = ssg_get_group_cred_from_file(g_ssg_file.c_str(),&credential_id);
    if(credential_id == -1)
        return cookie;
    //ssg_group_destroy(gid);

    drc_info_handle_t drc_credential_info;

    ret = drc_access(credential_id, 0, &drc_credential_info);
    if(ret != DRC_SUCCESS) {
        spdlog::critical("drc_access failed (ret = {})", ret);
        exit(-1);
    }

    cookie = drc_get_first_cookie(drc_credential_info);
#endif
    return cookie;
}
