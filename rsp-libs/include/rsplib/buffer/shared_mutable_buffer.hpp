/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

#pragma once
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace rsp {
namespace libs {

// A reference-counted modifiable buffer class.
class shared_mutable_buffer {
 public:
  // Construct from a std::string.
  explicit shared_mutable_buffer(const std::string& data)
      : data_(new std::vector<char>(data.begin(), data.end())),
        buffer_(boost::asio::buffer(*data_)) {}
  explicit shared_mutable_buffer(std::vector<char> data)
      : data_(new std::vector<char>(data.begin(), data.end())),
        buffer_(boost::asio::buffer(*data_)) {}

  // Implement the ConstBufferSequence requirements.
  typedef boost::asio::mutable_buffer value_type;
  typedef const boost::asio::const_buffer* const_iterator;
  const boost::asio::mutable_buffer* begin() const { return &buffer_; }
  const boost::asio::mutable_buffer* end() const { return &buffer_ + 1; }
  std::vector<char>::iterator data_begin() const { return data_->begin(); }
  std::vector<char>::iterator data_end() const { return data_->end(); }
  size_t size() { return data_->size(); }

 private:
  std::shared_ptr<std::vector<char>> data_;
  boost::asio::mutable_buffer buffer_;
};
}  // namespace libs
}  // namespace rsp
