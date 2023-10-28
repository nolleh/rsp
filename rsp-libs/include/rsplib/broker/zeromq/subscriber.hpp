
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <set>
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

class subscriber : public broker_interface {
 public:
  subscriber(CastType type, const std::string& service_name,
             const uint8_t context, const std::string& topic)
      : type_(type), context_id_(context), topics_({topic}) {}

  subscriber(subscriber&& r)
      : type_(std::move(r.type_)),
        context_id_(r.context_id_),
        context_(std::move(r.context_)),
        topics_(std::move(r.topics_)),
        socket_(std::move(r.socket_)) {}

  ~subscriber() { stop(); }

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
      auto& logger = rsp::libs::logger::logger();
      logger.info() << "created context(" << &io_context_ << ") socket ("
                    << &socket_ << ")" << rsp::libs::logger::L_endl;
      socket_.bind("tcp://*:5558");
    });
  }

  void stop() override {
    auto& logger = rsp::libs::logger::logger();
    logger.info() << "stop context(" << &io_context_ << ") socket (" << &socket_
                  << rsp::libs::logger::L_endl;
    io_context_.post([&] {
      if (promise_) {
        promise_->set_exception(
            std::make_exception_ptr(interrupted_exception()));
      }
    });
    io_context_.stop();
  }

  void add_topic(const std::string& topic) override { topics_.insert(topic); }

  void sub_topic(const std::string& topic) override { topics_.erase(topic); }

  void send(const std::string& topic, const raw_buffer& os) override {
    throw std::runtime_error("not supported");
  }

  std::future<raw_buffer> recv(const std::string& topic) override {
    promise_ = std::make_unique<std::promise<raw_buffer>>();

    io_context_.post([&] {
      auto& logger = rsp::libs::logger::logger();
      logger.trace() << "recv context(" << &io_context_ << ") socket ("
                     << &socket_ << ")" << rsp::libs::logger::L_endl;
      // auto cp = s_recv(&socket_);
      zmq::message_t msg;
      socket_.recv(&msg);
      // if (!cp) {
      if (msg.size() < 0) {
        promise_->set_exception(
            std::make_exception_ptr(interrupted_exception{}));
        promise_.reset();
        return;
      }
      // auto cs = std::string{cp};
      auto cs = std::string{static_cast<char*>(msg.data()), msg.size()};
      auto buffer = raw_buffer{cs.begin(), cs.end()};
      promise_->set_value(buffer);
      promise_.reset();
    });

    return promise_->get_future();
  }

 private:
  void create_broadcast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::sub});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_anycast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::pull});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_unicast() {
    socket_ = std::move(zmq::socket_t{context_, zmq::socket_type::req});
    // socket_.set(zmq::sockopt::linger, 1);
  }

  CastType type_;
  uint8_t context_id_;
  std::set<std::string> topics_;

  std::thread th_;
  ba::io_context io_context_;
  ba::executor_work_guard<decltype(io_context_.get_executor())> work_guard_{
      io_context_.get_executor()};
  zmq::context_t context_;
  zmq::socket_t socket_;
  std::unique_ptr<std::promise<raw_buffer>> promise_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
