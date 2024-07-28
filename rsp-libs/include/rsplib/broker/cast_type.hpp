
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

// TODO(@nolleh) look's like using zmq, the wrapping interface cannot hold the
// usage. change interface
enum class CastType {
  kBroadCast /** all listener notification */,
  kUniCast /** specific listener req/res */,
  kAnyCast /** any of listener req/res */,

  kReq,
  kRep,
  kPub,
  kSub,
};
