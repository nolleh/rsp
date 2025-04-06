/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <string>
#include <vector>

#include "room/contents_interface/types.hpp"
// namespace rsp {
// namespace room {

class room_api_inteface {
  /***
   * in room, send message to user(s).
   * */
  void send_to_user(const std::vector<Uid> uids,
                    const std::string msg);

  /**
   * kickout the user. before kick out,
   * by using send_to_user interface, communicate (vote) with
   * to determine kickout. the voting is content's role.
   * */
  void kick_out_user(Uid uid);
};

// }  // namespace room
// }  // namespace rsp
