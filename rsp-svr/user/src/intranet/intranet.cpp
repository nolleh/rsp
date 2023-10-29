/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "user/intranet/intranet.hpp"

namespace rsp {
namespace user {

std::once_flag intranet::s_flag;
std::unique_ptr<intranet> intranet::s_instance;

intranet& intranet::instance() {
  std::call_once(intranet::s_flag,
                 []() { intranet::s_instance.reset(new intranet); });
  return *intranet::s_instance;
}

}  // namespace user
}  // namespace rsp
