/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "room/config.h"
#include "room/contents_interface/so.hpp"

namespace rsp {
namespace room {

class so_factory {
 public:
  so_factory() {
    // TODO(@nolleh) configuration
    handler_ = dlopen(Room_CONTENTS_BIN, RTLD_NOW);

    if (!handler_) {
      throw std::runtime_error(dlerror());
    }
    reset_dlerror();
    creator_ = reinterpret_cast<so_creator_t>(dlsym(handler_, "create"));
    check_dlerror();
  }

  std::unique_ptr<so_interface> create() const {
    return std::unique_ptr<so_interface>(creator_());
  }

  ~so_factory() {
    if (handler_) {
      dlclose(handler_);
    }
  }

 private:
  static void reset_dlerror() { dlerror(); }

  static void check_dlerror() {
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
      throw std::runtime_error(dlsym_error);
    }
  }
  void *handler_;
  so_creator_t creator_;
};
}  // namespace room
}  // namespace rsp
