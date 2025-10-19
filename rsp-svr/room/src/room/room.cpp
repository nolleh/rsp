/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#include "room/room/room.hpp"

#include <memory>
#include <string>

#include "proto/room/room.pb.h"
#include "room/intranet/intranet.hpp"
#include "room/intranet/user_topology.hpp"

namespace rsp {
namespace room {

void room::send_to_user_impl(const SenderType& sender_type,
                             const std::shared_ptr<user> sender,
                             const std::shared_ptr<std::vector<user>> users,
                             const lm::buffer_ptr buffer) {
  User2RoomReqFwdClient msg;
  msg.set_sender_type(sender_type);
  if (SenderType::kUser == sender_type) {
    msg.set_sender_uid(sender->uid);
  }
  msg.set_message(std::string{buffer->cbegin(), buffer->cend()});

  auto& user_servers = intranet::instance().user();

  std::for_each(users->cbegin(), users->cend(),
                [&user_servers, &msg](const auto& user) {
                  msg.set_uid(user.uid);
                  user_servers.send_message(user.addr, msg);
                });
}

void room::kick_out_user_impl(const Uid& uid, const KickoutReason& reason) {
  const auto user = users_.find(uid);
  if (users_.end() == user) {
    logger_.debug() << std::format("unable to find user({0}) in room", uid);
    return;
  }

  User2RoomNtfLeaveRoom msg;
  msg.set_reason(LeaveRoomReason::kKickedOut);
  msg.set_kickout_reason(static_cast<int>(reason));

  auto& user_servers = intranet::instance().user();
  user_servers.send_message(user->second.addr, msg);

  contents_->on_kicked_out_user(uid, reason);
  users_.erase(uid);
}

}  // namespace room
}  // namespace rsp
