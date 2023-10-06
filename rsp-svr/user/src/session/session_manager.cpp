/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/


#include "user/session/session_manager.hpp"

namespace rsp {
namespace user {
namespace session {

std::once_flag session_manager::s_flag;
std::unique_ptr<session_manager> session_manager::s_instance;

}  // namespace session
}  // namespace user
}  // namespace rsp
