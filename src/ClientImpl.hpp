/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_CLIENT_IMPL_H
#define __COLZA_CLIENT_IMPL_H

#include <thallium.hpp>
#include <thallium/serialization/stl/unordered_set.hpp>
#include <thallium/serialization/stl/unordered_map.hpp>
#include <thallium/serialization/stl/string.hpp>

namespace colza {

namespace tl = thallium;

class ClientImpl {

    public:

    tl::engine           m_engine;
    tl::remote_procedure m_check_pipeline;
    tl::remote_procedure m_start;
    tl::remote_procedure m_stage;
    tl::remote_procedure m_execute;
    tl::remote_procedure m_cleanup;
    tl::remote_procedure m_abort;

    ClientImpl(const tl::engine& engine)
    : m_engine(engine)
    , m_check_pipeline(m_engine.define("colza_check_pipeline"))
    , m_start(m_engine.define("colza_start"))
    , m_stage(m_engine.define("colza_stage"))
    , m_execute(m_engine.define("colza_execute"))
    , m_cleanup(m_engine.define("colza_cleanup"))
    , m_abort(m_engine.define("colza_abort"))
    {}

    ClientImpl(margo_instance_id mid)
    : ClientImpl(tl::engine(mid)) {}

    ~ClientImpl() {}
};

}

#endif
