/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <algorithm>
#include <bitset>
#include <iterator>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/logger/logger.hpp"
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/serializer.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {

namespace link {
class link;
}

namespace message {

namespace lg = rsp::libs::logger;
using link = rsp::libs::link::link;
/** this class instantiated(owned) per connection for buffering
 * no concern about stratgy (patt) for serializer, for now
 * */
class conn_interpreter {
  /**
   * CONTENT_LEN (8 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  conn_interpreter() : dispatcher_(&message_dispatcher::instance()) {}
  conn_interpreter(message_dispatcher_interface* dispatcher,
                   link* link = nullptr)
      : dispatcher_(dispatcher) {}

  // aggregate message until ready.
  void handle_buffer(const std::array<char, 128>& buffer, size_t len) {
    lg::logger().trace() << "handle_buffer, read size:" + std::to_string(len)
                         << lg::L_endl;

    buffer_.insert(buffer_.end(), buffer.begin(), buffer.begin() + len);

    auto meta = serializer::destruct_buffer(buffer_);
    if (!meta.size) {
      return;
    }

    buffer_ = retrieve_v(buffer_, meta.size, buffer_.size());
    lg::logger().trace() << "about to dispatch" << lg::L_endl;
    dispatcher_->dispatch(meta.type, meta.payload, link_);
  }

  void attach_link(link* link) {
    // ts is guaranteed from user.
    // this is temp code.
    link_ = link;
  }

 private:
  link* link_ = nullptr;
  // TODO(@nolleh) remove this?
  message_dispatcher_interface* dispatcher_;
  raw_buffer buffer_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
