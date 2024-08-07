/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

// https://www.cppstories.com/2021/stream-logger/
#pragma once
#include <source_location>

#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "rsplib/logger/color.hpp"
#include "rsplib/logger/flag.hpp"
#include "rsplib/logger/level.hpp"

namespace rsp {
namespace libs {
namespace logger {

#define L_location = location();

using streamable = std::ostream;

class s_logger {
 public:
  std::string executable_name() {
#if defined(PLATFORM_POSIX) || \
    defined(__linux__)  // check defines for your setup

    std::string sp;
    std::ifstream("/proc/self/comm") >> sp;
    return sp;

#elif defined(_WIN32)

    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return buf;

#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (_NSGetExecutablePath(buf, &bufsize)) return {};

    puts(buf);
    std::string path{buf};
    return path.substr(path.find_last_of('/') + 1);
#else
    static_assert(false, "unrecognized platform");

#endif
  }

  void activate(bool makeActive = true) {
    makeActive ? flags_ -= L_null : flags_ += L_null;
  }

  flags add_flag(flags flag) { return flags_ += flag; }
  flags remove_flag(flags flag) { return flags_ -= flag; }

  virtual void flush() {
    if (has_color()) stream() << "\x1b[0m";
    stream().flush();
    flags_ -= L_startWithFlushing;
  }

  virtual bool open() { return false; }

  template <typename T>
  s_logger &log(T value);

  s_logger &operator<<(flags);
  s_logger &operator<<(decltype(std::endl<char, std::char_traits<char>>)) {
    return *this << L_endl;
  }

  s_logger &operator<<(decltype(std::hex) manip) {
    stream() << manip;
    return *this;
  }

  s_logger &operator<<(decltype(std::setw) manip) {
    stream() << manip;
    return *this;
  }

  virtual s_logger &operator<<(log_level level) {
    streaming_level_ = level;
    if (has_color()) stream() << "\x1b[" << color_code(streaming_level_) << "m";
    // REMARK(@nolleh) color code reset will be performed when flush
    return *this;
  }

  s_logger &operator<<(const std::source_location &s) {
    if (!has_meet_level()) return *this;

    const auto name = [](const std::source_location &s) {
      const std::string filename = s.file_name();
      return filename.substr(filename.find_last_of('/') + 1);
    };
    stream() << name(s) << ":" << s.line();
    return *this;
  }

  virtual s_logger &print_level() = 0;

  s_logger &trace(std::source_location s = std::source_location::current()) {
    std::lock_guard<std::mutex> l(m_);
    *this << log_level::kTrace << L_time << L_space << executable_name_
          << "::" << s << L_space << L_level;
    return *this;
  }

  s_logger &debug(std::source_location s = std::source_location::current()) {
    std::lock_guard<std::mutex> l(m_);
    *this << log_level::kDebug << L_time << L_space << executable_name_
          << "::" << s << L_space << L_level;
    return *this;
  }

  s_logger &info(std::source_location s = std::source_location::current()) {
    std::lock_guard<std::mutex> l(m_);
    *this << log_level::kInfo << L_time << L_space << executable_name_
          << "::" << s << L_space << L_level;
    return *this;
  }

  s_logger &warn(std::source_location s = std::source_location::current()) {
    std::lock_guard<std::mutex> l(m_);
    *this << log_level::kWarn << L_time << L_space << executable_name_
          << "::" << s << L_space << L_level;
    return *this;
  }

  s_logger &error(std::source_location s = std::source_location::current()) {
    std::lock_guard<std::mutex> l(m_);
    *this << log_level::kError << L_time << L_space << executable_name_
          << "::" << s << L_space << L_level;
    return *this;
  }

  virtual streamable &stream() = 0;

  using ostream_ptr = streamable *;

  virtual s_logger *mirror_stream(ostream_ptr *mirror_stream) = 0;

 protected:
  explicit s_logger(flags initFlag = L_null,
                    log_level level = log_level::kTrace)
      : global_flags_{initFlag},
        flags_{initFlag},
        level_(level),
        executable_name_(executable_name()) {}
  explicit s_logger(flags initFlag = L_null, streamable & = std::clog,
                    log_level level = log_level::kTrace)
      : global_flags_{initFlag},
        flags_{initFlag},
        level_(level),
        executable_name_(executable_name()) {}

  virtual s_logger &log_time();

  s_logger &location(
      const std::source_location &s = std::source_location::current()) {
    *this << s;
    return *this;
  }

  template <class T>
  friend s_logger &operator<<(s_logger &s_logger, T value);
  bool is_space() const { return flags_ & L_space; }
  bool is_tabs() const { return flags_ & L_tabs || has_level(); }
  bool is_null() const { return flags_ == L_null; }
  bool is_cout() const { return flags_ & L_cout; }
  bool has_time() const { return (flags_ & L_time) == L_time; }
  bool has_level() const { return (flags_ & L_level) == L_level; }
  bool has_color() const { return (flags_ & L_color) == L_color; }
  bool has_meet_level() const { return streaming_level_ >= level_; }

  friend class file_name_generator;

  static tm *get_time();

  struct log_date {
    unsigned char day_no;
    unsigned char month_no;
  } inline static log_date{0, 0};

  flags global_flags_;
  flags flags_ = L_startWithFlushing;
  log_level level_;
  log_level streaming_level_ = log_level::kTrace;

 private:
  std::mutex m_;
  std::string executable_name_;
};

template <typename T>
s_logger &s_logger::log(T value) {
  if (is_null()) return *this;
  auto stream_ptr = &stream();
  s_logger *logger = this;
  do {
    if (is_space()) {
      *stream_ptr << ' ';
    }
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
s_logger &operator<<(s_logger &logger, T value) {
  if (!logger.has_meet_level()) return logger;
  return logger.log(value);
}

}  // namespace logger
}  // namespace libs
}  // namespace rsp
