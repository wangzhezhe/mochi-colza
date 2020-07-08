#ifndef __COLZA_REQUEST_HPP
#define __COLZA_REQUEST_HPP

#include <thallium.hpp>

namespace colza {

class communicator;

namespace tl = thallium;

class request {
  friend class communicator;
 private:
  request(std::shared_ptr<tl::eventual<void>> e)
  : m_eventual(std::move(e)) {}

 public:
  std::shared_ptr<tl::eventual<void>> m_eventual;

  request() = default;
  ~request() = default;
  request(const request& other) = default;  // copy constructor
  request& operator=(const request& other) = default; // copy assignment

  request(request&& other) = default;             // move constructor
  request& operator=(request&& other) = default;  // move assignment

  operator bool() const {
    return static_cast<bool>(m_eventual);
  }

};

}  // namespace colza

#endif
