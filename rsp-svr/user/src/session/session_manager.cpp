
#include "session/session_manager.hpp"

namespace rsp {
namespace user {
namespace session {

std::once_flag session_manager::s_flag;
std::unique_ptr<session_manager> session_manager::s_instance;

// session_manager& session_manager::instance() {
//   std::call_once(session_manager::s_flag, []() {
//     session_manager::s_instance.reset(new session_manager{log_level::TRACE});
//   });
//   return *session_manager::s_instance;
// }

}  // namespace utils
}  // namespace libs
}  // namespace rsp
