/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <iomanip>
#include <iostream>
#include <cstring>

namespace rsp {
namespace cli {

enum color {
  kRed = 31,
  kGreen = 32,
  kYellow = 33,
  kDarkBlue = 34,
  kMagenta = 35,
  kBlue = 36,
  kGray = 37
};

template <typename State>
class prompt {
 public:
  explicit prompt(State* state) : state_(state) {}

  template <typename T>
  prompt& operator<<(const T& value) {
    const int kWidth = 50;
    std::cout << "\x1b[" << color::kMagenta << "m" << std::setw(kWidth)
              << std::setfill('=') << std::endl;

    std::cout << "\n\x1b[" << color::kYellow << "m" << *state_ << std::endl;

    std::cout << "\x1b[0m" << std::endl;

    std::cout << std::setw(kWidth - std::strlen(value)) << std::setfill(' ')
              << value;

    std::cout << "\x1b[" << color::kMagenta << "m";
    std::cout << std::setw(kWidth) << std::setfill('=') << std::endl;

    std::cout << "\x1b[0m" << std::endl;
    return *this;
  }

 private:
  State* state_;
};
}  // namespace cli
}  // namespace rsp
