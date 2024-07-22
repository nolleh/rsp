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
namespace ba = boost::asio;

class user_receiver {
 public:
  user_receiver() : logger_(lg::logger()), threads_(1), dispatcher_(this) {
    subscriber_ = br::broker::s_create_subscriber(CastType::kAnyCast, "room", 1,
                                                  "tcp://*:5560", "topic");
  }

  Address addr() const {
    // TODO(@nolleh) how to get my ip address?
    return "tcp://127.0.0.1:5560";
  }

  void start() {
    // threads_.start();
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
    subscriber_->stop();
    threads_.stop();
  }

  template <typename T>
  void on_recv(const T& t) {
    logger_.debug() << "on recv on user listening intranet receiver"
                    << lg::L_endl;
    pass_to_session(t);
  }

 private:
  ba::awaitable<void> start_recv() {
    auto buffer = subscriber_->recv("topic").get();
    logger_.trace() << "got message from intranet" << lg::L_endl;

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
    co_return;
  }

  template <typename Message>
  void pass_to_session(const Message& message) {
    auto s = session::session_manager::instance().find_session(message.uid());
    if (nullptr == s) {
      return;
    }
    s->on_recv(message);
  }

  lg::s_logger& logger_;
  libs::thread_pool threads_;

  message_dispatcher<user_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> subscriber_;
};

template <>
inline void user_receiver::on_recv(const Ping&) {}

template <>
inline void user_receiver::on_recv(const Pong&) {}
}  // namespace user
}  // namespace rsp
