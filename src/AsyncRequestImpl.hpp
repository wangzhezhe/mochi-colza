/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_ASYNC_REQUEST_IMPL_H
#define __COLZA_ASYNC_REQUEST_IMPL_H

#include <functional>
#include <vector>
#include <thallium.hpp>

namespace colza {

namespace tl = thallium;

struct AsyncRequestImpl {

    AsyncRequestImpl(tl::async_response&& async_response) {
        m_async_responses.push_back(std::move(async_response));
    }

    AsyncRequestImpl(std::vector<tl::async_response>&& async_responses)
    : m_async_responses(std::move(async_responses)) {}

    std::vector<tl::async_response>        m_async_responses;
    bool                                   m_waited = false;
    std::function<void(AsyncRequestImpl&)> m_wait_callback;

};

}

#endif
