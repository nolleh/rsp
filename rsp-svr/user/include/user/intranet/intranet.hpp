
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>

#include <boost/asio.hpp>

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

class room_sender {
 public:
  room_sender() : logger_(lg::logger()), threads_(1), dispatcher_(this) {
    room_sender_ =
        br::broker::s_create_publisher(CastType::kAnyCast, "room", 1);
  }

  ~room_sender() { stop(); }

  void start() {
    room_sender_->start();

    // sender live longer threads
    co_spawn(
        threads_.get_executor(), [self = this] { return self->recv(); },
        ba::detached);
  }

  void stop() {
    threads_.join();
    room_sender_->stop();
  }

  ba::awaitable<void> recv() {
    // TODO(@nolleh) awaitable.
    // auto buffer = co_await room_sender_->recv("topic");
    auto buffer = room_sender_->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    co_return;
  }

  template <typename T>
  void send_request(MessageType type, T req) {
    requests_.insert(req.request_id(), req);
    namespace msg = rsp::libs::message;
    auto buffer = msg::serializer::serialize(type, req);
    room_sender_->send("topic", buffer);
  }

  template <typename T>
  void send_notification(MessageType type, T req) {
    namespace msg = rsp::libs::message;
    auto buffer = msg::serializer::serialize(type, req);
    room_sender_->send("topic", buffer);
  }

  void on_recv(const Ping& ping) {
    logger_.debug() << "received ping" << lg::L_endl;
  }
  void on_recv(const Pong& pong) {
    logger_.debug() << "received pong" << lg::L_endl;
  }
  void on_recv(const ResCreateRoom& create_room) {
    send_to_waiter(create_room);
  }
  void on_recv(const ResJoinRoom& join_room) { send_to_waiter(join_room); }

 private:
  template <typename T>
  void send_to_waiter(const T& t) {
    auto iter = requests_.find(t.request_id());
    if (requests_.end() == iter) {
      return;
    }

    auto handler = iter->second;
    const auto message_handler = static_cast<void(const T*)>(handler);
    message_handler(&t);
  }

  lg::s_logger& logger_;
  th::thread_pool threads_;

  message_dispatcher<room_sender> dispatcher_;
  std::shared_ptr<br::broker_interface> room_sender_;

  // TODO(@nolleh) timeout
  std::map<uint32_t, void(const void*)> requests_;
};

class intranet {
 public:
  intranet() : logger_(lg::logger()) {}

  void start() { room_sender_.start(); }

 private:
  lg::s_logger& logger_;
  room_sender room_sender_;
};

}  // namespace user
}  // namespace rsp
