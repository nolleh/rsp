/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
// logged in users
#include <memory>
#include <string>

#include "proto/common/ping.pb.h"
#include "proto/user/login.pb.h"
#include "rsplib/job/job_scheduler.hpp"
#include "rsplib/link/link.hpp"
#include "rsplib/logger/logger.hpp"
#include "rsplib/server/tcp_connection.hpp"
#include "user/server/worker.hpp"

namespace rsp {
namespace user {
namespace session {

enum class UserState {
  kLogouted,
  kLoggedIn,
  kDetached,
  kInRoom,
};

using link = libs::link::link;
namespace server = libs::server;
namespace lg = rsp::libs::logger;
using job_scheduler = libs::job::job_scheduler;
using worker = rsp::user::server::worker;
class session;
using session_ptr = std::shared_ptr<session>;

namespace ba = boost::asio;

class session : public link, public std::enable_shared_from_this<session> {
 public:
  explicit session(server::connection_ptr conn)
      : worker_(worker::instance()), link(conn), state_(UserState::kLogouted) {
    conn->attach_link(this);
  }

  ~session() {
    // session destroy meaning logout
    lg::logger().trace() << "destroy session" << lg::L_endl;
    stop(true);
  }

  void start() {
    // https://stackoverflow.com/questions/70939861/does-boostasio-co-spawn-create-an-actual-thread
    co_spawn(
        worker_.get_executor(),
        [self = shared_from_this()] { return self->send_heartbeats(); },
        ba::detached);
  }

  void stop(bool force_close) {
    stop_ = true;
    link::stop(force_close);
  }

  void on_connected() override {
    // TODO(@nolleh) notify to other servers that user attached
  }

  // unexpected discon
  void on_disconnected() override {
    // TODO(@nolleh) notify to other servers that user detached
  }

  // by client/server, requested closed was done
  void on_closed() override {
    // nothing todo
  }

  template <typename Message>
  void on_recv(Message&& msg) {
    lg::logger().debug() << "session - base on_recv" << lg::L_endl;
    last_received_ = std::time(nullptr);
  }

  void on_recv_unknown() {
    lg::logger().error() << "session - on_recv, unknown message" << lg::L_endl;
    last_received_ = std::time(nullptr);
    // unknown message = uncommunicatable with message. stop
    enqueue_stop(false);
  }

  void enqueue_stop(bool force);

  void set_user(const std::string& uid) { uid_ = uid; }

  std::string uid() { return uid_; }

  uint16_t room_id() { return room_id_; }

 private:
  void enqueue_job(libs::job::job_ptr job) {
    worker_.post(std::bind(&job_scheduler::push_and_run, &scheduler_, job));
    lg::logger().trace() << "enqueue finished" << lg::L_endl;
  }

  // this logic is here because make running in worker.
  // if io pool is enough to handle, then consider move this.
  ba::awaitable<void> send_heartbeats() {
    ba::steady_timer timer{worker_.get_executor()};

    const int kPingPeriod = 5;
    const int kMaxFailedCount = 3;
    try {
      int failed_count = 0;
      while (!stop_.load()) {
        // TODO(@nolleh) there is precondition, thread isn't locked check;
        timer.expires_after(std::chrono::seconds(kPingPeriod));
        co_await timer.async_wait(ba::deferred);

        if (stop_.load()) {
          break;
        }

        std::time_t now = std::time(nullptr);
        if (last_received_.load() + kPingPeriod > now) {
          continue;
        }
        if (++failed_count >= kMaxFailedCount) {
          enqueue_stop(true);
          break;
        }
        Ping ping;
        const auto buffer =
            rsp::libs::message::serializer::serialize(MessageType::kPing, ping);
        send(buffer);
      }
    } catch (const std::exception& ex) {
      lg::logger().error() << "ping error" << lg::L_endl;
    }
    lg::logger().info() << "end of heartbeats" << lg::L_endl;
  }

  worker& worker_;
  job_scheduler scheduler_;
  UserState state_;
  std::string uid_;

  std::atomic<time_t> last_received_;
  std::atomic<bool> stop_;
  uint16_t room_id_;
};

template <>
void session::on_recv(Ping& msg);

template <>
void session::on_recv(ReqLogin& msg);

template <>
void session::on_recv(ReqLogout& msg);

}  // namespace session
}  // namespace user
}  // namespace rsp
