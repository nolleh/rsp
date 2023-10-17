
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

enum class CastType {
  kBroadCast /** all listener notification */,
  kUniCast /** secific listener req/res */,
  kAnyCast /** any of listener req/res */,
};

