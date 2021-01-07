/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Admin.hpp>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <vector>

namespace tl = thallium;

static std::string g_address;
static std::string g_protocol;
static std::string g_pipeline;
static std::string g_type;
static std::string g_config;
static std::string g_library;
static std::string g_token;
static std::string g_operation;
static std::string g_log_level = "info";

static void parse_command_line(int argc, char** argv);

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    // Initialize the thallium server
    tl::engine engine(g_protocol, THALLIUM_CLIENT_MODE);

    try {

        // Initialize an Admin
        colza::Admin admin(engine);

        if(g_operation == "create") {
            admin.createPipeline(g_address, 0, g_pipeline, g_type, g_config, g_library, g_token);
            spdlog::info("Created pipeline {}", g_pipeline);
        } else if(g_operation == "destroy") {
            admin.destroyPipeline(g_address, 0, g_pipeline, g_token);
            spdlog::info("Destroyed pipeline {}", g_pipeline);
        }

        // Any of the above functions may throw a colza::Exception
    } catch(const colza::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Colza admin", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or server", true,"","string");
        TCLAP::ValueArg<std::string> tokenArg("q","token","Security token", false,"","string");
        TCLAP::ValueArg<std::string> libraryArg("l","library","Dynamic library containing pipeline definition",false,"","string");
        TCLAP::ValueArg<std::string> typeArg("t","type","Pipeline type", false,"simple_stager","string");
        TCLAP::ValueArg<std::string> pipelineArg("n","pipeline","Pipeline name", false,"","string");
        TCLAP::ValueArg<std::string> configArg("c","config","Pipeline configuration", false,"","string");
        TCLAP::ValueArg<std::string> logLevel("v","verbose",
            "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        std::vector<std::string> options = { "create", "destroy" };
        TCLAP::ValuesConstraint<std::string> allowedOptions(options);
        TCLAP::ValueArg<std::string> operationArg("x","exec","Operation to execute",true,"create",&allowedOptions);
        cmd.add(addressArg);
        cmd.add(typeArg);
        cmd.add(tokenArg);
        cmd.add(configArg);
        cmd.add(pipelineArg);
        cmd.add(logLevel);
        cmd.add(libraryArg);
        cmd.add(operationArg);
        cmd.parse(argc, argv);
        g_address = addressArg.getValue();
        g_library = libraryArg.getValue();
        g_token = tokenArg.getValue();
        g_config = configArg.getValue();
        g_type = typeArg.getValue();
        g_pipeline = pipelineArg.getValue();
        g_operation = operationArg.getValue();
        g_log_level = logLevel.getValue();
        g_protocol = g_address.substr(0, g_address.find(":"));
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}
