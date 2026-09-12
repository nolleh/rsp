/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rsplib/broker/zeromq/routed_channel.hpp"

#include <chrono>
#include <cstring>
#include <deque>
#include <future>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <zmq_addon.hpp>

#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace broker {

namespace {

constexpr auto kPollInterval = std::chrono::milliseconds(10);

raw_buffer to_buffer(const zmq::message_t& message) {
  raw_buffer buffer(message.size());
  if (!buffer.empty()) {
    std::memcpy(buffer.data(), message.data(), message.size());
  }
  return buffer;
}

void log_channel_error(const char* channel, const std::exception& exception) {
  logger::logger().error() << channel << " channel failed: " << exception.what()
                           << logger::L_endl;
}

}  // namespace

dealer_channel::dealer_channel(std::string address, routing_id identity,
                               zmq_context context)
    : address_(std::move(address)),
      identity_(std::move(identity)),
      context_(std::move(context)) {
}

dealer_channel::~dealer_channel() { stop(); }

void dealer_channel::start(message_handler handler) {
  if (running_.exchange(true)) return;

  handler_ = std::move(handler);
  std::promise<void> ready;
  auto started = ready.get_future();
  thread_ = std::thread(&dealer_channel::run, this, std::move(ready));
  try {
    started.get();
  } catch (...) {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    throw;
  }
}

void dealer_channel::stop() {
  running_ = false;
  if (thread_.joinable()) thread_.join();
}

void dealer_channel::send(raw_buffer message) {
  std::lock_guard<std::mutex> lock(outgoing_mutex_);
  outgoing_.push_back(std::move(message));
}

void dealer_channel::run(std::promise<void> ready) {
  try {
    zmq::socket_t socket{*context_, zmq::socket_type::dealer};
    socket.set(zmq::sockopt::routing_id, identity_);
    socket.set(zmq::sockopt::linger, 0);
    socket.connect(address_);
    ready.set_value();

    while (running_) {
      send_pending(&socket);

      zmq::pollitem_t item{socket.handle(), 0, ZMQ_POLLIN, 0};
      zmq::poll(&item, 1, kPollInterval);
      if ((item.revents & ZMQ_POLLIN) == 0) continue;

      zmq::message_t message;
      if (socket.recv(message, zmq::recv_flags::none) && handler_) {
        try {
          handler_(to_buffer(message));
        } catch (const std::exception& exception) {
          log_channel_error("dealer handler", exception);
        }
      }
    }
  } catch (const std::exception& exception) {
    try {
      ready.set_exception(std::current_exception());
    } catch (const std::future_error&) {
      log_channel_error("dealer", exception);
    }
    running_ = false;
  }
}

void dealer_channel::send_pending(zmq::socket_t* socket) {
  std::deque<raw_buffer> pending;
  {
    std::lock_guard<std::mutex> lock(outgoing_mutex_);
    pending.swap(outgoing_);
  }

  for (const auto& message : pending) {
    socket->send(zmq::buffer(message), zmq::send_flags::none);
  }
}

router_channel::router_channel(std::string address, zmq_context context)
    : address_(std::move(address)), context_(std::move(context)) {}

router_channel::~router_channel() { stop(); }

void router_channel::start(message_handler handler) {
  if (running_.exchange(true)) return;

  handler_ = std::move(handler);
  std::promise<void> ready;
  auto started = ready.get_future();
  thread_ = std::thread(&router_channel::run, this, std::move(ready));
  try {
    started.get();
  } catch (...) {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    throw;
  }
}

void router_channel::stop() {
  running_ = false;
  if (thread_.joinable()) thread_.join();
}

void router_channel::wait() {
  if (thread_.joinable()) thread_.join();
}

void router_channel::send(routing_id destination, raw_buffer message) {
  std::lock_guard<std::mutex> lock(outgoing_mutex_);
  outgoing_.emplace_back(std::move(destination), std::move(message));
}

void router_channel::run(std::promise<void> ready) {
  try {
    zmq::socket_t socket{*context_, zmq::socket_type::router};
    socket.set(zmq::sockopt::router_mandatory, 1);
    socket.set(zmq::sockopt::linger, 0);
    socket.bind(address_);
    ready.set_value();

    while (running_) {
      send_pending(&socket);

      zmq::pollitem_t item{socket.handle(), 0, ZMQ_POLLIN, 0};
      zmq::poll(&item, 1, kPollInterval);
      if ((item.revents & ZMQ_POLLIN) == 0) continue;

      std::vector<zmq::message_t> frames;
      const auto received =
          zmq::recv_multipart(socket, std::back_inserter(frames));
      if (!received || frames.size() != 2 || !handler_) continue;

      try {
        handler_(frames[0].to_string(), to_buffer(frames[1]));
      } catch (const std::exception& exception) {
        log_channel_error("router handler", exception);
      }
    }
  } catch (const std::exception& exception) {
    try {
      ready.set_exception(std::current_exception());
    } catch (const std::future_error&) {
      log_channel_error("router", exception);
    }
    running_ = false;
  }
}

void router_channel::send_pending(zmq::socket_t* socket) {
  std::deque<outgoing_message> pending;
  {
    std::lock_guard<std::mutex> lock(outgoing_mutex_);
    pending.swap(outgoing_);
  }

  for (const auto& [destination, message] : pending) {
    try {
      socket->send(zmq::buffer(destination), zmq::send_flags::sndmore);
      socket->send(zmq::buffer(message), zmq::send_flags::none);
    } catch (const zmq::error_t& error) {
      logger::logger().warn()
          << "unable to route message to " << destination << ": "
          << error.what() << logger::L_endl;
    }
  }
}

}  // namespace broker
}  // namespace libs
}  // namespace rsp
