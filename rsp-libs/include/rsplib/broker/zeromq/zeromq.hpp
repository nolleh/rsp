/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <memory>
#include <string>
#include <utility>
#include <zmq.hpp>

#include "rsplib/broker/broker_interface.hpp"
#include "rsplib/broker/zeromq/publisher.hpp"

namespace rsp {
namespace libs {
namespace broker {

template <typename T>
class zeromq : public broker_interface {
 public:
  static std::shared_ptr<zeromq> s_create_publisher(
      CastType type, const std::string& service_name, const uint8_t context) {
    return std::shared_ptr<zeromq>(new zeromq({type, service_name, context}));
  }
  static std::shared_ptr<zeromq> s_create_subscriber(
      CastType type, const std::string& service_name, const uint8_t context,
      const std::string& topic /* passed to filter */) {
    throw std::runtime_error("not implemented");
  }

  void add_topic(const std::string& topic) override { impl_.add_topic(topic); }

  void sub_topic(const std::string& topic) override { impl_.sub_topic(topic); }

  void send(const std::string& topic, std::ostream& os) override {
    impl_.send(topic, os);
  }

  std::future<std::istream> recv(const std::string& topic) override {
    return impl_.recv(topic);
  }

 private:
  explicit zeromq(T&& t) : impl_(std::move(t)) {}
  T impl_;
};

}  // namespace broker
}  // namespace libs
}  // namespace rsp
