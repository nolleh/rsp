/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once
#include <random>

namespace rsp {
namespace libs {
namespace util {

inline uint64_t rng(uint64_t min, uint64_t max) {
  thread_local std::random_device rd;
  thread_local std::mt19937 rng{rd()};
  std::uniform_int_distribution<uint64_t> uni{min, max};
  return uni(rng);
}

}  // namespace util
}  // namespace libs
}  // namespace rsp
