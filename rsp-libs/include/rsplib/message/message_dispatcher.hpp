#pragma once
#include <array>
#include <functional>
#include <iostream>

#include "proto/common/message_type.pb.h"
#include "rsplib/buffer/shared_const_buffer.hpp"

namespace rsp {
namespace libs {
namespace message {

/** this class used for regitering message handlers
 * */
class message_dispatcher {
  /**
   * CONTENT_LEN (4 Bytes) | TYPE (1 Bytes) | [0..LEN]
   * */
 public:
  const size_t CONTENT_LEN = 4;
  const size_t TYPE = 1;

  // hum.. it is hard to reminded c++....  LOL
  // void register_handler(MessageType type, std::function<void> f) {
  // TODO (@nolleh) make map and register
  // }

 private:
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
