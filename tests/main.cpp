#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <ssg-mpi.h>
#include <ssg.h>

#include <thallium.hpp>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

namespace tl = thallium;

tl::engine* engine;
colza::controller* m_controller;
std::shared_ptr<colza::communicator> m_comm;

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  engine = new tl::engine("ofi+tcp", THALLIUM_SERVER_MODE, true, 4);
  ssg_init();
  engine->push_prefinalize_callback([]() { ssg_finalize(); });
  // Sleeping is needed to make sure other processes have
  // initialized SSG and are ready to respond
  tl::thread::sleep(*engine, 1000);
  // create the communicator and the controller
  m_controller = colza::controller::create(engine, MPI_COMM_WORLD);
  m_comm = m_controller->build_world_communicator();

  // Get the top level suite from the registry
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  // Change the default outputter to a compiler error format outputter
  // runner.setOutputter( new CppUnit::XmlOutputter( &runner.result(), std::cerr
  // ) );
  // Run the tests.
  bool wasSucessful = runner.run();

  // Sleeping is needed to make sure all processes correctly shutdown SSG
  tl::thread::sleep(*engine, 1000);
  engine->finalize();
  delete engine;

  MPI_Finalize();
  // Return error code 1 if the one of test failed.
  return wasSucessful ? 0 : 1;
}
