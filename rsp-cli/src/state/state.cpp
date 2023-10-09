/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

std::ostream& operator<<(std::ostream& os, const base_state& state) {
  std::string str;
  switch (state.state_) {
    case State::kInit:
      str = "Init";
      break;

    case State::kLoggedIn:
      str = "LoggedIn";
      break;

    case State::kInRoom:
      str = "InRoom";
      break;

    case State::kExit:
      str = "Exit";
      break;
  }
  os << "[Client:" << &state << "] current state: " << str;
  return os;
}

}  // namespace state
}  // namespace cli
}  // namespace rsp
