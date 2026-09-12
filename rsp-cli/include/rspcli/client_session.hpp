/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <string_view>

#include <boost/asio.hpp>

#include "rspcli/state/context.hpp"
#include "rspcli/state/state.hpp"
#include "rsplib/message/conn_interpreter.hpp"
#include "rsplib/message/message_dispatcher_interface.hpp"

namespace rsp::cli {

class client_session : public std::enable_shared_from_this<client_session> {
 public:
  using socket = boost::asio::ip::tcp::socket;
  using exit_handler = std::function<void()>;

  explicit client_session(socket socket);

  void start();
  void handle_command(std::string_view command);
  void stop();
  void set_exit_handler(exit_handler handler);

  state::State current_state() const { return current_state_; }

 private:
  class session_dispatcher final
      : public libs::message::message_dispatcher_interface {
   public:
    explicit session_dispatcher(client_session* owner) : owner_(owner) {}

    void dispatch(MessageType type, const libs::message::raw_buffer& payload,
                  libs::link::link*) override;

   private:
    client_session* owner_;
  };

  void read_next();
  void dispatch(MessageType type, const libs::message::raw_buffer& payload);
  void transition_to(state::State next);
  void queue_write(MessageType type, libs::message::raw_buffer message);
  void write_next();
  void finish();

  socket socket_;
  state::context context_;
  session_dispatcher dispatcher_;
  libs::message::conn_interpreter interpreter_;
  std::unique_ptr<state::base_state> state_;
  std::array<char, 128> read_buffer_{};
  std::deque<libs::message::raw_buffer> write_queue_;
  state::State current_state_ = state::State::kInit;
  exit_handler exit_handler_;
  bool stopped_ = false;
};

}  // namespace rsp::cli
