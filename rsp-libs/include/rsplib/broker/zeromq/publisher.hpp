
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <future>
#include <memory>
#include <string>
#include <utility>
#include <zmq.hpp>

#include "rsplib/broker/address.hpp"
#include "rsplib/broker/cast_type.hpp"

namespace rsp {
namespace libs {
namespace broker {

class publisher {
 public:
  publisher(CastType type, const std::string& service_name,
            const uint8_t context)
      : context_(context) {
    // address_ = generate_address(type, service_name, context);
    switch (type) {
      case CastType::kBroadCast:
        create_broadcast(context);
      case CastType::kUniCast:
        create_unicast(context);
      case CastType::kAnyCast:
        create_anycast(context);
    }
  }

  ~publisher() {}

  publisher(publisher&& r) : socket_(std::move(r.socket_)) {}

  void add_topic(const std::string& topic) {}

  void sub_topic(const std::string& topic) {}

  void send(const std::string& topic, std::ostream& os) {}

  std::future<std::istream> recv(const std::string& topic) {
    std::runtime_error("not implemented");
  }

 private:
  void create_broadcast(const uint8_t context) {
    zmq::socket_t socket(context_, zmq::socket_type::pub);
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_anycast(const uint8_t context) {
    zmq::socket_t socket(context_, zmq::socket_type::push);
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_unicast(const uint8_t context) {
    zmq::socket_t socket(context_, zmq::socket_type::rep);
    // socket_.set(zmq::sockopt::linger, 1);
  }

  address address_;
  zmq::context_t context_;
  zmq::socket_t socket_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
