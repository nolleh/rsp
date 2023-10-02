// https://www.cppstories.com/2021/stream-logger/
#pragma once

#include <iomanip>
#include <iostream>
#include <string>

#include "rsplib/logger/flag.hpp"
#include "rsplib/logger/logger.hpp"

namespace rsp {
namespace libs {
namespace logger {
// enum class log_level { TRACE, DEBUG, INFO, WARN, ERROR };

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

  s_logger& operator<<(log_level level) {
    _streaming_level = level;
    return *this;
  }

  virtual streamable& stream() = 0;

  using ostream_ptr = streamable*;

  virtual s_logger* mirror_stream(ostream_ptr* mirror_stream) = 0;

 protected:
  explicit s_logger(flags initFlag = L_null, log_level level = log_level::TRACE)
      : _flags{initFlag}, _level(level) {}
  explicit s_logger(flags initFlag = L_null, streamable& = std::clog,
                    log_level level = log_level::TRACE)
      : _flags{initFlag}, _level(level) {}

  virtual s_logger& log_time();

  template <class T>
  friend s_logger& operator<<(s_logger& s_logger, T value);

  bool is_tabs() const { return _flags & L_tabs || has_time(); }
  bool is_null() const { return _flags == L_null; }
  bool is_cout() const { return _flags & L_cout; }
  bool has_time() const { return (_flags & 7) == L_time; }
  bool has_meet_level() const { return _streaming_level >= _level; }

  friend class file_name_generator;

  static tm* get_time();

  struct log_date {
    unsigned char dayNo;
    unsigned char monthNo;
  } inline static log_date{0, 0};

  flags _flags = L_startWithFlushing;
  log_level _level;
  log_level _streaming_level = log_level::TRACE;
};

template <typename T>
s_logger& s_logger::log(T value) {
  if (is_null()) return *this;
  auto stream_ptr = &stream();
  s_logger* logger = this;
  do {
    if (is_tabs()) {
      *stream_ptr << "\t";
    }
    *stream_ptr << value;
    logger = logger->mirror_stream(&stream_ptr);
  } while (stream_ptr);
  remove_flag(L_time);
  return *this;
}

template <typename T>
s_logger& operator<<(s_logger& logger, T value) {
  if (!logger.has_meet_level()) return logger;
  return logger.log(value);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
