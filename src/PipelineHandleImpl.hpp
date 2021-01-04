/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PIPELINE_HANDLE_IMPL_H
#define __COLZA_PIPELINE_HANDLE_IMPL_H

#include <colza/UUID.hpp>

namespace colza {

class PipelineHandleImpl {

    public:

    UUID                        m_pipeline_id;
    std::shared_ptr<ClientImpl> m_client;
    tl::provider_handle         m_ph;

    PipelineHandleImpl() = default;

    PipelineHandleImpl(const std::shared_ptr<ClientImpl>& client,
                       tl::provider_handle&& ph,
                       const UUID& pipeline_id)
    : m_pipeline_id(pipeline_id)
    , m_client(client)
    , m_ph(std::move(ph)) {}
};

}

#endif
