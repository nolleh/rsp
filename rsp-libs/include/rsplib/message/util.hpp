#pragma once
#include <boost/archive/binary_oarchive.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace rsp {
namespace libs {
namespace message {

template <typename Type>
inline Type retrieve_parts(std::vector<char> buf, int begin, int end) {
  // TODO(@nolleh) do not construct. change more efficiently.
  return Type{buf.begin() + begin, buf.begin() + end};
}

inline std::string retrieve_s(std::vector<char> buf, int begin, int end) {
  return retrieve_parts<std::string>(buf, begin, end);
}

inline std::vector<char> retrieve_v(std::vector<char> buf, int begin, int end) {
  return retrieve_parts<std::vector<char>>(buf, begin, end);
}

template <typename T>
inline void mset(std::vector<char>* dest, T&& t) {
  auto ptr = reinterpret_cast<const char*>(&t);
  dest->insert(dest->end(), ptr, ptr + sizeof(t));
  //
  // auto ptr = reinterpret_cast<char*>(&t);
  // std::copy(ptr, ptr + sizeof(t), dest);
  // memcpy(dest, ptr, sizeof(t));
}

template <typename T>
inline void mget(const std::vector<char>& src, T* dest, uint8_t offset) {
  // memcpy(reinterpret_cast<T*>(&src[0] + offset), dest, sizeof(dest));
  const auto ptr = reinterpret_cast<const char*>(&src[0] + offset);
  // byte array, so you can use like this.
  std::copy(ptr, ptr + sizeof(*dest), dest);
}

}  // namespace message
}  // namespace libs
}  // namespace rsp
