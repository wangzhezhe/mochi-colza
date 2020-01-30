#include <thallium.hpp>
#include <ssg.h>
#include <ssg-mpi.h>
#include <colza/controller.hpp>
#include <colza/communicator.hpp>

namespace tl = thallium;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    tl::engine* engine = new tl::engine("ofi+tcp", THALLIUM_SERVER_MODE, true, -1);
    ssg_init();
    engine->push_prefinalize_callback([]() { ssg_finalize(); });
    tl::thread::sleep(*engine, 1000);

    auto controller = colza::controller::create(engine, MPI_COMM_WORLD);
    auto comm       = controller->build_world_communicator();

    tl::thread::sleep(*engine, 1000);

    int colza_rank = comm->rank();
    int colza_size = comm->size();
    std::vector<char> v(256);
    if(colza_rank == 0) {
        for(unsigned i = 0; i < 256; i++) {
            v[i] = 'A'+(i%26);
        }
        tl::thread::sleep(*engine, 1000);
        std::cerr << "Before send" << std::endl;
        int ret = comm->send((const void*)v.data(), 256, 1, 1234);
        std::cerr << "After send" << std::endl;
    } else {
        std::cerr << "Before receive" << std::endl;
        int ret = comm->recv((void*)v.data(), 256, 0, 1234);
        std::cerr << "After receive" << std::endl;
    }

    engine->finalize();
    delete engine;

    MPI_Finalize();
    return 0;
}
