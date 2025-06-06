
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/state/factory.hpp"

#include "rspcli/state/state_init.hpp"
#include "rspcli/state/state_login.hpp"
#include "rspcli/state/state_in_room.hpp"
#include "rspcli/state/state_exit.hpp"

namespace rsp {
namespace cli {
namespace state {

std::map<State, creator> factory::s_warehouse = {
    {State::kInit, state_init::create},
    {State::kLoggedIn, state_login::create},
    {State::kInRoom, state_in_room::create},
    {State::kExit, state_exit::create}};

}  // namespace state
}  // namespace cli
}  // namespace rsp
