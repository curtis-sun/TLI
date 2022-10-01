/**
 * The logging utilities used in libRDMA.
 */

#pragma once

#include <iostream>
#include <sstream>

namespace finedex {

/**
 * \def FATAL
 *   Used for fatal and probably irrecoverable conditions
 * \def ERROR
 *   Used for errors which are recoverable within the scope of the function
 * \def WARNING
 *   Logs interesting conditions which are probably not fatal
 * \def EMPH
 *   Outputs as INFO, but in WARNING colors. Useful for
 *   outputting information you want to emphasize.
 * \def INFO
 *   Used for providing general useful information
 * \def DEBUG
 *   Debugging purposes only
 * \def EVERYTHING
 *   Log everything
 */

enum loglevel {
  NONE = 7,
  FATAL = 6,
  ERROR = 5,
  WARNING = 4,
  EMPH = 3,
  INFO = 2,
  DEBUG = 1,
  EVERYTHING = 0
};

#define unlikely(x) __builtin_expect(!!(x), 0)

#ifndef LOG_LEVEL
#define LOG_LEVEL ::finedex::INFO
#endif

// logging macro definiations
// default log
#define LOG(n)                                                            \
  if (n >= LOG_LEVEL)                                                     \
  ::finedex::MessageLogger((char *)__FILE__, __LINE__, n).stream()

// log with tag
#define TLOG(n, t)                                                        \
  if (n >= LOG_LEVEL)                                                     \
  ::finedex::MessageLogger((char *)__FILE__, __LINE__, n).stream()              \
      << "[" << (t) << "]"

#define LOG_IF(n, condition)                                              \
  if (n >= LOG_LEVEL && (condition))                                      \
  ::finedex::MessageLogger((char *)__FILE__, __LINE__, n).stream()

#define ASSERT(condition)                                                 \
  if (unlikely(!(condition)))                                                  \
  ::finedex::MessageLogger((char *)__FILE__, __LINE__, ::finedex::FATAL + 1)     \
          .stream()                                                            \
      << "Assertion! "

#define VERIFY(n, condition) LOG_IF(n, (!(condition)))

class MessageLogger {
public:
  MessageLogger(const char *file, int line, int level) : level_(level) {
    if (level_ < LOG_LEVEL)
      return;
    stream_ << "[" << StripBasename(std::string(file)) << ":" << line << "] ";
  }

  ~MessageLogger() {
    if (level_ >= LOG_LEVEL) {
      stream_ << "\n";
      std::cout << "\033[" << DEBUG_LEVEL_COLOR[std::min(level_, 6)] << "m"
                << stream_.str() << EndcolorFlag();
      if (level_ >= ::finedex::FATAL)
        abort();
    }
  }

  // Return the stream associated with the logger object.
  std::stringstream &stream() { return stream_; }

private:
  std::stringstream stream_;
  int level_;

  // control flags for color
  enum {
    R_BLACK = 39,
    R_RED = 31,
    R_GREEN = 32,
    R_YELLOW = 33,
    R_BULE = 34,
    R_MAGENTA = 35,
    R_CYAN = 36,
    R_WHITE = 37
  };

  const int DEBUG_LEVEL_COLOR[7] = {R_BLACK,   R_YELLOW, R_BLACK, R_GREEN,
                                         R_MAGENTA, R_RED,    R_RED};

  static std::string StripBasename(const std::string &full_path) {
    const char kSeparator = '/';
    size_t pos = full_path.rfind(kSeparator);
    if (pos != std::string::npos) {
      return full_path.substr(pos + 1, std::string::npos);
    } else {
      return full_path;
    }
  }

  static std::string EndcolorFlag() {
    char flag[7];
    snprintf(flag, 7, "%c[0m", 0x1B);
    return std::string(flag);
  }
};

} // end namespace findex
