#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/types.hpp"
#include "session/session.hpp"

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
    send_reslogin(request_.uid());
  }

  void send_reslogin(std::string uid) const {
    ResLogin login;
    login.set_uid(uid);
    login.set_success(true);

    auto content_len = login.ByteSizeLong();
    message::raw_buffer message;
    message::mset(&message, content_len);
    message::mset(&message, static_cast<int>(MessageType::RES_LOGIN));
    const auto str = login.SerializeAsString();
    message.insert(message.end(), str.begin(), str.end());
    session_->send(message);
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
