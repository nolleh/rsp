/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <random>

namespace rsp {
namespace libs {
namespace util {

inline ulong rng(ulong min, ulong max) {
  thread_local std::random_device rd;
  thread_local std::mt19937 rng{rd()};
  std::uniform_int_distribution<ulong> uni{min, max};
  return uni(rng);
}

}  // namespace util
}  // namespace libs
}  // namespace rsp
