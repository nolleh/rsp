
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
            const uint8_t context, const std::string& host)
      : type_(type),
        context_id_(context),
        context_(zmq::context_t(context_id_)),
        addr_("tcp://" + host) {
    // address_ = generate_address(type, service_name, context);
  }

  ~publisher() { stop(); }

  publisher(publisher&& r)
      : type_(std::move(r.type_)),
        context_id_(r.context_id_),
        context_(std::move(r.context_)),
        addr_(std::move(r.addr_)),
        socket_(std::move(r.socket_)) {}


  void start() override {
    create_socket();
    auto& logger = rsp::libs::logger::logger();
    logger.info() << "start publisher:" << addr_ << rsp::libs::logger::L_endl;
    ba::io_context::work work(io_context_);
    th_ = std::thread([this] { io_context_.run(); });
    io_context_.post([&] {
      stop_ = false;
      socket_.connect(addr_);
      logger.info() << "created context(" << &io_context_ << ") socket ("
                    << &socket_ << ") " << rsp::libs::logger::L_endl;
    });
  }

  void stop() override {
    auto& logger = rsp::libs::logger::logger();
    logger.info() << "stop publisher" << rsp::libs::logger::L_endl;
    if (stop_.load()) {
      logger.trace() << "already stopped" << rsp::libs::logger::L_endl;
      return;
    }

    stop_ = true;
    logger.info() << "stop context(" << &io_context_ << ") socket (" << &socket_
                  << ") joinable(" << th_.joinable() << ")"
                  << rsp::libs::logger::L_endl;

    io_context_.stop();
    if (th_.joinable()) {
      th_.join();
    }

    try {
      spromise_.set_exception(std::make_exception_ptr(interrupted_exception()));
      rpromise_.set_exception(std::make_exception_ptr(interrupted_exception()));
    } catch (const std::future_error& e) {
    }
  }

  void add_topic(const std::string& topic) override {}

  void sub_topic(const std::string& topic) override {}

  void send(const std::string& topic, const raw_buffer& buffer) override {
    // TODO(@nolleh) wait mechanism
    auto& logger = rsp::libs::logger::logger();
    if (stop_.load()) {
      logger.trace() << "already stopped" << rsp::libs::logger::L_endl;
      return;
    }
    spromise_ = std::promise<int>();
    io_context_.post([&, buffer] {
      if (stop_.load()) {
        logger.trace() << "already stopped" << rsp::libs::logger::L_endl;
        return;
      }
      logger.trace() << "send context(" << &io_context_ << ") socket ("
                     << &socket_ << ")" << rsp::libs::logger::L_endl;
      // auto rc = s_send(&socket_, buffer.data());
      zmq::message_t msg(buffer.size());
      memcpy(msg.data(), buffer.data(), buffer.size());
      logger.trace() << "send:" << msg.size() << rsp::libs::logger::L_endl;
      auto rc = socket_.send(msg, zmq::send_flags::none);
      spromise_.set_value(rc.value());
    });
  }

  std::future<raw_buffer> recv(const std::string& topic) override {
    // static_assert(false, "not supperted calling recv from publisher");
    //
    if (CastType::kBroadCast == type_) {
      throw std::runtime_error("not supported");
    }

    auto& logger = rsp::libs::logger::logger();
    rpromise_ = std::promise<raw_buffer>();
    io_context_.post([&] {
      if (stop_.load()) {
        logger.trace() << "already stopped" << rsp::libs::logger::L_endl;
        rpromise_.set_exception(
            std::make_exception_ptr(interrupted_exception{}));
        return;
      }

      logger.trace() << "recv context(" << &io_context_ << ") socket ("
                     << &socket_ << ")" << rsp::libs::logger::L_endl;

      zmq::message_t msg;
      auto rc = socket_.recv(msg, zmq::recv_flags::none);

      // if (!cp) {
      if (!rc || msg.size() < 0) {
        rpromise_.set_exception(
            std::make_exception_ptr(interrupted_exception{}));
        return;
      }

      // auto cs = std::string{cp};
      auto cs = std::string{static_cast<char*>(msg.data()), msg.size()};
      auto buffer = raw_buffer{cs.begin(), cs.end()};
      logger.trace() << "read:" << buffer.size() << rsp::libs::logger::L_endl;
      rpromise_.set_value(buffer);
    });

    return rpromise_.get_future();
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
    socket_ = zmq::socket_t{context_, zmq::socket_type::pub};
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_anycast() {
    // socket_ = zmq::socket_t{context_, zmq::socket_type::push};
    socket_ = zmq::socket_t{context_, zmq::socket_type::req};
    // socket_.set(zmq::sockopt::linger, 1);
  }

  void create_unicast() {
    socket_ = zmq::socket_t{context_, zmq::socket_type::req};
    // socket_.set(zmq::sockopt::linger, 1);
  }

  CastType type_;
  uint8_t context_id_;
  zmq::context_t context_;
  std::string addr_;
  address address_;

  std::thread th_;
  ba::io_context io_context_;
  ba::executor_work_guard<decltype(io_context_.get_executor())> work_guard_{
      io_context_.get_executor()};
  zmq::socket_t socket_;
  std::promise<int> spromise_;
  std::promise<raw_buffer> rpromise_;
  std::atomic<bool> stop_;
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
