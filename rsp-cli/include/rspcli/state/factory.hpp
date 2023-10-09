/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once

#include <map>
#include <memory>

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

using creator = std::function<std::shared_ptr<base_state>(socket*)>;
class factory {
 public:
  static std::shared_ptr<base_state> create(State state, socket* socket) {
    return s_warehouse[state](socket);
  }

 private:
  static std::map<State, creator> s_warehouse;
};

}  // namespace state
}  // namespace cli
}  // namespace rsp
