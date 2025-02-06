//==============================================================================
//
//  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#pragma once

#include <atomic>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <mutex>
#include <android/log.h>

#include "QnnLog.h"

#define __FILENAME__ (strrchr(__FILE__, '/') + 1)


// Logging codes for this project
#define LOG_TAG "QNN"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)


auto logExecutionTime = [](const std::string &stage, auto &&func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    LOGD("%s time: %.3f ms", stage.c_str(), duration.count());
};


/**
 * @brief Log something with the current logger. Always valid to call, though
 *        it won't do something if no logger has been set.
 */

//#define QNN_LOG_LEVEL(level, fmt, ...)                                \
//  do {                                                                \
//    auto logger = ::qnn::log::Logger::getLogger();                    \
//    if (logger) {                                                     \
//      logger->log(level, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__); \
//    }                                                                 \
//  } while (0)


#define QNN_LOG_TAG "QNN"

//#define LOG_TAG "QnnLog"
//#define QNN_INFO(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define QNN_ERROR(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
//#define QNN_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#define QNN_LOG_LEVEL(level, fmt, ...)                                  \
  do {                                                                  \
    ::qnn::log::Logger::logToAndroid(level, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__); \
  } while (0)


#define QNN_ERROR(fmt, ...) QNN_LOG_LEVEL(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)
#define QNN_ERROR_EXIT(fmt, ...)   \
  {                                \
    QNN_ERROR(fmt, ##__VA_ARGS__); \
    exit(EXIT_FAILURE);            \
  }

#define QNN_WARN(fmt, ...) QNN_LOG_LEVEL(ANDROID_LOG_WARN, fmt, ##__VA_ARGS__)
#define QNN_INFO(fmt, ...) QNN_LOG_LEVEL(ANDROID_LOG_INFO, fmt, ##__VA_ARGS__)
#define QNN_DEBUG(fmt, ...) QNN_LOG_LEVEL(ANDROID_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define QNN_VERBOSE(fmt, ...) QNN_LOG_LEVEL(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)

#define QNN_FUNCTION_ENTRY_LOG QNN_LOG_LEVEL(ANDROID_LOG_VERBOSE, "Entering %s", __func__)
#define QNN_FUNCTION_EXIT_LOG QNN_LOG_LEVEL(ANDROID_LOG_VERBOSE, "Returning from %s", __func__)

//#define QNN_ERROR(fmt, ...) QNN_LOG_LEVEL(QNN_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
//
//#define QNN_ERROR_EXIT(fmt, ...)   \
//  {                                \
//    QNN_ERROR(fmt, ##__VA_ARGS__); \
//    exit(EXIT_FAILURE);            \
//  }

//#define QNN_WARN(fmt, ...) QNN_LOG_LEVEL(QNN_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
//
//#define QNN_INFO(fmt, ...) QNN_LOG_LEVEL(QNN_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
//
//#define QNN_DEBUG(fmt, ...) QNN_LOG_LEVEL(QNN_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
//
//#define QNN_VERBOSE(fmt, ...) QNN_LOG_LEVEL(QNN_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
//
//#define QNN_FUNCTION_ENTRY_LOG QNN_LOG_LEVEL(QNN_LOG_LEVEL_VERBOSE, "Entering %s", __func__)
//
//#define QNN_FUNCTION_EXIT_LOG QNN_LOG_LEVEL(QNN_LOG_LEVEL_VERBOSE, "Returning from %s", __func__)

namespace qnn {
namespace log {

bool initializeLogging();

QnnLog_Callback_t getLogCallback();

QnnLog_Level_t getLogLevel();

bool isLogInitialized();

bool setLogLevel(QnnLog_Level_t maxLevel);

class Logger final {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(Logger&&) = delete;

  void setMaxLevel(QnnLog_Level_t maxLevel) {
    m_maxLevel.store(maxLevel, std::memory_order_seq_cst);
  }

  QnnLog_Level_t getMaxLevel() { return m_maxLevel.load(std::memory_order_seq_cst); }

  QnnLog_Callback_t getLogCallback() { return m_callback; }

  void log(QnnLog_Level_t level, const char* file, long line, const char* fmt, ...);

  static std::shared_ptr<Logger> createLogger(QnnLog_Callback_t callback,
                                              QnnLog_Level_t maxLevel,
                                              QnnLog_Error_t* status);

  static bool isValid() { return (s_logger != nullptr); }

  static std::shared_ptr<Logger> getLogger() { return s_logger; }

  static void reset() { s_logger = nullptr; }

    static void logToAndroid(int level, const char* file, long line, const char* fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        __android_log_print(level, QNN_LOG_TAG, "%s:%ld: %s", file, line, buffer);
    }
 private:
  Logger(QnnLog_Callback_t callback, QnnLog_Level_t maxLevel, QnnLog_Error_t* status);

  uint64_t getTimestamp() const;

  QnnLog_Callback_t m_callback;
  std::atomic<QnnLog_Level_t> m_maxLevel;
  uint64_t m_epoch;
  static std::shared_ptr<Logger> s_logger;
  static std::mutex s_logMutex;
};

}  // namespace log
}  // namespace qnn
