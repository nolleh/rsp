#pragma once
#include <vector>
#include <string>

namespace rsp {
namespace libs {
namespace message {

template <typename Type>
Type retrieve_parts(std::vector<char> buf, int begin, int end) {
  // TODO(@nolleh) do not construct. change more efficiently.
  return Type{buf.begin() + begin, buf.begin() + end};
}

std::string retrieve_s(std::vector<char> buf, int begin, int end) {
  return retrieve_parts<std::string>(buf, begin, end);
}

std::vector<char> retrieve_v(std::vector<char> buf, int begin, int end) {
  return retrieve_parts<std::vector<char>>(buf, begin, end);
}

}  // namespace message
}  // namespace libs
}  // namespace rsp
