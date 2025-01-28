
/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#include "room/so/so_manager.hpp"

#include <memory>

namespace rsp {
namespace room {

std::once_flag so_manager::s_flag;
std::unique_ptr<so_manager> so_manager::s_instance;

}  // namespace room
}  // namespace rsp
