#include "colza/communicator.hpp"

namespace colza {

size_t communicator::size() const {
    return m_size;
}

size_t communicator::rank() const {
    return m_rank;
}

}
