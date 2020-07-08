#ifndef __COLZA_REQUEST_HPP
#define __COLZA_REQUEST_HPP

#include <thallium.hpp>

namespace colza {

namespace tl = thallium;

class request {
 public:
  std::shared_ptr<tl::eventual<void>> m_eventual;

  request() : m_eventual(std::make_shared<tl::eventual<void>>()) {}
  ~request() = default;
  request(const request& other) = default;  // copy constructor
  request& operator=(const request& other) = default; // copy assignment

  request(request&& other) = default;             // move constructor
  request& operator=(request&& other) = default;  // move assignment

  int wait();
};

}  // namespace colza

#endif
