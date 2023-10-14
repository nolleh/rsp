/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <functional>
#include <memory>
#include <vector>

namespace rsp {
namespace libs {
namespace link {
class link;
}
namespace message {

using raw_buffer = std::vector<char>;
using buffer_ptr = std::shared_ptr<raw_buffer>;
using handler = std::function<void(buffer_ptr, rsp::libs::link::link*)>;

}  // namespace message
}  // namespace libs
}  // namespace rsp
