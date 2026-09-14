/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>

#include <zmq.hpp>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace broker {

using event_topic = std::string;
using event_message_handler =
std::function<void(event_topic, message::raw_buffer)>;
using event_zmq_context = std::shared_ptr<zmq::context_t>;

enum class endpoint_mode { kBind, kConnect };

class event_publisher {
 public:
  event_publisher(
      std::string address, endpoint_mode mode,
      event_zmq_context context = std::make_shared<zmq::context_t>(1));
  ~event_publisher();

  event_publisher(const event_publisher&) = delete;
  event_publisher& operator=(const event_publisher&) = delete;

  void start();
  void stop();
  void publish(event_topic topic, message::raw_buffer payload);

 private:
  using outgoing_event = std::pair<event_topic, message::raw_buffer>;

  void run(std::promise<void> ready);
  void publish_pending(zmq::socket_t* socket);

  const std::string address_;
  const endpoint_mode mode_;
  const event_zmq_context context_;
  std::mutex outgoing_mutex_;
  std::deque<outgoing_event> outgoing_;
  std::thread thread_;
  std::atomic<bool> running_{false};
};

class event_subscriber {
 public:
  event_subscriber(
      std::string address, endpoint_mode mode,
      event_zmq_context context = std::make_shared<zmq::context_t>(1));
  ~event_subscriber();

  event_subscriber(const event_subscriber&) = delete;
  event_subscriber& operator=(const event_subscriber&) = delete;

  void subscribe(event_topic prefix);
  void unsubscribe(const event_topic& prefix);
  void start(event_message_handler handler);
  void stop();

 private:
  enum class subscription_operation { kSubscribe, kUnsubscribe };
  using subscription_change =
      std::pair<subscription_operation, event_topic>;

  void run(std::promise<void> ready);
  void apply_subscription_changes(zmq::socket_t* socket);

  const std::string address_;
  const endpoint_mode mode_;
  const event_zmq_context context_;
  event_message_handler handler_;
  std::mutex subscription_mutex_;
  std::set<event_topic> subscriptions_;
  std::deque<subscription_change> subscription_changes_;
  std::thread thread_;
  std::atomic<bool> running_{false};
};

}  // namespace broker
}  // namespace libs
}  // namespace rsp
