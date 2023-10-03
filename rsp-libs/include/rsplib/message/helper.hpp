/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <algorithm>
#include <bitset>
#include <ostream>
#include <string>
#include <vector>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace message {

template <typename Type>
inline Type retrieve_parts(raw_buffer buf, int begin, int end) {
  // TODO(@nolleh) do not construct. change more efficiently.
  if (begin >= end) {
    return {};
  }
  return Type{buf.begin() + begin, buf.begin() + end};
}

inline std::string retrieve_s(raw_buffer buf, int begin, int end) {
  return retrieve_parts<std::string>(buf, begin, end);
}

inline std::vector<char> retrieve_v(raw_buffer buf, int begin, int end) {
  return retrieve_parts<std::vector<char>>(buf, begin, end);
}

template <typename T>
inline void mset(raw_buffer* dest, T&& t) {
  auto ptr = reinterpret_cast<const char*>(&t);
  dest->insert(dest->end(), ptr, ptr + sizeof(t));
  //
  // auto ptr = reinterpret_cast<char*>(&t);
  // std::copy(ptr, ptr + sizeof(t), dest);
  // memcpy(dest, ptr, sizeof(t));
}

template <typename T>
inline void mget(const raw_buffer& src, T* dest, uint8_t offset) {
  // memcpy(reinterpret_cast<T*>(&src[0] + offset), dest, sizeof(dest));
  const auto ptr = reinterpret_cast<const char*>(&src[0] + offset);
  // byte array, so you can use like this.
  std::copy(ptr, ptr + sizeof(*dest), dest);
}

template <typename T>
// inline std::ostream& operator<<(std::ostream& os, T&& src) {
inline std::bitset<sizeof(T) * 8> to_string(T&& src) {
  // os << std::bitset<sizeof(T)>(src);
  return std::bitset<sizeof(T) * 8>(src);
}

}  // namespace message
}  // namespace libs
}  // namespace rsp
