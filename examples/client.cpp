/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Client.hpp>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>
#include <iostream>

namespace tl = thallium;

static std::string g_address;
static std::string g_protocol;
static std::string g_pipeline;
static unsigned    g_provider_id;
static std::string g_log_level = "info";

static void parse_command_line(int argc, char** argv);

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    // Initialize the thallium server
    tl::engine engine(g_protocol, THALLIUM_CLIENT_MODE);

    try {

        // Initialize a Client
        colza::Client client(engine);

        // Open the Database "mydatabase" from provider 0
        colza::PipelineHandle pipeline =
            client.makePipelineHandle(g_address, g_provider_id, g_pipeline);

        // create some data
        std::vector<double> mydata(32*54);
        for(unsigned i=0; i < 32; i++)
            for(unsigned j=0; j < 54; j++)
                mydata[i*54+j] = i*j;
        std::vector<size_t> dimensions = { 32, 54 };
        std::vector<int64_t> offsets = { 0, 0 };
        auto type = colza::Type::FLOAT64;

        // stage the data at iteration 42
        int32_t result;
        pipeline.stage("mydata", 42, 0,
                       dimensions, offsets,
                       type, mydata.data(),
                       &result);

        // execute the pipeline
        pipeline.execute(42);

        // cleanup the pipeline
        pipeline.cleanup(42);

    } catch(const colza::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Colza client", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or server", true,"","string");
        TCLAP::ValueArg<unsigned>    providerArg("p", "provider", "Provider id to contact (default 0)", false, 0, "int");
        TCLAP::ValueArg<std::string> pipelineArg("n","pipeline","Pipeline name", true, "","string");
        TCLAP::ValueArg<std::string> logLevel("v","verbose", "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        cmd.add(addressArg);
        cmd.add(providerArg);
        cmd.add(pipelineArg);
        cmd.add(logLevel);
        cmd.parse(argc, argv);
        g_address = addressArg.getValue();
        g_provider_id = providerArg.getValue();
        g_pipeline = pipelineArg.getValue();
        g_log_level = logLevel.getValue();
        g_protocol = g_address.substr(0, g_address.find(":"));
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}
