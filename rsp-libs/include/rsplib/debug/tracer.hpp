/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/
#pragma once

#if defined __linux__ || defined __APPLE__
extern "C" {
#include <execinfo.h>
#include <unistd.h>
}
#include <csignal>
#include <cstddef>
#endif

#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {

class tracer {
 public:
  static void install() {
#if defined __linux__ || defined __APPLE__
    signal(SIGABRT, handler);
    signal(SIGSEGV, handler);
#endif
  }
  static void handler(int sig) {
#if defined __linux__ || defined __APPLE__
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    backtrace_symbols_fd(array, size, STDERR_FILENO);

    logger::logger().error() << "signal " << sig << logger::L_endl;
    exit(1);
#endif
  }
};

}  // namespace libs
}  // namespace rsp

// elif -> SHS.. forgoted. some library
