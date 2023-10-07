/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/serializer.hpp"
#include "user/session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
namespace lg = rsp::libs::logger;

using job = rsp::libs::job::job;
using link = rsp::libs::link::link;
// using link_ptr = rsp::libs::link::link_ptr;
using session = session::session;
// using session_ptr = rsp::user::session::session_ptr;

class job_login : public job {
 public:
  explicit job_login(const ReqLogin& login) : request_(login) {}

  void run(link* link) {
    lg::logger().debug() << "job_login: " << request_.uid() << lg::L_endl;
    // TODO(@nolleh) signup / login process.
    // session_ = std::dynamic_pointer_cast<session>(link);
    session_ = dynamic_cast<session*>(link);
    send_res_login(request_.uid(), session_);
  }

  void send_res_login(std::string uid, session* session) const {
    ResLogin login;
    login.set_uid(uid);
    login.set_success(true);
    login.set_is_attached(session->room_id() ? true : false);
    login.set_room_id(session->room_id());

    const auto buffer =
        message::serializer::serialize(MessageType::kResLogin, login);
    session_->send(buffer);
    lg::logger().debug() << "sent success login response for:" << uid
                         << lg::L_endl;
  }

 private:
  const ReqLogin request_;
  mutable session* session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
