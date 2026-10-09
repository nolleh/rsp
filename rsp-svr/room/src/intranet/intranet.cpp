
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "room/intranet/intranet.hpp"

namespace rsp {
namespace room {

intranet& intranet::instance() {
  // Construction initializes room_manager, which initializes so_manager.
  // Function-local statics are then destroyed in the reverse dependency order.
  static intranet server;
  return server;
}

}  // namespace room
}  // namespace rsp
