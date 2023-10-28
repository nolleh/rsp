
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <zmq.hpp>

#include <boost/asio.hpp>

#include "rsplib/broker/broker_interface.hpp"
#include "rsplib/broker/exception.hpp"
#include "rsplib/broker/zeromq/cppzmq/zhelpers.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace broker {

namespace ba = boost::asio;

class publisher : public broker_interface {
 public:
  publisher(CastType type, const std::string& service_name,
            const uint8_t context)
      : type_(type), context_id_(context) {
    // address_ = generate_address(type, service_name, context);
  }

  ~publisher() { stop(); }

  publisher(publisher&& r)
      : type_(std::move(r.type_)),
        context_id_(r.context_id_),
        context_(std::move(r.context_)),
        socket_(std::move(r.socket_)) {}

  void start() override {
    th_ = std::thread([this] { io_context_.run(); });
    ba::io_context::work work(io_context_);
    io_context_.post([&] {
      context_ = zmq::context_t(context_id_);
      switch (type_) {
        case CastType::kBroadCast:
          create_broadcast();
        case CastType::kUniCast:
          create_unicast();
        case CastType::kAnyCast:
          create_anycast();
      }
      socket_.connect("tcp://127.0.0.1:5558");
    });
  }

  void stop() override {
    promise_.set_exception(std::make_exception_ptr(interrupted_exception()));
    io_context_.stop();
  }

  void add_topic(const std::string& topic) override {}

  void sub_topic(const std::string& topic) override {}

  void send(const std::string& topic, const raw_buffer& buffer) override {
    promise_ = std::promise<int>();
    io_context_.post([&, buffer] {
      auto& logger = rsp::libs::logger::logger();
      logger.trace() << "send context(" << &io_context_ << ") socket ("
                     << &socket_ << ")" << rsp::libs::logger::L_endl;
      // auto rc = s_send(&socket_, buffer.data());
      zmq::message_t msg(buffer.size());
      memcpy(msg.data(), buffer.data(), buffer.size());
      logger.trace() << "send:" << msg.size() << rsp::libs::logger::L_endl;
      auto rc = socket_.send(msg);
      promise_.set_value(rc);
    });
  }

  std::future<raw_buffer> recv(const std::string& topic) override {
    // static_assert(false, "not supperted calling recv from publisher");
    throw std::runtime_error("not supported");
  }

 private:
  void create_broadcast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::pub});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_anycast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::push});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_unicast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::rep});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  CastType type_;
  uint8_t context_id_;
  address address_;

  std::thread th_;
  ba::io_context io_context_;
  ba::executor_work_guard<decltype(io_context_.get_executor())> work_guard_{
      io_context_.get_executor()};
  zmq::context_t context_;
  zmq::socket_t socket_;
  std::promise<int> promise_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
