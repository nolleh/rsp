
/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#include "rspcli/state/factory.hpp"
namespace rsp {
namespace cli {
namespace state {

std::map<State, creator> factory::s_warehouse = {
    {State::kInit, base_state::create},
    {State::kLoggedIn, state_login::create}};

}
}  // namespace cli
}  // namespace rsp
