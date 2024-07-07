/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <string>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {

class buffer {
 public:
  static const message::buffer_ptr make_buffer_ptr(const std::string& msg) {
    auto buffer = std::make_shared<message::raw_buffer>();
    buffer->insert(buffer->end(), msg.begin(), msg.end());
    return buffer;
  }
};

}  // namespace libs
}  // namespace rsp
