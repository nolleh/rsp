/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <memory>
#include <string>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {

class buffer {
 public:
  static message::buffer_ptr make_buffer_ptr(const std::string& msg) {
    return std::make_shared<const message::raw_buffer>(msg.begin(), msg.end());
  }
};

}  // namespace libs
}  // namespace rsp
