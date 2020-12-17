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

        colza::UUID pipeline_id;
        // Create a valid Pipeline
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.createPipeline should return a valid Pipeline",
                pipeline_id = admin.createPipeline(addr, 0, pipeline_type, pipeline_config));

        // Create a Pipeline with a wrong backend type
        colza::UUID bad_id;
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.createPipeline should throw an exception (wrong backend)",
                bad_id = admin.createPipeline(addr, 0, "blabla", pipeline_config),
                colza::Exception);

        // Destroy the Pipeline
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("admin.destroyPipeline should not throw on valid Pipeline",
            admin.destroyPipeline(addr, 0, pipeline_id));

        // Destroy an invalid Pipeline
        CPPUNIT_ASSERT_THROW_MESSAGE("admin.destroyPipeline should throw on invalid Pipeline",
            admin.destroyPipeline(addr, 0, bad_id),
            colza::Exception);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION( AdminTest );
