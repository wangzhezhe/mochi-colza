#ifndef __COLZA_CONTROLLER_HPP
#define __COLZA_CONTROLLER_HPP

#ifndef COLZA_DEFAULT_GROUP_NAME
#define COLZA_DEFAULT_GROUP_NAME "colza"
#endif

#include <thallium.hpp>

namespace colza {

namespace tl = thallium;

class controller {

    public:

    controller(tl::engine& engine);
    ~controller();
    controller(const controller&) = delete;
    controller(controller&&) = default;
    controller& operator=(const controller&) = delete;
    controller& operator=(controller&&) = default;

    private:

    tl::engine& m_engine;
};

}

#endif
