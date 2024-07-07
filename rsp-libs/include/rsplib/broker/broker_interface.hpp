/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <future>
#include <iostream>
#include <memory>
#include <string>

#include "rsplib/broker/address.hpp"
#include "rsplib/broker/cast_type.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace broker {

using raw_buffer = rsp::libs::message::raw_buffer;
/**
 * interface that broker implementation should follow.
 *
 *  to prevent things getting too redundant,
 *  interface combine publisher/subscriber.
 *  and there is possibility some broker does not separate publisher, subscriber
 *  not sure this is right choice. can be modified.
 * */
class broker_interface {
 public:
  static std::shared_ptr<broker_interface> s_create_publisher(
      CastType type, const std::string& service_name, const uint8_t context,
      const std::string& addr);

  static std::shared_ptr<broker_interface> s_create_subscriber(
      CastType type, const std::string& service_name, const uint8_t context,
      const std::string& addr, const std::string& topic = nullptr);

  virtual void start() = 0;

  virtual void stop() = 0;

  virtual void add_topic(const std::string& topic) = 0;

  virtual void sub_topic(const std::string& topic) = 0;

  virtual void send(const std::string& topic, const raw_buffer& os) = 0;

  virtual std::future<raw_buffer> recv(const std::string& topic) = 0;

  virtual address get_addr() { return address_; }

 protected:
  address address_;
};

}  // namespace broker
}  // namespace libs
}  // namespace rsp
