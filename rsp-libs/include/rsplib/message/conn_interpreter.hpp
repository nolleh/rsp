
#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"
// #include "rsplib/buffer/shared_mutable_buffer.hpp"

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

  // aggregate message until ready.
  void handle_buffer(shared_const_buffer buffer) {
    const size_t len = buffer.end() - buffer.begin();

    if (len + buffer_.size() < CONTENT_LEN + TYPE) {
      // need more bytes.
      buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
      return;
    }

    buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
    const auto length_parts =
        retrieve_parts<std::string>(buffer_, 0, CONTENT_LEN);
    const size_t content_length = stoi(length_parts);

    if (buffer_.size() < content_length + TYPE) {
      // wait more until full message is retrievend
      return;
    }

    // hello, message!
    const auto type_parts = retrieve_parts<std::string>(
        buffer_, CONTENT_LEN + 1, CONTENT_LEN + 1 + TYPE);
    const auto type = static_cast<MessageType>(stoi(type_parts));
    // dispatch_message(type, message);
  }

  template <typename Type>
  Type retrieve_parts(std::vector<char> buf, int begin, int end) const {
    // TODO(@nolleh) do not construct. change more efficiently.
    return Type{buf.begin() + begin, buf.begin() + end};
  }

 private:
  // buffer::shared_mutable_buffer buffer_;
  std::vector<char> buffer_;
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
