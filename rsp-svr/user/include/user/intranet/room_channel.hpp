/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>

#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"
#include "rsplib/broker/zeromq/routed_channel.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/pending_requests.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"
#include "user/intranet/message_dispatcher.hpp"
#include "user/session/session_manager.hpp"

namespace rsp {
namespace user {

#define PROTOBUF_NAMESPACE_ID google::protobuf
using Message = PROTOBUF_NAMESPACE_ID::Message;

namespace lg = rsp::libs::logger;
namespace libs = rsp::libs;
namespace br = rsp::libs::broker;

class room_channel {
 public:
  room_channel()
      : logger_(lg::logger()),
        dispatcher_(this),
        channel_("tcp://127.0.0.1:5559", "user-server-1") {}

  ~room_channel() { stop(); }

  void start() {
    channel_.start([this](libs::message::raw_buffer buffer) {
      auto destructed = libs::message::serializer::destruct_buffer(buffer);
      dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    });
    requests_.start();
  }

  void stop() {
    requests_.stop();
    channel_.stop();
    requests_.clear();
  }

  template <typename T>
  uint64_t send_request(
      MessageType type, T request,
      std::function<void(const std::shared_ptr<Message>)> handler,
      std::function<void()> on_timeout) {
    const auto request_id = next_request_id_.fetch_add(1);
    request.set_request_id(request_id);
    requests_.add(
        request_id, std::move(handler),
        [on_timeout = std::move(on_timeout)] {
          try {
            on_timeout();
          } catch (const std::exception& error) {
            lg::logger().error() << "unable to dispatch room timeout: "
                                 << error.what() << lg::L_endl;
          }
        },
        std::chrono::steady_clock::now() + std::chrono::seconds(5));

    channel_.send(libs::message::serializer::serialize(type, request));
    logger_.trace() << "send to room, requestId:" << request_id
                    << ", type: " << typeid(request).name() << lg::L_endl;
    return request_id;
  }

  void cancel_request(uint64_t request_id) {
    requests_.cancel(request_id);
  }

  template <typename T>
  void send_notification(MessageType type, const T& notification) {
    channel_.send(libs::message::serializer::serialize(type, notification));
  }

  void on_recv(const Ping&) {
    logger_.debug() << "received ping" << lg::L_endl;
  }

  void on_recv(const Pong&) {
    logger_.debug() << "received pong" << lg::L_endl;
  }

  void on_recv(const User2RoomFwdClient& message) {
    pass_to_session(message);
  }

  void on_recv(const User2RoomNtfLeaveRoom& message) {
    pass_to_session(message);
  }

  template <typename T>
  void on_recv(const T& response) {
    logger_.trace() << "received message requestId: "
                    << response.request_id() << "," << typeid(response).name()
                    << lg::L_endl;
    send_to_waiter(response);
  }

 private:
  template <typename T>
  void send_to_waiter(const T& response) {
    requests_.complete(response.request_id(), std::make_shared<T>(response));
  }

  template <typename T>
  void pass_to_session(const T& message) {
    auto session =
        session::session_manager::instance().find_session(message.uid());
    if (!session) {
      logger_.debug() << "failed to find session for uid(" << message.uid()
                      << ")" << lg::L_endl;
      return;
    }
    session->on_recv(message);
  }

  lg::s_logger& logger_;
  message_dispatcher<room_channel> dispatcher_;
  br::dealer_channel channel_;
  libs::message::pending_requests<Message> requests_;
  std::atomic<uint64_t> next_request_id_{1};
};

}  // namespace user
}  // namespace rsp
