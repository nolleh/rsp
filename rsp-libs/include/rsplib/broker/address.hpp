/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <algorithm>
#include <string>

#include "rsplib/broker/cast_type.hpp"
namespace rsp {
namespace libs {
namespace broker {

/** service_name@cast_letter-numbers. for example, user@U12345  */
// TODO(@nolleh) how can we efficently but safely allocate the numbers?
// zookeeper? haha too-big.. redis? mysql? postgres?
using address = std::string;

inline address generate_address(CastType type, const std::string& service_name,
                                const uint8_t context) {
  auto type_letter = [](CastType type) {
    return type == CastType::kAnyCast   ? 'A'
           : type == CastType::kUniCast ? 'U'
                                        : 'M';
  }(type);

  auto randchar = []() {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };

  std::string postfix(5, 0);
  std::generate_n(postfix.begin(), 5, randchar);
  return service_name + type_letter + postfix;
}

}  // namespace broker
}  // namespace libs
}  // namespace rsp
