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

class PipelineTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( PipelineTest );
    CPPUNIT_TEST( testMakePipelineHandle );
#if 0
    CPPUNIT_TEST( testSayHello );
    CPPUNIT_TEST( testComputeSum );
#endif
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* pipeline_config = "{ \"path\" : \"mydb\" }";
    colza::UUID pipeline_id;

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

    void testMakePipelineHandle() {
        colza::Client client(engine);
        std::string addr = engine.self();

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for valid id.",
                client.makePipelineHandle(addr, 0, pipeline_id));

        auto bad_id = colza::UUID::generate();
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makePipelineHandle should throw for invalid id.",
                client.makePipelineHandle(addr, 0, bad_id),
                colza::Exception);

        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makePipelineHandle should throw for invalid provider.",
                client.makePipelineHandle(addr, 1, pipeline_id),
                std::exception);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for invalid id when check=false.",
                client.makePipelineHandle(addr, 0, bad_id, false));

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for invalid provider when check=false.",
                client.makePipelineHandle(addr, 1, pipeline_id, false));
    }

#if 0
    void testSayHello() {
        colza::Client client(engine);
        std::string addr = engine.self();

        colza::PipelineHandle my_pipeline = client.makePipelineHandle(addr, 0, pipeline_id);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.sayHello() should not throw.",
                my_pipeline.sayHello());
    }

    void testComputeSum() {
        colza::Client client(engine);
        std::string addr = engine.self();

        colza::PipelineHandle my_pipeline = client.makePipelineHandle(addr, 0, pipeline_id);

        int32_t result = 0;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.computeSum() should not throw.",
                my_pipeline.computeSum(42, 51, &result));

        CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "42 + 51 should be 93",
                93, result);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.computeSum() should not throw when passed NULL.",
                my_pipeline.computeSum(42, 51, nullptr));

        colza::AsyncRequest request;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.computeSum() should not throw when called asynchronously.",
                my_pipeline.computeSum(42, 51, &result, &request));

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "request.wait() should not throw.",
                request.wait());
    }
#endif

};
CPPUNIT_TEST_SUITE_REGISTRATION( PipelineTest );
