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
    CPPUNIT_TEST( testStage );
    CPPUNIT_TEST( testExecute );
    CPPUNIT_TEST( testCleanup );
    CPPUNIT_TEST_SUITE_END();

    static constexpr const char* pipeline_config = "{}";

    public:

    void setUp() {
        const std::string pipeline_name = "abc";
        colza::Admin admin(engine);
        std::string addr = engine.self();
        admin.createPipeline(addr, 0, pipeline_name, pipeline_type, pipeline_config);
    }

    void tearDown() {
        const std::string pipeline_name = "abc";
        colza::Admin admin(engine);
        std::string addr = engine.self();
        admin.destroyPipeline(addr, 0, pipeline_name);
    }

    void testMakePipelineHandle() {
        const std::string pipeline_name = "abc";
        colza::Client client(engine);
        std::string addr = engine.self();

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for valid id.",
                client.makePipelineHandle(addr, 0, pipeline_name));

        auto bad_name = std::string("bad_pipeline");
        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makePipelineHandle should throw for invalid id.",
                client.makePipelineHandle(addr, 0, bad_name),
                colza::Exception);

        CPPUNIT_ASSERT_THROW_MESSAGE(
                "client.makePipelineHandle should throw for invalid provider.",
                client.makePipelineHandle(addr, 1, pipeline_name),
                std::exception);

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for invalid id when check=false.",
                client.makePipelineHandle(addr, 0, bad_name, false));

        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "client.makePipelineHandle should not throw for invalid provider when check=false.",
                client.makePipelineHandle(addr, 1, pipeline_name, false));
    }

    void testStage() {
        const std::string pipeline_name = "abc";
        colza::Client client(engine);
        std::string addr = engine.self();

        colza::PipelineHandle my_pipeline = client.makePipelineHandle(addr, 0, pipeline_name);

        // create some data
        std::vector<double> mydata(32*54);
        for(unsigned i=0; i < 32; i++)
            for(unsigned j=0; j < 54; j++)
                mydata[i*54+j] = i*j;
        std::vector<size_t> dimensions = { 32, 54 };
        std::vector<int64_t> offsets = { 0, 0 };
        auto type = colza::Type::FLOAT64;

        int32_t result;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.stage() should not throw.",
                my_pipeline.stage("mydata", 42, 0,
                       dimensions, offsets,
                       type, mydata.data(),
                       &result));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "result should be 0.",
                0, result);
    }

    void testExecute() {
        const std::string pipeline_name = "abc";
        colza::Client client(engine);
        std::string addr = engine.self();

        colza::PipelineHandle my_pipeline = client.makePipelineHandle(addr, 0, pipeline_name);

        int32_t result;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.execute() should not throw.",
                my_pipeline.execute(42, &result));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "result should be 0.",
                0, result);
    }

    void testCleanup() {
        const std::string pipeline_name = "abc";
        colza::Client client(engine);
        std::string addr = engine.self();

        colza::PipelineHandle my_pipeline = client.makePipelineHandle(addr, 0, pipeline_name);

        int32_t result;
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
                "my_pipeline.cleanup() should not throw.",
                my_pipeline.cleanup(42, &result));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "result should be 0.",
                0, result);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION( PipelineTest );
