#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace message {

// using ProtoMessage = ::PROTOBUF_NAMESPACE_ID::Message;
/** this class used for regitering message handlers
 * */
class message_dispatcher {
  /**
   * CONTENT_LEN (4 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 8;
  const size_t TYPE = 4;
  static message_dispatcher& instance();

  /**
   * make sure register handler that running worker threads
   * **/
  void register_handler(MessageType type, handler f) {
    // TODO(@nolleh) looks like it is better change this logic to macro
    handlers_[type] = f;
  }

  void dispatch(MessageType type, const raw_buffer& buffer) const {
    // it is hard to determine message struct in here.
    // so delegate parsing role to handler

    // TODO(@nolleh) worker thread::strand!
    //

    std::cout << "dispatch..." << std::endl;
    auto iter = handlers_.find(type);
    if (handlers_.end() == iter) return;

    handler handler = iter->second;
    std::cout << "handler..." << &handler << std::endl;
    handler(std::make_shared<raw_buffer>(buffer));
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
