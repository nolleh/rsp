
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include <memory>
#include <string>

#include "rsplib/broker/broker.hpp"

#define ZEROMQ

#ifdef ZEROMQ
#include "rsplib/broker/zeromq/zeromq.hpp"
#endif

namespace rsp {
namespace libs {
namespace broker {

std::shared_ptr<broker_interface> broker::s_create_publisher(
    CastType type, const std::string& service_name, const uint8_t context) {
  std::shared_ptr<broker_interface> pub = nullptr;

#ifdef ZEROMQ
  pub = zeromq<publisher>::s_create_publisher(type, service_name, context);

#endif
  return pub;
}

std::shared_ptr<broker_interface> broker::s_create_subscriber(
    CastType type, const std::string& service_name, const uint8_t context,
    const std::string& topic) {
  std::shared_ptr<broker_interface> sub = nullptr;
#ifdef ZEROMQ
  sub = zeromq<subscriber>::s_create_subscriber(type, service_name, context,
                                                topic);

#endif
  return sub;
}

}  // namespace broker
}  // namespace libs
}  // namespace rsp
