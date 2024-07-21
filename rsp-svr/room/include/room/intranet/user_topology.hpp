/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "room/intranet/message_trait.hpp"
#include "room/intranet/unicast_message_dispatcher.hpp"
#include "room/room/room_message_handler.hpp"
#include "room/types.hpp"
#include "rsplib/broker/broker.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace room {

namespace lg = rsp::libs::logger;
namespace br = rsp::libs::broker;
namespace msg = rsp::libs::message;

class user_topology {
 public:
  user_topology()
      : logger_(lg::logger()),
        dispatcher_(this),
        message_handler_(room_message_handler()) {}

  void register_server(const Address& addr) const {
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
    svr->start();
  }

  void unregister_server(const Address& addr) {
    std::lock_guard<std::mutex> l{m_};
    user_servers_.erase(addr);
  }

  void start_recv(const Address& addr) {
    auto iter_server = user_servers_.find(addr);

    if (user_servers_.end() == iter_server) {
      return;
    }

    auto user_server = iter_server->second;
    auto buffer = user_server->recv("topic").get();

    namespace msg = rsp::libs::message;
    auto destructed = msg::serializer::destruct_buffer(buffer);
    dispatcher_.dispatch(destructed.type, destructed.payload, nullptr);
    logger_.trace() << "dispatch finished. start recv" << lg::L_endl;
    start_recv(addr);
  }

  template <typename T>
  uint8_t send_message(const Address& addr, const T& req) const {
    decltype(user_servers_)::const_iterator iter;

    {
      std::lock_guard<std::mutex> l{m_};
      iter = user_servers_.find(addr);
      if (user_servers_.end() == iter) {
        return -1;
      }
    }

    auto buffer = msg::serializer::serialize(message_trait<T>::type, req);
    iter->second->send("topic", buffer);
    return buffer.size();
  }

  template <typename T>
  void on_recv(const T& msg) {
    logger_.trace() << "on_recv:" << typeid(msg).name() << lg::L_endl;
    message_handler_.handle(msg);
  }

 private:
  lg::s_logger& logger_;
  mutable std::mutex m_;
  unicast_message_dispatcher<user_topology> dispatcher_;
  mutable std::map<Address, std::shared_ptr<br::broker_interface>>
      user_servers_;
  room_message_handler message_handler_;
};
}  // namespace room
}  // namespace rsp
