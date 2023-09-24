#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"

namespace rsp {
namespace libs {
namespace message {

using handler = std::function<void(std::shared_ptr<std::vector<char>>)>;

// using ProtoMessage = ::PROTOBUF_NAMESPACE_ID::Message;
/** this class used for regitering message handlers
 * */
class message_dispatcher {
  /**
   * CONTENT_LEN (4 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 4;
  const size_t TYPE = 1;
  static message_dispatcher& instance() {
    std::call_once(message_dispatcher::s_flag, []() {
      message_dispatcher::s_instance.reset(new message_dispatcher);
    });
    return *message_dispatcher::s_instance;
  }

  /**
   * make sure register handler that running worker threads
   * **/
  void register_handler(MessageType type, handler f) {
    // TODO(@nolleh) looks like it is better change this logic to macro
    handlers_[type] = f;
  }

  void dispatch(MessageType type, const std::vector<char>& buffer) const {
    // it is hard to determine message struct in here.
    // so delegate parsing role to handler

    // TODO(@nolleh) worker thread::strand!
    //

    auto iter = handlers_.find(type);
    if (handlers_.end() == iter) return;

    handler handler;
    std::tie(std::ignore, handler) = *iter;
    handler(make_shared<std::vector<char>>(buffer));
  }

 private:
  message_dispatcher() {}
  message_dispatcher(const message_dispatcher&) = delete;
  message_dispatcher& operator=(const message_dispatcher&) = delete;

  static std::once_flag s_flag;
  static std::unique_ptr<message_dispatcher> s_instance;

  std::map<MessageType, handler> handlers_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
