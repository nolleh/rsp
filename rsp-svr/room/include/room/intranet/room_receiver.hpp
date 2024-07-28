/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <typeinfo>

#include "room/intranet/message_dispatcher.hpp"
#include "room/intranet/message_trait.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace ba = boost::asio;

class intranet;
class room_receiver {
 public:
  room_receiver()
      : logger_(lg::logger()),
        threads_(1),
        dispatcher_(this),
        message_handler_(room_message_handler()) {
    room_receiver_ = br::broker::s_create_subscriber(
        CastType::kRep, "room", 1, "tcp://*:5559", "topic");
  }

  void start() {
    room_receiver_->start();
    // logger_.info() << "waiting message is ready" << lg::L_endl;
    // sleep(3);

    // TODO(@nolleh) return start and give a work
    start_recv();
  }

  void stop() {
    threads_.stop();
  }

  void start_recv() {
    auto buffer = room_receiver_->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
    start_recv();
  }

  template <typename T>
  void on_recv(const T& msg) {
    handle(msg);
  }

  template <typename T>
  void send_response(MessageType type, const T& res) {
    namespace msg = rsp::libs::message;
    auto buffer = msg::serializer::serialize(type, res);
    logger_.trace() << "send_reponse: " << typeid(res).name() << lg::L_endl;
    room_receiver_->send("topic", buffer);
  }

 private:
  template <typename T>
  void handle(const T& msg) {
    logger_.trace() << "on_recv: " << typeid(msg).name() << lg::L_endl;
    auto response = message_handler_.handle(msg);
    send_response(message_trait<T>::res_type, response);
  }

  lg::s_logger& logger_;
  libs::thread_pool threads_;
  intranet* intranet_;
  message_dispatcher<room_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> room_receiver_;
  room_message_handler message_handler_;
};

// template <>
// void room_receiver::on_recv(const User2RoomReqCreateRoom& msg) {
//   handle(msg);
// }

}  // namespace room
}  // namespace rsp
