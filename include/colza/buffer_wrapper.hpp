#ifndef __COLZA_BUFFER_WRAPPER_HPP
#define __COLZA_BUFFER_WRAPPER_HPP

#include <exception>

namespace colza {

class buffer_wrapper {

    public:

    buffer_wrapper(const char* buffer, size_t size)
    : m_buffer(buffer)
    , m_size(size) {}

    buffer_wrapper(const buffer_wrapper&) = default;

    buffer_wrapper(buffer_wrapper&&) = default;

    buffer_wrapper& operator=(const buffer_wrapper&) = default;

    buffer_wrapper& operator=(buffer_wrapper&&) = default;

    ~buffer_wrapper() = default;

    template<typename A>
    void save(A& ar) const {
        ar.write(&m_size, sizeof(m_size));
        ar.write(m_buffer, m_size);
    }

    template<typename A>
    void load(A& ar) {
        throw std::runtime_error("buffer_wrapper class cannot be loaded");
    }

    private:

    typedef std::vector<char>::size_type size_type;

    const char*  m_buffer = nullptr;
    size_type    m_size   = 0;

};

}

#endif
