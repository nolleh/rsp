
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
#include "rsplib/message/helper.hpp"

namespace rsp {
namespace libs {
namespace broker {

namespace ba = boost::asio;

class subscriber : public broker_interface {
 public:
  subscriber(CastType type, const std::string& service_name,
             const uint8_t context, const std::string& topic)
      : type_(type),
        context_id_(context),
        context_(zmq::context_t(context_id_)),
        // socket_(create_socket()),
        topics_({topic}) {}

  subscriber(subscriber&& r)
      : type_(std::move(r.type_)),
        context_id_(r.context_id_),
        context_(std::move(r.context_)),
        socket_(std::move(r.socket_)),
        topics_(std::move(r.topics_)) {}

  ~subscriber() { stop(); }

  void start() override {
    // zmq::socket_t not allowed to move. so temporarily, do initialize in start
    create_socket();
    auto& logger = rsp::libs::logger::logger();
    logger.info() << "start subscriber" << rsp::libs::logger::L_endl;
    ba::io_context::work work(io_context_);
    th_ = std::thread([this] { io_context_.run(); });
    io_context_.post([&] {
      stop_ = false;
      auto& logger = rsp::libs::logger::logger();
      socket_.bind("tcp://*:5558");
      // socket_.connect("tcp://localhost:5558");
      logger.info() << "created context(" << &io_context_ << ") socket ("
                    << &socket_ << ")" << rsp::libs::logger::L_endl;
    });
  }

  void stop() override {
    auto& logger = rsp::libs::logger::logger();
    logger.info() << "stop subscriber" << rsp::libs::logger::L_endl;
    if (stop_.load()) {
      auto& logger = rsp::libs::logger::logger();
      logger.trace() << "already stopped" << rsp::libs::logger::L_endl;
      return;
    }

    stop_ = true;
    logger.info() << "stop context(" << &io_context_ << ") socket (" << &socket_
                  << ") joinable(" << th_.joinable() << ")"
                  << rsp::libs::logger::L_endl;

    logger.debug() << "context stop, joining.." << rsp::libs::logger::L_endl;
    io_context_.stop();
    if (th_.joinable()) th_.join();
    try {
      promise_.set_exception(std::make_exception_ptr(interrupted_exception{}));
    } catch (const std::future_error& e) {
    }

    logger.debug() << "stopped" << rsp::libs::logger::L_endl;
  }

  void add_topic(const std::string& topic) override { topics_.insert(topic); }

  void sub_topic(const std::string& topic) override { topics_.erase(topic); }

  void send(const std::string& topic, const raw_buffer& buffer) override {
    // TODO (@cindy) wait mechanism
    auto& logger = rsp::libs::logger::logger();
    if (stop_.load()) {
      logger.debug() << "already stopped" << rsp::libs::logger::L_endl;
      return;
    }

    io_context_.post([&, buffer] {
      if (stop_.load()) {
        logger.debug() << "already stopped" << rsp::libs::logger::L_endl;
        return;
      }
      // logger.trace() << "send context(" << &io_context_ << ") socket ("
      //                << &socket_ << ")" << rsp::libs::logger::L_endl;
      // auto rc = s_send(&socket_, buffer.data());
      zmq::message_t msg(buffer.size());
      memcpy(msg.data(), buffer.data(), buffer.size());

      auto rc = socket_.send(msg, zmq::send_flags::none);
      // auto rc = s_send(&socket_, buffer.data());
      logger.trace() << "send:" << rc.value() << rsp::libs::logger::L_endl;
    });
  }

  std::future<raw_buffer> recv(const std::string& topic) override {
    // promise_ = std::make_unique<std::promise<raw_buffer>>();
    auto& logger = rsp::libs::logger::logger();
    promise_ = std::promise<raw_buffer>();
    if (stop_.load()) {
      logger.debug() << "already stopped" << rsp::libs::logger::L_endl;
      promise_.set_exception(std::make_exception_ptr(interrupted_exception{}));
      return promise_.get_future();
    }

    io_context_.post([&] {
      if (stop_.load()) {
        logger.debug() << "already stopped" << rsp::libs::logger::L_endl;
        promise_.set_exception(
            std::make_exception_ptr(interrupted_exception{}));
        return;
      }

      logger.trace() << "recv context(" << &io_context_ << ") socket ("
                     << &socket_ << ")" << rsp::libs::logger::L_endl;

      zmq::message_t msg;
      auto rc = socket_.recv(msg, zmq::recv_flags::none);

      // if (!cp) {
      if (!rc || msg.size() < 0) {
        promise_.set_exception(
            std::make_exception_ptr(interrupted_exception{}));
        return;
      }

      // auto cs = std::string{cp};
      auto cs = std::string{static_cast<char*>(msg.data()), msg.size()};
      auto buffer = raw_buffer{cs.begin(), cs.end()};
      logger.trace() << "read:" << buffer.size() << rsp::libs::logger::L_endl;
      promise_.set_value(buffer);
    });

    return promise_.get_future();
  }

 private:
  void create_socket() {
    switch (type_) {
      case CastType::kBroadCast:
        create_broadcast();
        break;
      case CastType::kUniCast:
        create_unicast();
        break;
      case CastType::kAnyCast:
        create_anycast();
        break;
    }
  }
  void create_broadcast() {
    socket_ = zmq::socket_t{context_, zmq::socket_type::sub};
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_anycast() {
    // socket_ = zmq::socket_t{context_, zmq::socket_type::pull};
    socket_ = zmq::socket_t{context_, zmq::socket_type::rep};
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_unicast() {
    // socket_.set(zmq::sockopt::linger, 1);
    socket_ = zmq::socket_t{context_, zmq::socket_type::rep};
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
  // std::unique_ptr<std::promise<raw_buffer>> promise_;
  std::promise<raw_buffer> promise_;
  std::atomic<bool> stop_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
