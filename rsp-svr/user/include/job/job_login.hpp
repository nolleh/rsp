#pragma once

#include <memory>
#include <string>

#include "proto/common/message_type.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job.hpp"
#include "rsplib/message/helper.hpp"
#include "rsplib/message/types.hpp"
#include "session/session.hpp"

namespace rsp {
namespace user {
namespace job {

namespace message = rsp::libs::message;
using job = rsp::libs::job::job;
using link_ptr = rsp::libs::link::link_ptr;
using session = session::session;
using session_ptr = rsp::user::session::session_ptr;

class job_login : public job {
 public:
  explicit job_login(ReqLogin login)
      : request_(std::shared_ptr<ReqLogin>(&login)) {}

  void run(link_ptr link) {
    std::cout << "job_login: " << request_->uid() << std::endl;
    // TODO(@nolleh) signup / login process.
    session_ = std::dynamic_pointer_cast<session>(link);
    send_reslogin(request_->uid());
  }

  void send_reslogin(std::string uid) {
    ResLogin login;
    login.set_uid(uid);
    login.set_success(true);

    auto content_len = login.ByteSizeLong();
    std::cout << content_len << "," << sizeof(content_len) << std::endl;
    message::raw_buffer message;
    message::mset(&message, content_len);
    message::mset(&message, static_cast<int>(MessageType::RES_LOGIN));
    const auto str = login.SerializeAsString();
    message.insert(message.end(), str.begin(), str.end());
    session_->send(message);
  }

 private:
  std::shared_ptr<ReqLogin> request_;
  session_ptr session_;
};

}  // namespace job
}  // namespace user
}  // namespace rsp
