
#pragma once
#include <algorithm>
#include <bitset>
#include <iterator>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_mutable_buffer.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {

namespace link {
class link;
}

namespace message {

using shared_mutable_buffer = buffer::shared_mutable_buffer;
using logger = rsp::libs::logger::logger;
using link = rsp::libs::link::link;
/** this class instantiated(owned) per connection for buffering
 * no concern about stratgy (patt) for serializer, for now
 * */
class conn_interpreter {
  /**
   * CONTENT_LEN (8 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 8;
  const size_t TYPE = 4;
  conn_interpreter() : dispatcher_(&message_dispatcher::instance()) {}
  conn_interpreter(message_dispatcher_interface* dispatcher,
                   link* link = nullptr)
      : dispatcher_(dispatcher) {
  }

  // aggregate message until ready.
  void handle_buffer(shared_mutable_buffer buffer, size_t len) {
    logger::instance().trace("handle_buffer, read size:" + std::to_string(len) +
                             ", buf size:" + std::to_string(buffer.size()));

    if (len + buffer_.size() < CONTENT_LEN + TYPE) {
      buffer_.insert(buffer_.end(), buffer.data_begin(), buffer.data_end());
      return;
    }

    buffer_.insert(buffer_.end(), buffer.data_begin(),
                   buffer.data_begin() + len);
    size_t content_length;
    mget(buffer_, &content_length, 0);
    const size_t message_len = CONTENT_LEN + TYPE + content_length;

    if (buffer_.size() < message_len) {
      return;
    }

    // full message is retrievend
    int type_parts;
    mget(buffer_, &type_parts, CONTENT_LEN);

    const auto type = static_cast<MessageType>(type_parts);

    raw_buffer payload;
    std::copy(buffer_.begin() + CONTENT_LEN + TYPE, buffer_.end(),
              back_inserter(payload));

    // TODO(@nolleh) optimize
    buffer_ = retrieve_v(buffer_, message_len, buffer_.size());
    if (!link_)
      dispatcher_->dispatch(type, payload);
    else
      dispatcher_->dispatch(type, payload, link_);
  }

  void handle_buffer(const std::array<char, 128>& buffer, int len) {
    logger::instance().trace("handle_buffer, read size:" + std::to_string(len));

    if (len + buffer_.size() < CONTENT_LEN + TYPE) {
      buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
      return;
    }

    buffer_.insert(buffer_.end(), buffer.begin(), buffer.begin() + len);
    size_t content_length;
    mget(buffer_, &content_length, 0);
    const size_t message_len = CONTENT_LEN + TYPE + content_length;
    if (buffer_.size() < message_len) {
      return;
    }

    // full message is retrievend
    int type_parts;
    mget(buffer_, &type_parts, CONTENT_LEN);

    const auto type = static_cast<MessageType>(type_parts);

    raw_buffer payload;
    std::copy(buffer_.begin() + CONTENT_LEN + TYPE, buffer_.end(),
              back_inserter(payload));

    // TODO(@nolleh) optimize
    buffer_ = retrieve_v(buffer_, message_len, buffer_.size());
    if (!link_) {
      dispatcher_->dispatch(type, payload);
    } else {
      dispatcher_->dispatch(type, payload, link_);
    }
  }

  void attach_link(link* link) {
    // ts is garenteed from user.
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
