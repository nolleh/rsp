/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include "proto/common/message_type.pb.h"
#include "proto/common/ping.pb.h"
#include "proto/room/room.pb.h"

template <typename T>
struct message_trait {
  static constexpr MessageType type = kPing;
  static constexpr MessageType res_type = kPong;
};

#define MESSAGE_TRAIT(Msg, Type, ResType)         \
  template <>                                     \
  struct message_trait<Msg> {                     \
    static constexpr MessageType type = Type;     \
    static constexpr MessageType res_type = ResType; \
  };

MESSAGE_TRAIT(User2RoomReqCreateRoom, MessageType::kUser2RoomReqCreateRoom,
              MessageType::kUser2RoomResCreateRoom);
MESSAGE_TRAIT(User2RoomReqJoinRoom, MessageType::kUser2RoomReqJoinRoom,
              MessageType::kUser2RoomResJoinRoom);
MESSAGE_TRAIT(User2RoomReqFwdRoom, MessageType::kUser2RoomReqFwdRoom,
              MessageType::kUser2RoomResFwdRoom);
