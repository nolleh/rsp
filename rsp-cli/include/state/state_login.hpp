#pragma once
#include "state/state.hpp"

namespace rsp {
namespace cli {
namespace state {

class state_login : public base_state {
 public:
  State state = State::LOGGED_IN;
  base_state* handle_buffer(std::array<char, 128> buf, size_t len) {
    std::cout << "loggedin state handle buffer" << std::endl;
    return this;
  }

  template <typename Message>
  class base_state* handle_message(Message&& message) {
    std::cout << "loggedin state handleMessage" << std::endl;
    return this;
  }
};
}  // namespace state
}  // namespace cli
}  // namespace rsp
