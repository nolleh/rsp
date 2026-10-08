/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <ostream>
#include <string>
#include <vector>
#include <iostream>

#include "rsplib/message/types.hpp"

namespace rsp {
namespace libs {
namespace message {

template <typename Type>
inline Type retrieve_parts(raw_buffer buf, int begin, int end) {
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

// Wire integers are stored most-significant byte first (network byte order).
template <typename T>
inline void mset_be(raw_buffer* dest, T value) {
  static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>);
  for (size_t remaining = sizeof(T); remaining > 0; --remaining) {
    const auto byte = static_cast<unsigned char>(
        (value >> ((remaining - 1) * 8)) & 0xff);
    // Copy the byte representation without signed-char numeric conversion.
    dest->insert(dest->end(), reinterpret_cast<const char*>(&byte),
                 reinterpret_cast<const char*>(&byte) + 1);
  }
}

template <typename T>
inline bool mget_be(const raw_buffer& src, T* dest, size_t offset) {
  static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>);
  if (offset > src.size() || src.size() - offset < sizeof(T)) {
    return false;
  }
  T value = 0;
  for (size_t index = 0; index < sizeof(T); ++index) {
    value = (value << 8) | static_cast<unsigned char>(src[offset + index]);
  }
  *dest = value;
  return true;
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
