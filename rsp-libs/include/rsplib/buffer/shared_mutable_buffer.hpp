#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

namespace rsp {
namespace libs {
namespace buffer {

// A reference-counted modifiable buffer class.
class shared_mutable_buffer {
 public:
  // Construct from a std::string.
  explicit shared_mutable_buffer(const std::string& data)
      : data_(new std::vector<char>(data.begin(), data.end())),
        buffer_(boost::asio::buffer(*data_)) {}
  explicit shared_mutable_buffer(std::vector<char> data)
      : data_(new std::vector<char>(data)),
        buffer_(boost::asio::buffer(*data_)) {}

  // Implement the ConstBufferSequence requirements.
  typedef boost::asio::mutable_buffer value_type;
  typedef const boost::asio::const_buffer* const_iterator;
  const boost::asio::mutable_buffer* begin() const { return &buffer_; }
  const boost::asio::mutable_buffer* end() const { return &buffer_ + 1; }

 private:
  std::shared_ptr<std::vector<char>> data_;
  boost::asio::mutable_buffer buffer_;
};
}  // namespace buffer
}  // namespace libs
}  // namespace rsp