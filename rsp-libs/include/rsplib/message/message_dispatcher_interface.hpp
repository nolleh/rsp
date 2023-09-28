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
  template <typename handler>
  void register_handler(MessageType type, handler f) {}

  void dispatch(MessageType type, const raw_buffer& buffer) const {}
  void dispatch(MessageType type, const raw_buffer& buffer,
                rsp::libs::link::link* link) const {}
};

}  // namespace message
}  // namespace libs
}  // namespace rsp
