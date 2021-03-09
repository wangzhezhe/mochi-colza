/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Admin.hpp>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>
#include <ssg.h>
#include <iostream>
#include <vector>
#ifdef COLZA_ENABLE_DRC
extern "C" {
#include <rdmacred.h>
}
#endif

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
static std::string g_ssg_file;

static void parse_command_line(int argc, char** argv);
static uint32_t get_credentials_from_ssg_file();

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

    ssg_init();

    uint32_t cookie = get_credentials_from_ssg_file();

    hg_init_info hii;
    memset(&hii, 0, sizeof(hii));
    std::string cookie_str = std::to_string(cookie);
    if(cookie != 0)
        hii.na_init_info.auth_key = cookie_str.c_str();

    // Initialize the thallium server
    tl::engine engine(g_protocol, THALLIUM_CLIENT_MODE, false, 0, &hii);

    try {

        // Initialize an Admin
        colza::Admin admin(engine);

        if(g_operation == "create") {
            admin.createDistributedPipeline(g_ssg_file, 0, g_pipeline, g_type, g_config, g_library, g_token);
            spdlog::info("Created pipeline {}", g_pipeline);
        } else if(g_operation == "destroy") {
            admin.destroyDistributedPipeline(g_ssg_file, 0, g_pipeline, g_token);
            spdlog::info("Destroyed pipeline {}", g_pipeline);
        } else if(g_operation == "shutdown") {
            admin.shutdownGroup(g_ssg_file);
            spdlog::info("Service shut down");
        }

        // Any of the above functions may throw a colza::Exception
    } catch(const colza::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    engine.finalize();
    ssg_finalize();

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Colza admin", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or protocol", true,"","string");
        TCLAP::ValueArg<std::string> tokenArg("q","token","Security token", false,"","string");
        TCLAP::ValueArg<std::string> typeArg("t","type","Pipeline type", false,"simple_stager","string");
        TCLAP::ValueArg<std::string> libraryArg("l","library","Dynamic library containing pipeline definition",false,"","string");
        TCLAP::ValueArg<std::string> pipelineArg("n","pipeline","Pipeline name", false,"","string");
        TCLAP::ValueArg<std::string> configArg("c","config","Pipeline configuration", false,"","string");
        TCLAP::ValueArg<std::string> logLevel("v","verbose",
            "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        TCLAP::ValueArg<std::string> ssgFileArg("s","ssg-file","SSG file name", true, "","string");
        std::vector<std::string> options = { "create", "destroy", "shutdown" };
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
        cmd.add(ssgFileArg);
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
        g_ssg_file = ssgFileArg.getValue();
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

uint32_t get_credentials_from_ssg_file() {
    uint32_t cookie = 0;
#ifdef COLZA_ENABLE_DRC
    int num_addrs = 1;
    ssg_group_id_t gid;
    int ret = ssg_group_id_load(g_ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS) {
        spdlog::critical("Could not load group id from file");
        exit(-1);
    }
    int64_t credential_id = ssg_group_id_get_cred(gid);
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
