
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <memory>

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_exit : public base_state {
 public:
  static std::shared_ptr<base_state> create(socket* socket,
                                            struct context* context) {
    return std::shared_ptr<state_exit>(new state_exit(socket, context));
  }

  void init() override {
    prompt_ << "closing...";
    return;
  }

 protected:
  explicit state_exit(socket* socket, struct context* context)
      : base_state(socket, context) {
    state_ = State::kExit;
    next_ = state_;
  }
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
