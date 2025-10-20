/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <typeinfo>

#include <boost/asio.hpp>
// #include <experimental/future>

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
    room_receiver_ = br::broker::s_create_subscriber(CastType::kRep, "room", 1,
                                                     "tcp://*:5559", "topic");
    room_sub_receiver_ = br::broker::s_create_subscriber(
        CastType::kSub, "room", 1, "tcp://*:5561", "topic");
  }

  void start() {
    stop_ = false;
    room_receiver_->start();
    room_sub_receiver_->start();
    threads_.start();
    // logger_.info() << "waiting message is ready" << lg::L_endl;
    // sleep(3);

    // co_spawn(
    //     threads_.get_executor(), [self = this] { return self->start_recv();
    //     }, ba::detached);

    // TODO(@nolleh) async mechanism should be refined. hm
    co_spawn(
        threads_.get_executor(),
        [self = this] { return self->start_sub_recv(); }, ba::detached);
    // TODO(@nolleh) return start and give a work
    start_recv();
  }

  void stop() {
    threads_.stop();
    room_sub_receiver_->stop();
    room_receiver_->stop();
  }

  void start_recv() {
    if (!stop_) {
      auto buffer = room_receiver_->recv("topic").get();
      namespace msg = rsp::libs::message;
      auto destructed = msg::serializer::destruct_buffer(buffer);
      dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
      logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
      start_recv();
    }
  }

  ba::awaitable<void> start_sub_recv() {
    while (!stop_) {
      auto buffer = room_sub_receiver_->recv("topic").get();
      namespace msg = rsp::libs::message;
      auto destructed = msg::serializer::destruct_buffer(buffer);
      dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
      logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
    }
    co_return;
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
  std::atomic<bool> stop_;
  intranet* intranet_;
  message_dispatcher<room_receiver> dispatcher_;
  std::shared_ptr<br::broker_interface> room_receiver_;
  std::shared_ptr<br::broker_interface> room_sub_receiver_;
  room_message_handler message_handler_;
};

// template <>
// void room_receiver::on_recv(const User2RoomReqCreateRoom& msg) {
//   handle(msg);
// }

template <>
void room_receiver::handle(const User2RoomReqFwdRoom& msg);

}  // namespace room
}  // namespace rsp
