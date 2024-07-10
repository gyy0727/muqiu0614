/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-25 15:26:41
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-16 20:59:05
 * @FilePath: /桌面/sylar/include/Log.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include<mutex>
#include "Mutex.h"
#include "Singleton.h"
#include "util.h"
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <vector>
#define SYLAR_LOG_LEVEL(logger, level)                                         \
  if (logger->getLevel() <= level)                                             \
  Sylar::LogEventWrap(                                                         \
      Sylar::LogEvent::ptr(new Sylar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, Sylar::GetThreadId(),          \
          Sylar::GetFiberId(), time(0), "日志器01")))                          \
      .getSS()
#define SYLAR_LOG_NAME(name) Sylar::LoggerMgr::getInstance()->getLogger(name)

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, Sylar::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, Sylar::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, Sylar::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, Sylar::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, Sylar::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                           \
  if (logger->getLevel() <= level)                                             \
  Sylar::LogEventWrap(                                                         \
      Sylar::LogEvent::ptr(new Sylar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, Sylar::GetThreadId(),          \
          Sylar::GetFiberId(), time(0), Sylar::Thread::GetName())))            \
      .getEvent()                                                              \
      ->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...)                                  \
  SYLAR_LOG_FMT_LEVEL(logger, Sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)                                   \
  SYLAR_LOG_FMT_LEVEL(logger, Sylar::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)                                   \
  SYLAR_LOG_FMT_LEVEL(logger, Sylar::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...)                                  \
  SYLAR_LOG_FMT_LEVEL(logger, Sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...)                                  \
  SYLAR_LOG_FMT_LEVEL(logger, Sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define SYLAR_LOG_ROOT() Sylar::LoggerMgr::getInstance()->getRoot()

namespace Sylar {

// 日志事件
class Logger;

/**
 * @description: 日志级别
 */
class LogLevel {
public:
  enum Level {
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5

  };
  //*将日志级别转换成文本内容
  static const char *ToString(LogLevel::Level level);
  static LogLevel::Level FromString(const std::string &str);
};

/**
 * @description: 包装日志事件
 */
class LogEvent {
public:
  //*起别名,ptr代指LogEvent的智能指针
  using ptr = std::shared_ptr<LogEvent>;
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level,
           const char *fliename, int32_t line, uint32_t elapse,
           uint32_t thread_id, uint32_t fiber_id, uint64_t time,
           const std::string &thread_name);

  const char *getFile() const { return m_file; }

  int32_t getLine() const { return m_line; }

  uint32_t getElapse() const { return m_elapse; }

  uint32_t getThreadId() const { return m_threadId; }

  uint32_t getFiberId() const { return m_fiberId; }

  uint64_t getTime() const { return m_time; }

  std::string getContent() const {
    return m_ss.str();
  }

  std::shared_ptr<Logger> getLogger() const { return m_logger; }

  LogLevel::Level getLevel() const { return m_level; }

  std::stringstream &getSS() { return m_ss; }

  std::string getThreadName() { return m_threadName; }

  void format(const char *fmt, ...);

  void format(const char *fmt, va_list al);

private:
    std::mutex m_mutex;
  const char *m_file = nullptr;     // *文件名
  int32_t m_line = 0;               // *行号
  uint32_t m_threadId = 0;          // *线程id
  uint32_t m_elapse = 0;            // *程序启动开始到现在的毫秒数
  uint32_t m_fiberId = 0;           // *协程id
  uint64_t m_time = 0;              // *时间戳
  std::string m_threadName;         //*线程名称
  std::stringstream m_ss;           //*日志内容
  std::shared_ptr<Logger> m_logger; //*日志器
  LogLevel::Level m_level;          //*日志级别
};

class LogEventWrap {
public:
  LogEventWrap(LogEvent::ptr e);

  ~LogEventWrap();

  LogEvent::ptr getEvent() const { return m_event; }

  std::stringstream &getSS();

private:
    std::mutex m_mutex;
  LogEvent::ptr m_event;
};
// *确定日志格式

class LogFormatter {
  /**
   *  %m 消息
   *  %p 日志级别
   *  %r 累计毫秒数
   *  %c 日志名称
   *  %t 线程id
   *  %n 换行
   *  %d 时间
   *  %f 文件名
   *  %l 行号
   *  %T 制表符
   *  %F 协程id
   *  %N 线程名称
   *
   *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
   */
public:
  using ptr = std::shared_ptr<LogFormatter>;

  std::string format(std::shared_ptr<Logger> logger, LogLevel::Level,
                     LogEvent::ptr event);
  std::ostream &format(std::ostream &ofs, std::shared_ptr<Logger> logger,
                       LogLevel::Level level, LogEvent::ptr event);
  LogFormatter(const std::string &pattern);

  class FormatItem {
  public:
    using ptr = std::shared_ptr<FormatItem>;
    // virtual ~FormatItem();

    virtual void format(std::ostream &os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) = 0;
  };

  bool isError() const { return m_error; }

  /**
   * @brief 返回日志模板
   */
  const std::string getPattern() const { return m_pattern; }

private:
  void init();
    std::mutex m_mutex;
  /// 是否有错误
  bool m_error = false;
  std::vector<FormatItem::ptr> m_item; //*格式被解析之后的内容
  std::string m_pattern;               //*用户自定义的格式
};

// *日志输出地
class LogAppender {
public:
  using Mutex = Spinlock;
  using ptr = std::shared_ptr<LogAppender>;

  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;

  void setFormatter(LogFormatter::ptr val) { m_logformatter = val; }

  LogFormatter::ptr getFormatter() const { return m_logformatter; }

  LogLevel::Level getLevel() const { return m_level; }

  bool hasFormatter() { return m_hasFormatter; }

  void setLevel(LogLevel::Level val) { m_level = val; }
  virtual std::string toJsonString() = 0;

protected:
  //*日志级别
    std::mutex m_mutex;
  LogLevel::Level m_level = LogLevel::DEBUG;

  //* 是否有自己的日志格式器
  bool m_hasFormatter = false;

  //*日志格式器
  LogFormatter::ptr m_logformatter;
};

//*日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
  using Mutex = Spinlock;
  using ptr = std::shared_ptr<Logger>;

  Logger(const std::string &name = "root");

  void log(LogLevel::Level level, LogEvent::ptr event);

  void debug(LogEvent::ptr event);

  void info(LogEvent::ptr event);

  void warn(LogEvent::ptr event);

  void fatal(LogEvent::ptr event);

  void error(LogEvent::ptr event);
  std::string toJsonString();
  //*添加及删除日志器
  void addAppender(LogAppender::ptr appender);

  void delAppender(LogAppender::ptr appender);

  LogLevel::Level getLevel() const { return m_level; }

  void setLevel(LogLevel::Level level) { m_level = level; }

  void clearAppenders();

  const std::string &getName() const { return m_name; }
  Logger::ptr getRoot() { return m_root; }

  void setFormatter(LogFormatter::ptr val);

  void setFormatter(const std::string &val);

  LogFormatter::ptr getFormatter();

private:
    std::mutex m_mutex;
  std::string m_name;                      //*日志名称
  LogLevel::Level m_level;                 //*日志级别
  std::list<LogAppender::ptr> m_appenders; //*Appender集合
  LogFormatter::ptr m_formatter;
  Logger::ptr m_root = nullptr;
};

//*输出到控制台
class StdoutLogAppender : public LogAppender {
public:
  StdoutLogAppender();

  using ptr = std::shared_ptr<StdoutLogAppender>;
  std::string toJsonString() override;
  void log(Logger::ptr logger, LogLevel::Level level,
           LogEvent::ptr event) override;
};

//*输出到文件
class FileLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<FileLogAppender>;

  FileLogAppender(const std::string &fliename);

  void log(Logger::ptr logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  std::string toJsonString() override;
  bool reopen();

private:
    std::mutex m_mutex;
  std::string m_name;
  std::ofstream m_filestream;
  uint64_t m_lastTime = 0;
};
class LoggerManager {
public:
  using Mutex = Spinlock;
  LoggerManager();

  Logger::ptr getLogger(const std::string &name);

  void init();

  Logger::ptr getRoot() const { return m_root; }

  std::string toJsonString();

private:
  /// Mutex
    std::mutex m_mutex;
  /// 日志器容器
  std::map<std::string, Logger::ptr> m_loggers;
  /// 主日志器
  Logger::ptr m_root;
};

// 日志器管理类单例模式
using LoggerMgr = Sylar::Singleton<LoggerManager>;

} // namespace Sylar
// namespace Sylar
