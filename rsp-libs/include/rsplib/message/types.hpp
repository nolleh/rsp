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
using handler = std::function<void(buffer_ptr)>;
using handler2 = std::function<void(buffer_ptr, rsp::libs::link::link*)>;

}  // namespace message
}  // namespace libs
}  // namespace rsp
