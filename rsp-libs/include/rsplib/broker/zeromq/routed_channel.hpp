/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include <zmq.hpp>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace broker {

using raw_buffer = message::raw_buffer;
using routing_id = std::string;
using zmq_context = std::shared_ptr<zmq::context_t>;

class dealer_channel {
 public:
  using message_handler = std::function<void(raw_buffer)>;

  dealer_channel(std::string address, routing_id identity,
                 zmq_context context = std::make_shared<zmq::context_t>(1));
  ~dealer_channel();

  dealer_channel(const dealer_channel&) = delete;
  dealer_channel& operator=(const dealer_channel&) = delete;

  void start(message_handler handler);
  void stop();
  void send(raw_buffer message);

 private:
  void run(std::promise<void> ready);
  void send_pending(zmq::socket_t* socket);

  const std::string address_;
  const routing_id identity_;
  const zmq_context context_;
  message_handler handler_;
  std::mutex outgoing_mutex_;
  std::deque<raw_buffer> outgoing_;
  std::thread thread_;
  std::atomic<bool> running_{false};
};

class router_channel {
 public:
  using message_handler =
      std::function<void(routing_id, raw_buffer)>;

  explicit router_channel(
      std::string address,
      zmq_context context = std::make_shared<zmq::context_t>(1));
  ~router_channel();

  router_channel(const router_channel&) = delete;
  router_channel& operator=(const router_channel&) = delete;

  void start(message_handler handler);
  void stop();
  void wait();
  void send(routing_id destination, raw_buffer message);

 private:
  using outgoing_message = std::pair<routing_id, raw_buffer>;

  void run(std::promise<void> ready);
  void send_pending(zmq::socket_t* socket);

  const std::string address_;
  const zmq_context context_;
  message_handler handler_;
  std::mutex outgoing_mutex_;
  std::deque<outgoing_message> outgoing_;
  std::thread thread_;
  std::atomic<bool> running_{false};
};

}  // namespace broker
}  // namespace libs
}  // namespace rsp
