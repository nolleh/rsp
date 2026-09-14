/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rsplib/broker/zeromq/event_channel.hpp"

#include <chrono>
#include <deque>
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

message::raw_buffer to_buffer(const zmq::message_t& message) {
  const auto* first = static_cast<const char*>(message.data());
  return {first, first + message.size()};
}

void configure_endpoint(zmq::socket_t* socket, endpoint_mode mode,
                        const std::string& address) {
  if (mode == endpoint_mode::kBind) {
    socket->bind(address);
    return;
  }
  socket->connect(address);
}

void log_event_channel_error(const char* channel,
                             const std::exception& exception) {
  logger::logger().error() << channel << " failed: " << exception.what()
                           << logger::L_endl;
}

}  // namespace

event_publisher::event_publisher(std::string address, endpoint_mode mode,
                                 event_zmq_context context)
    : address_(std::move(address)), mode_(mode), context_(std::move(context)) {}

event_publisher::~event_publisher() { stop(); }

void event_publisher::start() {
  if (running_.exchange(true)) return;

  std::promise<void> ready;
  auto started = ready.get_future();
  thread_ = std::thread(&event_publisher::run, this, std::move(ready));
  try {
    started.get();
  } catch (...) {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    throw;
  }
}

void event_publisher::stop() {
  running_ = false;
  if (thread_.joinable()) thread_.join();
}

void event_publisher::publish(event_topic topic, message::raw_buffer payload) {
  std::lock_guard<std::mutex> lock(outgoing_mutex_);
  outgoing_.emplace_back(std::move(topic), std::move(payload));
}

void event_publisher::run(std::promise<void> ready) {
  try {
    zmq::socket_t socket{*context_, zmq::socket_type::pub};
    socket.set(zmq::sockopt::linger, 0);
    configure_endpoint(&socket, mode_, address_);
    ready.set_value();

    while (running_) {
      publish_pending(&socket);
      std::this_thread::sleep_for(kPollInterval);
    }
  } catch (const std::exception& exception) {
    try {
      ready.set_exception(std::current_exception());
    } catch (const std::future_error&) {
      log_event_channel_error("event publisher", exception);
    }
    running_ = false;
  }
}

void event_publisher::publish_pending(zmq::socket_t* socket) {
  std::deque<outgoing_event> pending;
  {
    std::lock_guard<std::mutex> lock(outgoing_mutex_);
    pending.swap(outgoing_);
  }

  for (const auto& [topic, payload] : pending) {
    socket->send(zmq::buffer(topic), zmq::send_flags::sndmore);
    socket->send(zmq::buffer(payload), zmq::send_flags::none);
  }
}

event_subscriber::event_subscriber(std::string address, endpoint_mode mode,
                                   event_zmq_context context)
    : address_(std::move(address)), mode_(mode), context_(std::move(context)) {}

event_subscriber::~event_subscriber() { stop(); }

void event_subscriber::subscribe(event_topic prefix) {
  std::lock_guard<std::mutex> lock(subscription_mutex_);
  if (!subscriptions_.insert(prefix).second) return;
  subscription_changes_.emplace_back(subscription_operation::kSubscribe,
                                      std::move(prefix));
}

void event_subscriber::unsubscribe(const event_topic& prefix) {
  std::lock_guard<std::mutex> lock(subscription_mutex_);
  if (subscriptions_.erase(prefix) == 0) return;
  subscription_changes_.emplace_back(subscription_operation::kUnsubscribe,
                                      prefix);
}

void event_subscriber::start(event_message_handler handler) {
  if (running_.exchange(true)) return;

  handler_ = std::move(handler);
  std::promise<void> ready;
  auto started = ready.get_future();
  thread_ = std::thread(&event_subscriber::run, this, std::move(ready));
  try {
    started.get();
  } catch (...) {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    throw;
  }
}

void event_subscriber::stop() {
  running_ = false;
  if (thread_.joinable()) thread_.join();
}

void event_subscriber::run(std::promise<void> ready) {
  try {
    zmq::socket_t socket{*context_, zmq::socket_type::sub};
    socket.set(zmq::sockopt::linger, 0);
    configure_endpoint(&socket, mode_, address_);
    apply_subscription_changes(&socket);
    ready.set_value();

    while (running_) {
      apply_subscription_changes(&socket);

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
        log_event_channel_error("event subscriber handler", exception);
      }
    }
  } catch (const std::exception& exception) {
    try {
      ready.set_exception(std::current_exception());
    } catch (const std::future_error&) {
      log_event_channel_error("event subscriber", exception);
    }
    running_ = false;
  }
}

void event_subscriber::apply_subscription_changes(zmq::socket_t* socket) {
  std::deque<subscription_change> changes;
  {
    std::lock_guard<std::mutex> lock(subscription_mutex_);
    changes.swap(subscription_changes_);
  }

  for (const auto& [operation, prefix] : changes) {
    if (operation == subscription_operation::kSubscribe) {
      socket->set(zmq::sockopt::subscribe, prefix);
    } else {
      socket->set(zmq::sockopt::unsubscribe, prefix);
    }
  }
}

}  // namespace broker
}  // namespace libs
}  // namespace rsp
