#include "colza/communicator.hpp"

namespace colza {

communicator* communicator::create(controller* owner) {
    return new communicator(owner);
}

}
