

/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <iostream>

namespace rsp {
namespace libs {
namespace broker {

class interrupted_exception : public std::exception {
 public:
  char* what() { return "interrupted_exception"; }
};
}  // namespace broker
}  // namespace libs
}  // namespace rsp
