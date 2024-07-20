/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "user/intranet/message_dispatcher.hpp"
#include "user/intranet/types.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;

class user_receiver {
 public:
  user_receiver() : logger_(lg::logger()), dispatcher_(this) {
    subscriber_ = br::broker::s_create_subscriber(CastType::kAnyCast, "room", 1,
                                                  "tcp://*:5558", "topic");
  }

  Address addr() const {
    // TODO(@nolleh) how to get my ip address?
    return "tcp://127.0.0.1:5558";
  }

  void start() {
    subscriber_->start();
    start_recv();
  }

  void stop() { subscriber_->stop(); }

  template <typename T>
  void on_recv(const T& t) {
    pass_to_session(t);
  }

 private:
  void start_recv() {
    auto buffer = subscriber_->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
    start_recv();
  }

  template <typename Message>
  void pass_to_session(const Message& message) {
    auto s = session::session_manager::instance().find_session(message.uid());
    s->on_recv(message);
  }

  lg::s_logger& logger_;
  message_dispatcher<user_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> subscriber_;
};

template <>
inline void user_receiver::on_recv(const Ping&) {}

template <>
inline void user_receiver::on_recv(const Pong&) {}
}  // namespace user
}  // namespace rsp
