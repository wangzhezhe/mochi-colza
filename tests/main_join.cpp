#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <thallium.hpp>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

namespace tl = thallium;

tl::engine* engine;

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  engine = new tl::engine("ofi+tcp", THALLIUM_SERVER_MODE, true, 4);
  // Sleeping is needed to make sure other processes have
  // initialized SSG and are ready to respond
  tl::thread::sleep(*engine, 1000);

  // Get the top level suite from the registry
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  // Change the default outputter to a compiler error format outputter
  runner.setOutputter(CppUnit::CompilerOutputter::defaultOutputter(
      &runner.result(), std::cerr));

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
