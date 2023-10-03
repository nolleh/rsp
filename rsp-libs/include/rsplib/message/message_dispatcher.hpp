/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"
#include "rsplib/message/types.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace link {
class link;
}
namespace message {

using link = rsp::libs::link::link;
namespace lg = rsp::libs::logger;

// using ProtoMessage = ::PROTOBUF_NAMESPACE_ID::Message;
/** this class used for regitering message handlers
 * */
class message_dispatcher: public message_dispatcher_interface {
  /**
   * CONTENT_LEN (4 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 8;
  const size_t TYPE = 4;
  static message_dispatcher& instance();

  void register_handler(MessageType type, handler f) override {
    handlers_[type] = f;
  }

  void register_handler2(MessageType type, handler2 f) override {
    handlers2_[type] = f;
  }

  void dispatch(MessageType type, const raw_buffer& buffer) override {
    // it is hard to determine message struct in here.
    // so delegate parsing role to handler
    logger_.trace() << "dispatch..." << lg::L_endl;
    auto iter = handlers_.find(type);
    if (handlers_.end() == iter) return;

    handler handler = iter->second;
    logger_.trace() << "handler..." << &handler << lg::L_endl;
    handler(std::make_shared<raw_buffer>(buffer));
  }

  // TODO(@nolleh) refactor
  void dispatch(MessageType type, const raw_buffer& buffer,
                link* link) override {
    // it is hard to determine message struct in here.
    // so delegate parsing role to handler

    logger_.trace() << "dispatch..." << lg::L_endl;
    auto iter = handlers2_.find(type);
    if (handlers2_.end() == iter) return;

    handler2 handler = iter->second;
    logger_.trace() << "handler..." << &handler << lg::L_endl;
    handler(std::make_shared<raw_buffer>(buffer), link);
  }

 private:
  message_dispatcher(): logger_(lg::logger()) {}
  message_dispatcher(const message_dispatcher&) = delete;
  message_dispatcher& operator=(const message_dispatcher&) = delete;

  static std::once_flag s_flag;
  static std::unique_ptr<message_dispatcher> s_instance;

  std::map<MessageType, handler> handlers_;
  // TODO(@nolleh) refactor
  std::map<MessageType, handler2> handlers2_;
  
  lg::s_logger& logger_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
