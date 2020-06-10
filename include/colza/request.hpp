#ifndef __COLZA_REQUEST_HPP
#define __COLZA_REQUEST_HPP

#include <thallium.hpp>

namespace colza {

namespace tl = thallium;

class request {
 public:
  tl::eventual<void> m_eventual;

  request() = default;
  ~request() = default;

  int wait();
};

}  // namespace colza

#endif
