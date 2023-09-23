#pragma once

#include <iomanip>
#include <iostream>

#include "logger/flag.hpp"

namespace rsp {
namespace libs {
namespace logger {

using streamable = std::ostream;

class s_logger {
 public:
  void activate(bool makeActive = true) {
    makeActive ? _flags -= L_null : _flags += L_null;
  }

  flags add_flag(flags flag) { return _flags += flag; }
  flags remove_flag(flags flag) { return _flags -= flag; }

  virtual void flush() {
    stream().flush();
    _flags -= L_startWithFlushing;
  }

  virtual bool open() { return false; }

  template <typename T>
  s_logger& log(T value);

  s_logger& operator<<(flags);
  s_logger& operator<<(decltype(std::endl<char, std::char_traits<char>>)) {
    return *this << L_endl;
  }

  s_logger& operator<<(decltype(std::hex) manip) {
    stream() << manip;
    return *this;
  }

  s_logger& operator<<(decltype(std::setw) manip) {
    stream() << manip;
    return *this;
  }

  virtual streamable& stream();

  using ostream_ptr = streamable*;

  virtual s_logger* mirror_stream(ostream_ptr& mirror_stream) {
    mirror_stream = nullptr;
    return this;
  }

 protected:
  s_logger(flags initFlag = L_null) : _flags{initFlag} {}
  s_logger(flags initFlag = L_null, streamable& = std::clog)
      : _flags{initFlag} {}

  virtual s_logger& log_time();

  template <class T>
  friend s_logger& operator<<(s_logger& s_logger, T value);

  bool is_tabs() const { return _flags & L_tabs || has_time(); }
  bool is_null() const { return _flags == L_null; }
  bool is_cout() const { return _flags & L_cout; }
  bool has_time() const { return (_flags & 7) == L_time; }

  friend class file_name_generator;

  static tm* get_time();

  struct log_date {
    unsigned char dayNo;
    unsigned char monthNo;
  } inline static log_date{0, 0};

  flags _flags = L_startWithFlushing;
};
}  // namespace logger
}  // namespace libs
}  // namespace rsp
