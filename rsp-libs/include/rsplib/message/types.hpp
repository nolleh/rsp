#pragma once
#include <functional>
#include <memory>
#include <vector>

namespace rsp {
namespace libs {
namespace message {

using raw_buffer = std::vector<char>;
using buffer_ptr = std::shared_ptr<raw_buffer>;
using handler = std::function<void(buffer_ptr)>;

}  // namespace message
}  // namespace libs
}  // namespace rsp
