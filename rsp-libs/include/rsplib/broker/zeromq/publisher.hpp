
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>
#include <zmq.hpp>

#include "rsplib/broker/address.hpp"
#include "rsplib/broker/cast_type.hpp"

namespace rsp {
namespace libs {
namespace broker {

class publisher {
 public:
  publisher(CastType type, const std::string& service_name,
            const uint8_t context) {
    address_ = generate_address(type, service_name, context);
    switch (type) {
      case CastType::kBroadCast:
        create_broadcast(context);
      case CastType::kUniCast:
        create_unicast(context);
      case CastType::kAnyCast:
        create_anycast(context);
    }
  }

 private:
  void create_broadcast(const uint8_t context) {
    zmq::context_t ctx(context);
    socket_.reset(new zmq::socket_t(ctx, zmq::socket_type::pub));
  }

  void create_anycast(const uint8_t context) {
    zmq::context_t ctx(context);
    socket_.reset(new zmq::socket_t(ctx, zmq::socket_type::push));
  }

  void create_unicast(const uint8_t context) {
    zmq::context_t ctx(context);
    socket_.reset(new zmq::socket_t(ctx, zmq::socket_type::rep));
  }

  address address_;
  std::unique_ptr<zmq::socket_t> socket_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
