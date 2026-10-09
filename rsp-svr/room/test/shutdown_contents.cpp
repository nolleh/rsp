/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#include <atomic>
#include <cstdio>
#include <cstdlib>

#include "room/contents_interface/so.hpp"

namespace {
std::atomic<int> live_contents{0};
std::atomic<int> created_contents{0};
std::atomic<int> closed_contents{0};

void require(bool condition, const char* message) {
  if (!condition) {
    std::fprintf(stderr, "shutdown fixture: %s\n", message);
    std::abort();
  }
}

class contents final : public rsp::room::room_message_interface {
 public:
  contents() { ++live_contents; ++created_contents; }
  ~contents() override {
    require(closed_, "contents deleted before on_destroy_room");
    --live_contents;
  }
  void on_create_room(rsp::room::RoomId) override { created_ = true; }
  void on_user_enter(rsp::room::Uid) override {}
  void on_user_exit(rsp::room::Uid) override {}
  void on_destroy_room() override {
    require(created_, "room closed before queued creation callback");
    require(!closed_, "on_destroy_room called twice");
    closed_ = true;
    ++closed_contents;
  }
  void on_recv_message(const rsp::room::Uid&, const std::string&) override {}
  void on_kicked_out_user(const rsp::room::Uid&,
                         const rsp::room::KickoutReason&) override {}

 private:
  bool created_ = false;
  bool closed_ = false;
};

class library final : public so_interface {
 public:
  void on_load() override {}
  void on_unload() override {
    // Detect incorrect logical teardown even when a loader pins this DSO.
    require(live_contents == 0, "on_unload called with live room contents");
    require(closed_contents == created_contents,
            "on_unload called before all room shutdown callbacks");
    std::puts("shutdown fixture: contents closed before library unload");
  }
  rsp::room::room_message_interface* create_room(
      rsp::room::room_api_interface*) override {
    return new contents;
  }
  void destroy_room() override {}
};
}  // namespace

extern "C" so_interface* create() { return new library; }
