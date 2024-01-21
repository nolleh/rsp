
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "room/intranet/intranet.hpp"

namespace rsp {
namespace room {

std::once_flag intranet::s_flag;
std::unique_ptr<intranet> intranet::s_instance;

intranet& intranet::instance() {
  std::call_once(intranet::s_flag,
                 []() { intranet::s_instance.reset(new intranet); });
  return *intranet::s_instance;
}

}  // namespace room
}  // namespace rsp
