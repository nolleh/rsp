#include "rsplib/message/message_dispatcher.hpp"

namespace rsp {
namespace libs {
namespace message {

std::once_flag message_dispatcher::s_flag;
std::unique_ptr<message_dispatcher> message_dispatcher::s_instance;

message_dispatcher& message_dispatcher::instance() {
  std::call_once(message_dispatcher::s_flag, []() {
    message_dispatcher::s_instance.reset(new message_dispatcher);
  });
  return *message_dispatcher::s_instance;
}

}  // namespace message
}  // namespace libs
}  // namespace rsp
