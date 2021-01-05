/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <fstream>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <spdlog/spdlog.h>

#include <colza/Client.hpp>
#include <colza/Admin.hpp>
#include <colza/Provider.hpp>

namespace tl = thallium;

tl::engine engine;
std::string pipeline_type = "simple_stager";

int main(int argc, char** argv) {

    // Initialize logging
    spdlog::set_level(spdlog::level::trace);

    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of test to run
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( suite );

    std::ofstream xmlOutFile;
    if(argc >= 2) {
        const char* xmlOutFileName = argv[1];
        xmlOutFile.open(xmlOutFileName);
        // Change the default outputter to output XML results into a file
        runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), xmlOutFile));
    } else {
        // Change the default outputter to output XML results into stderr
        runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), std::cerr));
    }
    if(argc >= 3) {
        pipeline_type = argv[2];
    }

    // Initialize the thallium server
    engine = tl::engine("ofi+tcp", THALLIUM_SERVER_MODE);

    // Initialize SSG
    int ret = ssg_init();
    if(ret != SSG_SUCCESS) {
        std::cerr << "Could not initialize SSG" << std::endl;
    }

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
    mona_instance_t mona = mona_init("ofi+tcp", NA_TRUE, NULL);

    // Initialize the Sonata provider
    colza::Provider provider(engine, gid, mona);

    // Run the tests.
    bool wasSucessful = runner.run();

    mona_finalize(mona);

    ssg_group_destroy(gid);
    ret = ssg_finalize();
    if(ret != SSG_SUCCESS) {
        std::cerr << "Could not finalize SSG" << std::endl;
    }

    engine.finalize();

    if(argc >= 2)
       xmlOutFile.close();

    // Return error code 1 if the one of test failed.
    return wasSucessful ? 0 : 1;
}
