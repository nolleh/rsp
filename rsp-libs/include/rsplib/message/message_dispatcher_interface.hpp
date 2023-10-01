#pragma once
#include "proto/common/message_type.pb.h"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace link {
class link;
}
namespace message {

class message_dispatcher_interface {
 public:
  virtual void register_handler(MessageType type, handler f) {}
  virtual void register_handler2(MessageType type, handler2 f) {}

  virtual void dispatch(MessageType type, const raw_buffer& buffer) {}
  virtual void dispatch(MessageType type, const raw_buffer& buffer,
                        rsp::libs::link::link* link) {}
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
