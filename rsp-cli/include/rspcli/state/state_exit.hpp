/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <memory>
#include <string_view>
#include <utility>

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_exit : public base_state {
 public:
  static std::unique_ptr<base_state> create(context* context,
                                            message_sender sender) {
    return std::unique_ptr<state_exit>(
        new state_exit(context, std::move(sender)));
  }

  void enter() override { prompt_ << "closing..."; }
  transition on_command(std::string_view) override { return std::nullopt; }

 private:
  explicit state_exit(context* context, message_sender sender)
      : base_state(context, std::move(sender)) {
    state_ = State::kExit;
    context->clear();
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
