/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <utility>

#include "room/intranet/message_dispatcher.hpp"
#include "room/intranet/message_trait.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/zeromq/routed_channel.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/queued_requests.hpp"
#include "rsplib/message/serializer.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace msg = rsp::libs::message;

class user_channel {
 public:
  explicit user_channel(
      std::string address = "tcp://*:5559",
      br::zmq_context context = std::make_shared<zmq::context_t>(1),
      room_manager& manager = room_manager::instance())
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(manager),
        channel_(std::move(address), std::move(context)) {}

  void start() {
    start_async();
    channel_.wait();
  }

  void start_async() {
    channel_.start(
        [this](br::routing_id source, msg::raw_buffer buffer) {
          current_route_ = std::move(source);
          auto destructed = msg::serializer::destruct_buffer(buffer);
          dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
        });
  }

  void stop() { channel_.stop(); }

  template <typename T>
  void send_notification(const RoutingId& destination,
                         const T& notification) {
    channel_.send(destination, msg::serializer::serialize(
                                   message_trait<T>::type, notification));
  }

  template <typename T>
  void on_recv(const T& request) {
    logger_.trace() << "on_recv: " << typeid(request).name() << lg::L_endl;
    auto response = message_handler_.handle(request, current_route_);
    send_response(current_route_, message_trait<T>::res_type, response);
  }

  void on_recv(const User2RoomFwdRoom& notification) {
    message_handler_.handle(notification, current_route_);
  }

  void on_recv(const User2RoomCancelRequest& notification) {
    requests_->cancel({current_route_, notification.request_id()});
  }

  void on_recv(const User2RoomReqCreateRoom& request) {
    handle_request(request);
  }

  void on_recv(const User2RoomReqJoinRoom& request) {
    handle_request(request);
  }

  void on_recv(const User2RoomReqLeaveRoom& request) {
    handle_request(request);
  }

 private:
  template <typename T>
  void handle_request(const T& request) {
    const auto destination = current_route_;
    const msg::queued_requests::key key{destination, request.request_id()};
    const auto ticket = requests_->admit(key);
    if (!ticket) return;
    const auto registry = requests_;

    try {
      message_handler_.handle(
          request, destination,
          [this, destination, registry, key, ticket](auto response) {
            const bool started = ticket->started();
            if (!registry->finish(key, ticket)) return;
            if (started) {
              send_response(destination, message_trait<T>::res_type, response);
            }
          },
          [ticket] { return ticket->try_start(); });
    } catch (...) {
      registry->finish(key, ticket);
      throw;
    }
  }

  template <typename T>
  void send_response(const RoutingId& destination, MessageType type,
                     const T& response) {
    logger_.trace() << "send response: " << typeid(response).name()
                    << lg::L_endl;
    channel_.send(destination, msg::serializer::serialize(type, response));
  }

  lg::s_logger& logger_;
  message_dispatcher<user_channel> dispatcher_;
  room_message_handler message_handler_;
  br::router_channel channel_;
  RoutingId current_route_;
  std::shared_ptr<msg::queued_requests> requests_ =
      std::make_shared<msg::queued_requests>();
};

}  // namespace room
}  // namespace rsp
