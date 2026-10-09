/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <sstream>
#include <string>

// Own the input bytes so the stream remains valid independently of the buffer.
class imemstream : public std::istringstream {
 public:
  imemstream(const char* base, std::size_t size)
      : std::istringstream(size == 0 ? std::string{} : std::string(base, size),
                           std::ios::in | std::ios::binary) {}
};
