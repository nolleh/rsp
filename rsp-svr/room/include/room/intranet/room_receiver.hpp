/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <typeinfo>
#include <utility>

#include "room/intranet/message_dispatcher.hpp"
#include "room/intranet/message_trait.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/zeromq/routed_channel.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace msg = rsp::libs::message;

class room_receiver {
 public:
  room_receiver()
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(),
        channel_("tcp://*:5559") {}

  void start() {
    channel_.start(
        [this](br::routing_id source, msg::raw_buffer buffer) {
          current_route_ = std::move(source);
          auto destructed = msg::serializer::destruct_buffer(buffer);
          dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
        });
    channel_.wait();
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

  void on_recv(const User2RoomReqJoinRoom& request) {
    const auto destination = current_route_;
    message_handler_.handle(
        request, destination,
        [this, destination](User2RoomResJoinRoom response) {
          send_response(destination, MessageType::kUser2RoomResJoinRoom,
                        response);
        });
  }

  void on_recv(const User2RoomReqLeaveRoom& request) {
    const auto destination = current_route_;
    message_handler_.handle(
        request, destination,
        [this, destination](User2RoomResLeaveRoom response) {
          send_response(destination, MessageType::kUser2RoomResLeaveRoom,
                        response);
        });
  }

 private:
  template <typename T>
  void send_response(const RoutingId& destination, MessageType type,
                     const T& response) {
    logger_.trace() << "send response: " << typeid(response).name()
                    << lg::L_endl;
    channel_.send(destination, msg::serializer::serialize(type, response));
  }

  lg::s_logger& logger_;
  message_dispatcher<room_receiver> dispatcher_;
  room_message_handler message_handler_;
  br::router_channel channel_;
  RoutingId current_route_;
};

}  // namespace room
}  // namespace rsp
