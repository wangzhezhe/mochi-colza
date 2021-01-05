/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PIPELINE_HANDLE_IMPL_H
#define __COLZA_PIPELINE_HANDLE_IMPL_H

#include <string>
#include <memory>

namespace colza {

class ClientImpl;

class PipelineHandleImpl {

    public:

    std::string                 m_name;
    std::shared_ptr<ClientImpl> m_client;
    tl::provider_handle         m_ph;

    PipelineHandleImpl() = default;

    PipelineHandleImpl(const std::shared_ptr<ClientImpl>& client,
                       tl::provider_handle&& ph,
                       const std::string& pipeline_name)
    : m_name(pipeline_name)
    , m_client(client)
    , m_ph(std::move(ph)) {}
};

}

#endif
