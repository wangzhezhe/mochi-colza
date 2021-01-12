/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_EXCEPTION_HPP
#define __COLZA_EXCEPTION_HPP

#include <colza/ErrorCodes.hpp>
#include <exception>
#include <string>

namespace colza {

class Exception : public std::exception {

    ErrorCode   m_code;
    std::string m_error;

    public:

    template<typename ... Args>
    Exception(ErrorCode err, Args&&... args)
    : m_code(err)
    , m_error(std::forward<Args>(args)...) {}

    virtual const char* what() const noexcept override {
        return m_error.c_str();
    }

    ErrorCode code() const noexcept {
        return m_code;
    }

};

}

#endif
