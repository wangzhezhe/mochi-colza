/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <colza/Client.hpp>
#include <colza/Admin.hpp>

extern thallium::engine engine;
extern std::string pipeline_type;

class ClientTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( ClientTest );
    CPPUNIT_TEST( testOpenPipeline );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* pipeline_config = "{ \"path\" : \"mydb\" }";
    UUID pipeline_id;

    public:

    void setUp() {
        colza::Admin admin(engine);
        std::string addr = engine.self();
        pipeline_id = admin.createPipeline(addr, 0, pipeline_type, pipeline_config);
    }

    void tearDown() {
        colza::Admin admin(engine);
        std::string addr = engine.self();
        admin.destroyPipeline(addr, 0, pipeline_id);
    }

    void testOpenPipeline() {
        colza::Client client(engine);
        std::string addr = engine.self();
        
        Pipeline my_pipeline = client.open(addr, 0, pipeline_id);
        CPPUNIT_ASSERT_MESSAGE(
                "Pipeline should be valid",
                static_cast<bool>(my_pipeline));

        auto bad_id = UUID::generate();
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.open should fail on non-existing pipeline",
                client.open(addr, 0, bad_id);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION( ClientTest );
