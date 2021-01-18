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
#include <ssg.h>
#include <mpi.h>

namespace tl = thallium;

static std::string g_address;
static std::string g_pipeline;
static std::string g_log_level = "info";
static std::string g_ssg_file;

static void parse_command_line(int argc, char** argv);

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    MPI_Init(&argc, &argv);

    ssg_init();

    colza::MPIClientCommunicator comm(MPI_COMM_WORLD);
    int rank = comm.rank();

    // Initialize the thallium server
    tl::engine engine(g_address, THALLIUM_SERVER_MODE);

    try {

        // Initialize a Client
        colza::Client client(engine);

        // Open distributed pipeline from provider 0
        colza::DistributedPipelineHandle pipeline =
            client.makeDistributedPipelineHandle(
                &comm, g_ssg_file, 0, g_pipeline);

        // start iteration
        pipeline.start(42);

        // create some data
        std::vector<double> mydata(32*54);
        for(unsigned i=0; i < 32; i++)
            for(unsigned j=0; j < 54; j++)
                mydata[i*54+j] = i*j;
        std::vector<size_t> dimensions = { 32, 54 };
        std::vector<int64_t> offsets = { 0, 0 };
        auto type = colza::Type::FLOAT64;

        spdlog::trace("Calling stage");

        // stage the data at iteration 42
        int32_t result;
        pipeline.stage("mydata", 42, rank,
                       dimensions, offsets,
                       type, mydata.data(),
                       &result);

        spdlog::trace("Calling execute");

        // execute the pipeline
        pipeline.execute(42);

        spdlog::trace("Calling cleanup");

        // cleanup the pipeline
        pipeline.cleanup(42);

        spdlog::trace("Done");

    } catch(const colza::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }
    spdlog::trace("Finalizing engine");

    engine.finalize();

    spdlog::trace("Finalizing SSG");
    ssg_finalize();

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
        cmd.add(addressArg);
        cmd.add(pipelineArg);
        cmd.add(logLevel);
        cmd.add(ssgFileArg);
        cmd.parse(argc, argv);
        g_address = addressArg.getValue();
        g_pipeline = pipelineArg.getValue();
        g_log_level = logLevel.getValue();
        g_ssg_file = ssgFileArg.getValue();
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}
