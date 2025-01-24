/** Copyright (C) 2025  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "rspcli/state/types.hpp"

namespace rsp {
namespace cli {
namespace state {

struct context {
  Uid uid;
  RoomId room_id;

  context() {
    clear();
  }

  void clear() {
    uid = "";
    room_id = 0;
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
