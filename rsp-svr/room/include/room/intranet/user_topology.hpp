/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "room/intranet/unicast_message_dispatcher.hpp"
#include "room/intranet/message_trait.hpp"
#include "room/room/room_message_handler.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace msg = rsp::libs::message;

using address = std::string;
class user_topology {
 public:
  user_topology()
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(room_message_handler()) {}

  void register_server(const address& addr) {
    {
      std::lock_guard<std::mutex> l{m_};
      auto iter = user_servers_.find(addr);
      if (user_servers_.end() != iter) {
        return;
      }
    }

    auto svr =
        br::broker::s_create_publisher(CastType::kUniCast, "user", 1, addr);
    std::lock_guard<std::mutex> l{m_};
    user_servers_[addr] = svr;
  }

  void unregister_server(const address& addr) {
    std::lock_guard<std::mutex> l{m_};
    user_servers_.erase(addr);
  }

  template <typename T>
  uint8_t send_message(const address& addr, const T& req) {
    decltype(user_servers_)::const_iterator iter;

    {
      std::lock_guard<std::mutex> l{m_};
      iter = user_servers_.find(addr);
      if (user_servers_.end() == iter) {
        return -1;
      }
    }

    auto buffer = msg::serializer::serialize(message_trait<T>::type, req);
    return iter->second->send("topic", buffer);
  }

  template <typename T>
  void on_recv(const T& msg) {
    logger_.trace() << "on_recv:" << typeid(msg).name() << lg::L_endl;
    message_handler_.handle(msg);
  }

 private:
  lg::s_logger& logger_;
  std::mutex m_;
  unicast_message_dispatcher<user_topology> dispatcher_;
  std::map<address, std::shared_ptr<br::broker_interface>> user_servers_;
  room_message_handler message_handler_;
};
}  // namespace room
}  // namespace rsp
