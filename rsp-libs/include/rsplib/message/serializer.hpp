/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <utility>

#include "proto/common/message_type.pb.h"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/imemstream.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace message {

struct meta {
  size_t size;
  size_t payload_size;
  MessageType type;
  raw_buffer payload;

  meta() {}
  meta(size_t p_size, size_t p_payload_size, MessageType p_type,
       const raw_buffer& p_payload)
      : size(p_size),
        payload_size(p_payload_size),
        type(p_type),
        payload(p_payload) {}

  meta(const meta& r)
      : size(r.size),
        payload_size(r.payload_size),
        type(r.type),
        payload(r.payload) {}

  meta& operator=(meta&& r) {
    size = std::move(r.size);
    payload_size = std::move(r.payload_size);
    type = std::move(r.type);
    payload = std::move(r.payload);
    return *this;
  }
};

class serializer {
  static const size_t kContentLen = 8;
  static const size_t kType = 4;

 public:
  template <typename Message>
  static raw_buffer serialize(const MessageType type, const Message& message) {
    auto content_len = message.ByteSizeLong();
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
    const size_t message_len = kContentLen + kType + content_length;

    if (buffer.size() < message_len) {
      return {};
    }

    // full message is retrievend
    int type_parts;
    if (!mget(buffer, &type_parts, kContentLen)) return {};
    const auto type = static_cast<MessageType>(type_parts);
    raw_buffer payload;

    std::for_each(buffer.cbegin() + kContentLen + kType, buffer.cend(),
                  [&payload](auto k) { payload.emplace_back(k); });

    // payload.insert(payload.end(), buffer.cbegin() + kContentLen + kType,
    //                buffer.cend());
    return {message_len, content_length, type, payload};
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
