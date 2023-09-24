
#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"
#include "rsplib/message/message_dispatcher.hpp"
#include "rsplib/message/util.hpp"

namespace rsp {
namespace libs {
namespace message {

using shared_const_buffer = buffer::shared_const_buffer;
/** this class instantiated(owned) per connection for buffering
 * no concern about stratgy (patt) for serializer, for now
 * */
class conn_interpreter {
  /**
   * CONTENT_LEN (4 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 4;
  const size_t TYPE = 1;
  conn_interpreter() : dispatcher_(message_dispatcher::instance()) {}

  // aggregate message until ready.
  void handle_buffer(shared_const_buffer buffer) {
    const size_t len = buffer.end() - buffer.begin();

    if (len + buffer_.size() < CONTENT_LEN + TYPE) {
      buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
      return;
    }

    buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
    const auto length_parts = retrieve_s(buffer_, 0, CONTENT_LEN);
    const size_t content_length = stoi(length_parts);
    const size_t message_len = CONTENT_LEN + TYPE + content_length;
    if (buffer_.size() < message_len) {
      return;
    }

    // full message is retrievend
    const auto type_parts =
        retrieve_s(buffer_, CONTENT_LEN, CONTENT_LEN + TYPE);
    const auto type = static_cast<MessageType>(stoi(type_parts));
    const auto message = retrieve_v(buffer_, CONTENT_LEN + TYPE, message_len);

    // TODO(@nolleh) optimize
    buffer_ = retrieve_v(buffer_, message_len, buffer_.size());

    dispatcher_.dispatch(type, message);
  }

 private:
  message_dispatcher& dispatcher_;
  std::vector<char> buffer_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
