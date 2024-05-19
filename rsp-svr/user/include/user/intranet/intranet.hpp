
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <typeinfo>

#include <boost/asio.hpp>
// #include <google/protobuf/port_def.inc>

#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"
#include "rsplib/thread/thread_pool.hpp"
#include "user/intranet/message_dispatcher.hpp"

namespace rsp {
namespace user {
namespace lg = rsp::libs::logger;
namespace th = rsp::libs::thread;
namespace br = rsp::libs::broker;
namespace ba = boost::asio;

// TODO(@nolleh)
#define PROTOBUF_NAMESPACE_ID google::protobuf
using Message = PROTOBUF_NAMESPACE_ID::Message;

class room_sender {
 public:
  room_sender() : logger_(lg::logger()), threads_(1), dispatcher_(this) {
    room_sender_ =
        br::broker::s_create_publisher(CastType::kUniCast, "room", 1);
  }

  ~room_sender() { stop(); }

  void start() {
    // sender live longer threads
    room_sender_->start();
    threads_.start();

    // co_spawn(
    //     threads_.get_executor(), [self = this] { return self->start_recv();
    //     }, ba::detached);
  }

  void stop() {
    stop_ = true;
    room_sender_->stop();
    threads_.join();
  }

  ba::awaitable<void> start_recv() {
    logger_.info() << "start room receiving" << lg::L_endl;
    while (!stop_.load()) {
      // TODO(@nolleh) awaitable.
      // auto buffer = co_await room_sender_->recv("topic");
      auto buffer = room_sender_->recv("topic").get();

      namespace msg = rsp::libs::message;
      auto destructed = msg::serializer::destruct_buffer(buffer);
      dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    }
    logger_.info() << "stop room receiving" << lg::L_endl;
    co_return;
  }

  ba::awaitable<void> recv() {
    // TODO(@nolleh) improve with executors
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0443r14.html
    auto buffer = room_sender_->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    co_return;
  }

  template <typename T>
  void send_request(
      MessageType type, const T& req,
      std::function<void(const std::shared_ptr<Message>)> handler) {
    requests_[req.request_id()] =
        std::function<void(const std::shared_ptr<Message>)>(handler);
    namespace msg = rsp::libs::message;
    auto buffer = msg::serializer::serialize(type, req);
    room_sender_->send("topic", buffer);

    logger_.trace() << "send 2 room, requestId:" << req.request_id()
                    << ", type: " << typeid(req).name() << lg::L_endl;

    // hum......
    // TODO(@nolleh) improve with executors
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0443r14.html
    // https://stackoverflow.com/questions/63360248/where-is-stdfuturethen-and-the-concurrency-ts
    co_spawn(
        threads_.get_executor(), [self = this] { return self->recv(); },
        ba::detached);
  }

  template <typename T>
  void send_notification(MessageType type, const T& req) const {
    namespace msg = rsp::libs::message;
    auto buffer = msg::serializer::serialize(type, req);
    room_sender_->send("topic", buffer);
  }

  void on_recv(const Ping& ping) const {
    logger_.debug() << "received ping" << lg::L_endl;
  }
  void on_recv(const Pong& pong) const {
    logger_.debug() << "received pong" << lg::L_endl;
  }
  template <typename T>
  void on_recv(const T& msg) const {
    logger_.trace() << "received message requestId: " << msg.request_id() << ","
                    << typeid(msg).name() << lg::L_endl;
    send_to_waiter(msg);
  }

 private:
  template <typename T>
  void send_to_waiter(const T& t) const {
    auto iter = requests_.find(t.request_id());
    if (requests_.end() == iter) {
      return;
    }

    auto handler = iter->second;
    handler(std::make_shared<T>(t));
  }

  lg::s_logger& logger_;
  th::thread_pool threads_;

  message_dispatcher<room_sender> dispatcher_;
  std::shared_ptr<br::broker_interface> room_sender_;
  std::atomic<bool> stop_;

  // TODO(@nolleh) timeout
  mutable std::map<int32_t, std::function<void(const std::shared_ptr<Message>)>>
      requests_;
};

class intranet {
 public:
  static intranet& instance();

  void start() { room_sender_.start(); }
  void stop() { room_sender_.stop(); }

  room_sender& room() const { return room_sender_; }

 private:
  intranet(const intranet&) = delete;
  intranet& operator=(const intranet&) = delete;

  static std::once_flag s_flag;
  static std::unique_ptr<intranet> s_instance;

  intranet() : logger_(lg::logger()) {}
  lg::s_logger& logger_;
  mutable room_sender room_sender_;
};

}  // namespace user
}  // namespace rsp
