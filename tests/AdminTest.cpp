/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <colza/Admin.hpp>
#include <cppunit/extensions/HelperMacros.h>

extern thallium::engine engine;
extern std::string pipeline_type;

class AdminTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( AdminTest );
    CPPUNIT_TEST( testAdminCreatePipeline );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* pipeline_config = "{}";

    public:

    void setUp() {}
    void tearDown() {}

    void testAdminCreatePipeline() {

        colza::Admin admin(engine);
        std::string addr = engine.self();

        std::string pipeline_name = "my_pipeline";
        // Create a valid Pipeline
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.createPipeline should return a valid Pipeline",
                admin.createPipeline(addr, 0, pipeline_name, pipeline_type, pipeline_config));

        // Create a Pipeline with a wrong backend type
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.createPipeline should throw an exception (wrong backend)",
                admin.createPipeline(addr, 0, pipeline_name, "blabla", pipeline_config),
                colza::Exception);

        // Destroy the Pipeline
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.destroyPipeline should not throw on valid Pipeline",
            admin.destroyPipeline(addr, 0, pipeline_name));

        // Destroy an invalid Pipeline
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.destroyPipeline should throw on invalid Pipeline",
            admin.destroyPipeline(addr, 0, "not_my_pipeline"),
            colza::Exception);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION( AdminTest );
