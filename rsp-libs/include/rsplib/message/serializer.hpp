/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <stdexcept>
#include <utility>

#include "proto/common/message_type.pb.h"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/imemstream.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace message {

enum class parse_status {
  kIncomplete,
  kComplete,
  kInvalid,
};

struct meta {
  parse_status status = parse_status::kIncomplete;
  size_t size = 0;
  size_t payload_size = 0;
  MessageType type = MessageType{};
  raw_buffer payload;
};

class serializer {
  static constexpr size_t kContentLen = 8;
  static constexpr size_t kType = 4;

 public:
  static constexpr size_t kHeaderSize = kContentLen + kType;
  static constexpr size_t kMaxPayloadSize = 64 * 1024;

  // TODO(@nolleh) make MessageTypeTraits to attach MessageType to Message
  template <typename Message>
  static raw_buffer serialize(const MessageType type, const Message& message) {
    auto content_len = message.ByteSizeLong();
    if (content_len > kMaxPayloadSize) {
      throw std::length_error("message payload exceeds the maximum size");
    }

    raw_buffer buffer;
    mset(&buffer, content_len);
    mset(&buffer, static_cast<int>(type));
    const auto str = message.SerializeAsString();
    buffer.insert(buffer.end(), str.begin(), str.end());
    return buffer;
  }

  static meta destruct_buffer(const raw_buffer& buffer) {
    size_t content_length;
    if (!mget(buffer, &content_length, 0)) return {};
    if (content_length > kMaxPayloadSize) {
      return {.status = parse_status::kInvalid};
    }

    const size_t message_len = kHeaderSize + content_length;

    if (buffer.size() < message_len) {
      return {};
    }

    // full message is retrieved
    int type_parts;
    if (!mget(buffer, &type_parts, kContentLen)) return {};
    const auto type = static_cast<MessageType>(type_parts);

    raw_buffer payload;
    payload.insert(payload.end(), buffer.cbegin() + kContentLen + kType,
                   buffer.cbegin() + message_len);

    // payload.insert(payload.end(), buffer.cbegin() + kContentLen + kType,
    //                buffer.cend());
    return {parse_status::kComplete, message_len, content_length, type,
            payload};
  }

  template <typename Message>
  static bool deserialize(const raw_buffer& buffer, Message* message) {
    imemstream stream(const_cast<const char*>(buffer.data()), buffer.size());
    return message->ParseFromIstream(&stream);
  }
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
