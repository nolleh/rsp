#include "proto/room/room.pb.h"
#include "room/room/room_manager.hpp"
#include "rsplib/message/types.hpp"

namespace rsp {
namespace room {

class room_message_handler {
 public:
  room_message_handler() : room_manager_(room_manager::instance()) {}

  void handle(const ReqCreateRoom& create_room) {
    auto room = room_manager_.create_room(create_room.uid());
  }

  void handle(const ReqJoinRoom& join_room) {
    auto room = room_manager_.find_room(join_room.room_id());
    room->join_room();
  }

 private:
  room_manager& room_manager_;
};
}  // namespace room
}  // namespace rsp
