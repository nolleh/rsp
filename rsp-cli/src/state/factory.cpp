
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/state/factory.hpp"

#include "rspcli/state/state_login.hpp"
#include "rspcli/state/state_exit.hpp"

namespace rsp {
namespace cli {
namespace state {

std::map<State, creator> factory::s_warehouse = {
    {State::kInit, base_state::create},
    {State::kLoggedIn, state_login::create},
    {State::kExit, state_exit::create}};

}
}  // namespace cli
}  // namespace rsp
