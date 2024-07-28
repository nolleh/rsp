/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <typeinfo>
#include <atomic>

#include <boost/asio.hpp>

#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "user/intranet/types.hpp"
#include "user/intranet/unicast_message_dispatcher.hpp"
#include "user/session/session_manager.hpp"

namespace rsp {
namespace user {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace ba = boost::asio;

class user_receiver {
 public:
  user_receiver() : logger_(lg::logger()), threads_(1), dispatcher_(this) {
    // used as notification
    subscriber_ = br::broker::s_create_subscriber(CastType::kSub, "room", 1,
                                                  "tcp://*:5560", "topic");
  }

  Address addr() const {
    // TODO(@nolleh) how to get my ip address?
    return "tcp://127.0.0.1:5560";
  }

  void start() {
    stop_ = false;
    threads_.start();
    subscriber_->start();

    // hum......
    // TODO(@nolleh) improve with executors
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0443r14.html
    // https://stackoverflow.com/questions/63360248/where-is-stdfuturethen-and-the-concurrency-ts
    co_spawn(
        threads_.get_executor(), [self = this] { return self->start_recv(); },
        ba::detached);
  }

  void stop() {
    stop_ = true;
    subscriber_->stop();
    threads_.stop();
  }

  void on_recv(const Ping& ping) {
    logger_.debug() << "received ping" << lg::L_endl;
  }
  void on_recv(const Pong& pong) {
    logger_.debug() << "received pong" << lg::L_endl;
  }

  template <typename T>
  void on_recv(const T& t) {
    logger_.debug() << "on recv on user listening intranet receiver, type:"
                    << typeid(t).name() << lg::L_endl;
    pass_to_session(t);
  }

 private:
  ba::awaitable<void> start_recv() {
    while (!stop_.load()) {
      auto buffer = subscriber_->recv("topic").get();

      namespace msg = rsp::libs::message;
      auto destructed = msg::serializer::destruct_buffer(buffer);
      dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
      logger_.trace() << "dispatch finished. destructed type:"
                      << destructed.type << ", start recv" << lg::L_endl;
    }
    co_return;
  }

  template <typename Message>
  void pass_to_session(const Message& message) {
    auto s = session::session_manager::instance().find_session(message.uid());
    if (nullptr == s) {
      logger_.debug() << "failed to find out session for uid(" << message.uid()
                      << ")" << lg::L_endl;
      return;
    }
    s->on_recv(message);
  }

  lg::s_logger& logger_;
  libs::thread_pool threads_;

  unicast_message_dispatcher<user_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> subscriber_;
  std::atomic<bool> stop_;
};

// template <>
// inline void user_receiver::on_recv(const Ping&) {}
//
// template <>
// inline void user_receiver::on_recv(const Pong&) {}
}  // namespace user
}  // namespace rsp
