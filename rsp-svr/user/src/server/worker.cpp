/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "user/server/worker.hpp"

namespace rsp {
namespace user {
namespace server {

std::once_flag worker::s_flag;
std::unique_ptr<worker> worker::s_instance;

}  // namespace server
}  // namespace user
}  // namespace rsp
