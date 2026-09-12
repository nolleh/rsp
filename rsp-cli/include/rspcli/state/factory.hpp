/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

using state_ptr = std::unique_ptr<base_state>;
using creator = std::function<state_ptr(context*, message_sender)>;

class factory {
 public:
  static std::unique_ptr<base_state> create(State state, context* context,
                                            message_sender sender) {
    return s_warehouse.at(state)(context, std::move(sender));
  }

 private:
  static std::map<State, creator> s_warehouse;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
