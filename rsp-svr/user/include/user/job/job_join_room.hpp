/** Copyright (C) 2024  nolleh (nolleh7707@gmail.com) **/
#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "proto/common/message_type.pb.h"
#include "proto/room/room.pb.h"
#include "proto/user/login.pb.h"
#include "proto/user/to_room.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "user/intranet/intranet.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
namespace lg = rsp::libs::logger;
namespace ph = std::placeholders;

using job = rsp::libs::job::job;
using session_ptr = rsp::user::session::session_ptr;

class job_join_room : public job,
                      public std::enable_shared_from_this<job_join_room> {
 public:
  explicit job_join_room(const session_ptr& session,
                         const ReqJoinRoom& join_room)
      : intranet_(intranet::instance()),
        session_(session),
        request_(join_room) {}

  void run(completion done) override {
    lg::logger().debug() << "job_join_room: " << request_.DebugString()
                         << lg::L_endl;

    User2RoomReqJoinRoom request;
    request.set_uid(session_->uid());
    request.set_room_id(request_.room_id());
    done_ = std::move(done);
    deadline_ = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    room_request_id_ = intranet_.room().send_request(
        MessageType::kUser2RoomReqJoinRoom, request,
        [self = shared_from_this(), session = session_](
            const std::shared_ptr<Message>& response) {
          session->post_to_serial_context(
              [self, response] { self->handle_res_join_room(response); });
        });
  }

  bool expired(std::chrono::steady_clock::time_point now) const override {
    return room_request_id_ != 0 && now >= deadline_;
  }

  void cancel() override {
    intranet_.room().cancel_request(room_request_id_);
    send_timeout_response();
    finish();
  }

  void handle_res_join_room(const std::shared_ptr<Message> msg) {
    if (finished_) return;
    auto room_response = std::dynamic_pointer_cast<User2RoomResJoinRoom>(msg);
    lg::logger().trace() << "handle_res_join_room: "
                         << room_response->DebugString() << lg::L_endl;
    if (room_response->success()) {
      session_->set_enter_room(room_response->room_id());
    }

    ResJoinRoom response;
    response.set_room_id(room_response->room_id());
    response.set_request_id(request_.request_id());
    response.set_success(room_response->success());

    const auto buffer =
        message::serializer::serialize(MessageType::kResJoinRoom, response);
    session_->send(buffer);
    finish();
  }

 private:
  void send_timeout_response() {
    ResJoinRoom response;
    response.set_room_id(request_.room_id());
    response.set_request_id(request_.request_id());
    response.set_success(false);
    session_->send(
        message::serializer::serialize(MessageType::kResJoinRoom, response));
  }

  void finish() {
    if (finished_) return;
    finished_ = true;
    auto done = std::move(done_);
    if (done) done();
  }

  const intranet& intranet_;
  const ReqJoinRoom request_;
  const session_ptr session_;
  completion done_;
  uint64_t room_request_id_{0};
  std::chrono::steady_clock::time_point deadline_;
  bool finished_{false};
};

}  // namespace job
}  // namespace user
}  // namespace rsp
