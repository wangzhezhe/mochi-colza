/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_ADMIN_IMPL_H
#define __COLZA_ADMIN_IMPL_H

#include <thallium.hpp>

namespace colza {

namespace tl = thallium;

class AdminImpl {

    public:

    tl::engine           m_engine;
    tl::remote_procedure m_create_pipeline;
    tl::remote_procedure m_destroy_pipeline;

    AdminImpl(const tl::engine& engine)
    : m_engine(engine)
    , m_create_pipeline(m_engine.define("colza_create_pipeline"))
    , m_destroy_pipeline(m_engine.define("colza_destroy_pipeline"))
    {}

    AdminImpl(margo_instance_id mid)
    : AdminImpl(tl::engine(mid)) {
    }

    ~AdminImpl() {}
};

}

#endif
