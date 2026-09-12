/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/client_session.hpp"

#include <memory>
#include <utility>

#include "rspcli/state/factory.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp::cli {

namespace lg = libs::logger;

client_session::client_session(socket socket)
    : socket_(std::move(socket)),
      dispatcher_(this),
      interpreter_(&dispatcher_) {}  // NOLINT

void client_session::session_dispatcher::dispatch(
    MessageType type, const libs::message::raw_buffer& payload,
    libs::link::link*) {
  owner_->dispatch(type, payload);
}

void client_session::start() {
  transition_to(state::State::kInit);
  read_next();
}

void client_session::handle_command(std::string_view command) {
  if (stopped_ || !state_) return;

  if (const auto next = state_->on_command(command)) {
    transition_to(*next);
  }
}

void client_session::stop() {
  if (stopped_) return;
  transition_to(state::State::kExit);
}

void client_session::set_exit_handler(exit_handler handler) {
  exit_handler_ = std::move(handler);
}

void client_session::read_next() {
  if (stopped_) return;

  auto self = shared_from_this();
  socket_.async_read_some(
      boost::asio::buffer(read_buffer_),
      [self](const boost::system::error_code& error, std::size_t size) {
        if (error) {
          if (error != boost::asio::error::operation_aborted &&
              error != boost::asio::error::eof) {
            lg::logger().warn()
                << "client read failed: " << error.message() << lg::L_endl;
          }
          self->finish();
          return;
        }

        if (!self->interpreter_.handle_buffer(self->read_buffer_, size)) {
          lg::logger().warn() << "invalid message received" << lg::L_endl;
          self->transition_to(state::State::kExit);
          return;
        }
        self->read_next();
      });
}

void client_session::dispatch(MessageType type,
                              const libs::message::raw_buffer& payload) {
  if (stopped_ || !state_) return;

  if (const auto next = state_->on_message(
          type, std::make_shared<libs::message::raw_buffer>(payload))) {
    transition_to(*next);
  }
}

void client_session::transition_to(state::State next) {
  if (stopped_) return;

  current_state_ = next;
  state_ = state::factory::create(
      next, &context_,
      [this](MessageType type, libs::message::raw_buffer message) {
        queue_write(type, std::move(message));
      });
  state_->enter();

  if (next == state::State::kExit) finish();
}

void client_session::queue_write(MessageType,
                                 libs::message::raw_buffer message) {
  if (stopped_) return;

  const bool write_in_progress = !write_queue_.empty();
  write_queue_.push_back(std::move(message));
  if (!write_in_progress) write_next();
}

void client_session::write_next() {
  if (stopped_ || write_queue_.empty()) return;

  auto self = shared_from_this();
  boost::asio::async_write(
      socket_, boost::asio::buffer(write_queue_.front()),
      [self](const boost::system::error_code& error, std::size_t) {
        if (error) {
          if (error != boost::asio::error::operation_aborted) {
            lg::logger().warn()
                << "client write failed: " << error.message() << lg::L_endl;
          }
          self->transition_to(state::State::kExit);
          return;
        }

        self->write_queue_.pop_front();
        self->write_next();
      });
}

void client_session::finish() {
  if (stopped_) return;
  stopped_ = true;
  current_state_ = state::State::kExit;

  boost::system::error_code ignored;
  socket_.cancel(ignored);
  socket_.shutdown(socket::shutdown_both, ignored);
  socket_.close(ignored);

  if (exit_handler_) exit_handler_();
}

}  // namespace rsp::cli
