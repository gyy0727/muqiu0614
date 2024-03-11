
/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-25 15:26:34
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 21:49:57
 * @FilePath: /桌面/sylar/src/Log.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Log.h"
#include "../include/Config.h"

namespace Sylar {

Logger::Logger(const std::string &name)
    : m_name(name), m_level(LogLevel::DEBUG) {
  m_formatter.reset(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));

  m_appenders.push_back(Sylar::LogAppender::ptr(new StdoutLogAppender()));
  m_appenders.push_back(
      Sylar::LogAppender::ptr(new FileLogAppender("./Log.txt")));
  this->setFormatter(m_formatter);
}
/**
 * @description:将日志级别转为字符串,方便打印时获取
 * @param {Level} level
 * @return {返回日志日志级别的字符串形式}
 */
const char *LogLevel::ToString(LogLevel::Level level) {
  switch (level) {
#define GETNAME(name)                                                          \
  case LogLevel::name:                                                         \
    return #name;                                                              \
    break;
    GETNAME(DEBUG);
    GETNAME(INFO);
    GETNAME(FATAL);
    GETNAME(ERROR);
    GETNAME(WARN);
#undef GETNAME
  default:
    return "UNKNOW";
  }
}
LogLevel::Level LogLevel::FromString(const std::string &str) {
#define XX(level, v)                                                           \
  if (str == #v) {                                                             \
    return level;                                                              \
  }
  XX(DEBUG, debug);
  XX(INFO, info);
  XX(WARN, warn);
  XX(ERROR, error);
  XX(FATAL, fatal);

  XX(DEBUG, DEBUG);
  XX(INFO, INFO);
  XX(WARN, WARN);
  XX(ERROR, ERROR);
  XX(FATAL, FATAL);
  return LogLevel::UNKNOW;
#undef XX
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   const char *fliename, int32_t line, uint32_t elapse,
                   uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                   const std::string &thread_name)
    : m_file(fliename), m_line(line), m_threadId(thread_id), m_elapse(elapse),
      m_fiberId(fiber_id), m_time(time), m_threadName(thread_name),
      m_logger(logger), m_level(level) {}

void LogEvent::format(const char *fmt, ...) {
  va_list al;
  va_start(al, fmt);
  format(fmt, al);
  va_end(al);
}

void LogEvent::format(const char *fmt, va_list al) {
  char *buf = nullptr;
  int len = vasprintf(&buf, fmt, al);
  if (len != -1) {
    m_ss << std::string(buf, len);
    free(buf);
  }
}
LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern) {
  init();
}
std::string LogFormatter::format(std::shared_ptr<Logger> logger,
                                 LogLevel::Level level, LogEvent::ptr event) {
  std::stringstream ss;
  for (auto &i : m_item) {
    i->format(ss, logger, level, event);
  }
  return ss.str();
}

std::ostream &LogFormatter::format(std::ostream &ofs,
                                   std::shared_ptr<Logger> logger,
                                   LogLevel::Level level, LogEvent::ptr event) {
  for (auto &i : m_item) {
    i->format(ofs, logger, level, event);
  }
  return ofs;
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
  MessageFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getContent();
  }
};
LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {}

LogEventWrap::~LogEventWrap() {
  m_event->getLogger()->log(m_event->getLevel(), m_event);
}
std::stringstream &LogEventWrap::getSS() { return m_event->getSS(); }

class LevelFormatItem : public LogFormatter::FormatItem {
public:
  LevelFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << LogLevel::ToString(level);
  }
};
StdoutLogAppender::StdoutLogAppender() {}
class ElapseFormatItem : public LogFormatter::FormatItem {
public:
  ElapseFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getElapse();
  }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
  NameFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getLogger()->getName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
  ThreadIdFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getThreadId();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
  FiberIdFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFiberId();
  }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
  ThreadNameFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getThreadName();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
  DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
      : m_format(format) {
    if (m_format.empty()) {
      m_format = "%Y-%m-%d %H:%M:%S";
    }
  }

  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    struct tm tm;
    time_t time = event->getTime();
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), m_format.c_str(), &tm);
    os << buf;
  }

private:
  std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
  FilenameFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFile();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
  LineFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
  NewLineFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << std::endl;
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
  StringFormatItem(const std::string &str) : m_string(str) {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << m_string;
  }

private:
  std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
  TabFormatItem(const std::string &str = "") {}
  void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << "\t";
  }

private:
  std::string m_string;
};
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  auto self = shared_from_this();
  if (m_level <= level) {
    if (!m_appenders.empty()) {
      for (auto &i : m_appenders) {
        i->log(self, level, event);
      }
    } else if (m_root) {
      m_root->log(level, event);
    }
  }
}

void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::addAppender(LogAppender::ptr appender) {
  m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
    //*解引用之后才是appender类型的指针
    if (*it == appender) {
      m_appenders.erase(it);
      break;
    }
  }
}
void Logger::clearAppenders() { m_appenders.clear(); }

void Logger::setFormatter(LogFormatter::ptr val) {
  m_formatter = val;
  for (auto &i : m_appenders) {
    if (!i->hasFormatter()) {
      i->setFormatter(m_formatter);
    }
  }
}

void Logger::setFormatter(const std::string &val) {
  LogFormatter::ptr new_val(new LogFormatter(val));
  if (new_val->isError()) {
    std::cout << "Logger setFormatter name=" << m_name << " value=" << val
              << " invalid formatter" << std::endl;
    return;
  }
  // m_formatter = new_val;
  setFormatter(new_val);
}
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                            LogEvent::ptr event) {
  if (m_level <= level) {
    m_logformatter->format(std::cout, logger, level, event);
  }
}
FileLogAppender::FileLogAppender(const std::string &fliename)
    : m_name(fliename) {
  reopen();
}
void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level,
                          LogEvent::ptr event) {
  if(level >= m_level) {
        uint64_t now = event->getTime();
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        //if(!(m_filestream << m_formatter->format(logger, level, event))) {
        if(!m_logformatter->format(m_filestream, logger, level, event)) {
            std::cout << "error" << std::endl;
        }
    }
}
//*文件的重新打开
bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_name, std::ios::out | std::ios::app);
  //* !!是为了将m_filesttream转换为布尔值
  return !!m_filestream;
}

// namespace Sylar
void LogFormatter::init() {
  //*存储解析结果[格式化文本(如%d里面的d),解析出的内容
  // *(如"%Y-%m-%d %H:%M:%S"),是否是需要解析的内容(如"["等则为0)]
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr; //*非格式化文本如 "["
  for (size_t i = 0; i < m_pattern.size(); ++i) {
    //*检查第一个字符,不是%则可以确定为用户提供的非格式化文本,如[]
    if (m_pattern[i] != '%') {
      nstr.append(1, m_pattern[i]);
      continue;
    }
    //*走到这一步,证明第一个字符是%,检查第二个字符
    if (i + 1 < m_pattern.size()) {
      if (m_pattern[i + 1] == '%') {
        nstr.append(1, '%');
        vec.push_back(std::make_tuple(nstr, std::string(), 0));
        nstr.clear();
        i = i + 1;
        continue;
      }
    }
    size_t n = i + 1;
    int fmt_status = 0;
    size_t fmt_begin = 0;

    std::string str; //*存储格式化文本,如%d里面的d
    std::string fmt; //*存储%Y-%m-%d %H:%M:%S
    //*while函数负责解析字符串
    if (n < m_pattern.size()) {
      if (isalpha(m_pattern[n])) {
        str.append(1, m_pattern[n]);
        if (n + 1 < m_pattern.size() && !fmt_status &&
            m_pattern[n + 1] == '{') {
          fmt_status = 1;
          size_t m;
          m = n + 2;
          fmt_begin = m;
          while (m <= m_pattern.size()) {
            if (m_pattern[m] == '}') {
              fmt = m_pattern.substr(fmt_begin, m - fmt_begin);
              fmt_status = 0;
              i = m - 1;
              break;
            } else {
              m++;
            }
          }
          if (fmt_status == 1) {
            m_error = true;
            std::cout << "pattern parse error: " << m_pattern << " - "
                      << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple(str, "<<pattern_error>>", 1));
            if (!nstr.empty()) {
              vec.push_back(std::make_tuple(nstr, std::string(), 0));
              nstr.clear();
            }
            if (!str.empty() && !fmt.empty()) {
              vec.push_back(std::make_tuple(str, "error", 1));
              fmt.clear();
              str.clear();
            }
            // vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            i = i + 1;
            continue;
          }
        }
      }
    }
    i++;

    if (!nstr.empty()) {
      vec.push_back(std::make_tuple(nstr, std::string(), 0));
      nstr.clear();
    }
    if (!str.empty() && !fmt.empty()) {
      vec.push_back(std::make_tuple(str, fmt, 1));
      fmt.clear();
      str.clear();
    }
    if (!str.empty() && fmt.empty()) {
      vec.push_back(std::make_tuple(str, std::string(), 1));
      fmt.clear();
      str.clear();
    }
  }
  static std::map<std::string,
                  std::function<FormatItem::ptr(const std::string &str)>>
      formatterItems = {
#define XX(str, c)                                                             \
  {                                                                            \
    #str, [](const std::string &str) { return FormatItem::ptr(new c(str)); }   \
  }
          XX(m, MessageFormatItem),    // m:消息
          XX(p, LevelFormatItem),      // p:日志级别
          XX(r, ElapseFormatItem),     // r:累计毫秒数
          XX(c, NameFormatItem),       // c:日志名称
          XX(t, ThreadIdFormatItem),   // t:线程id
          XX(n, NewLineFormatItem),    // n:换行
          XX(d, DateTimeFormatItem),   // d:时间
          XX(f, FilenameFormatItem),   // f:文件名
          XX(l, LineFormatItem),       // l:行号
          XX(T, TabFormatItem),        // T:Tab
          XX(F, FiberIdFormatItem),    // F:协程id
          XX(N, ThreadNameFormatItem), // N:线程名称
#undef XX

      };
  for (auto &i : vec) {
    if (std::get<2>(i) == 0) {
      m_item.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
    } else {
      auto it = formatterItems.find(std::get<0>(i));
      if (it == formatterItems.end()) {
        m_item.push_back(FormatItem::ptr(
            new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
        m_error = true;
      } else {
        m_item.push_back(it->second(std::get<1>(i)));
      }
    }
  }
}
std::string Logger::toJsonString() {

  json node;
  node["name"] = m_name;
  if (m_level != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(m_level);
  }
  if (m_formatter) {
    node["formatter"] = m_formatter->getPattern();
  }

  for (auto &i : m_appenders) {
    node["appenders"].push_back(json::parse(i->toJsonString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}
std::string FileLogAppender::toJsonString() {
  json node;
  node["type"] = "FileLogAppender";
  node["file"] = m_name;
  if (m_level != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(m_level);
  }
  if (m_hasFormatter && this->getFormatter()) {
    node["formatter"] = this->getFormatter()->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}
LoggerManager::LoggerManager()  {
  m_root.reset(new Logger);
  // m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
  m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string &name) {

  auto it = m_loggers.find(name);
  if (it != m_loggers.end()) {
    return it->second;
  }

  Logger::ptr logger(new Logger(name));
  logger->getRoot() = m_root;
  m_loggers[name] = logger;
  return m_root;
}

struct LogAppenderDefine {
  int type = 0; // 1 File, 2 Stdout
  LogLevel::Level level = LogLevel::UNKNOW;
  std::string formatter;
  std::string file;

  bool operator==(const LogAppenderDefine &oth) const {
    return type == oth.type && level == oth.level &&
           formatter == oth.formatter && file == oth.file;
  }
};

std::string StdoutLogAppender::toJsonString() {
  json node;
  node["type"] = "StdoutLogAppender";
  if (m_level != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(m_level);
  }
  if (m_hasFormatter && this->getFormatter()) {
    node["formatter"] = this->getFormatter()->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

struct LogDefine {
  std::string name;
  LogLevel::Level level = LogLevel::UNKNOW;
  std::string formatter;
  std::vector<LogAppenderDefine> appenders;

  bool operator==(const LogDefine &oth) const {
    return name == oth.name && level == oth.level &&
           formatter == oth.formatter && appenders == appenders;
  }

  bool operator<(const LogDefine &oth) const { return name < oth.name; }

  bool isValid() const { return !name.empty(); }
};
template <> class LexicalCast<std::string, LogDefine> {
public:
  LogDefine operator()(const std::string &v) {

    json n = json::parse(v);
    LogDefine ld;
    auto name = n["name"];

    if (n.find("name") == n.end()) {
      std::cout << "log config error: name is null, " << n << std::endl;
      throw std::logic_error("log config name is null");
    }
    ld.name = n["name"];

    ld.level =
        LogLevel::FromString(n.find("level") != n.end() ? n["level"] : "");
    if (n.find("formatter") != n.end()) {
      ld.formatter = n["formatter"];
    }

    if (n.find("appender") != n.end()) {
      // std::cout << "==" << ld.name << " = " << n["appenders"].size() <<
      // std::endl;
     
      for (const auto &appender : n["appender"]) {
        auto a = appender;
      
        if (!a.contains("type")) {
          std::cout << "log config error: appender type is null, " << a
                    << std::endl;
          continue;
        }
        std::string type = a["type"];
        LogAppenderDefine lad;
        if (type == "FileLogAppender") {
          lad.type = 1;
          if (!a.contains("path")) {
            std::cout << "log config error: fileappender file is null, " << a
                      << std::endl;
            continue;
          }
          lad.file = a["path"];
          if (a.contains("formatter")) {
            lad.formatter = a["formatter"];
          }
        } else if (type == "StdoutLogAppender") {
          lad.type = 2;
          if (a.contains("formatter")) {
            lad.formatter = a["formatter"];
          }
        } else {
          std::cout << "log config error: appender type is invalid, " << a
                    << std::endl;
          continue;
        }
       
        ld.appenders.push_back(lad);
      }
    }

    return ld;
  }
};

template <> class LexicalCast<LogDefine, std::string> {
public:
  std::string operator()(const LogDefine &i) {
    json n;
    n["name"] = i.name;
    if (i.level != LogLevel::UNKNOW) {
      n["level"] = LogLevel::ToString(i.level);
    }
    if (!i.formatter.empty()) {
      n["formatter"] = i.formatter;
    }

    for (auto &a : i.appenders) {
      json na;
      if (a.type == 1) {
        na["type"] = "FileLogAppender";
        na["file"] = a.file;
      } else if (a.type == 2) {
        na["type"] = "StdoutLogAppender";
      }
      if (a.level != LogLevel::UNKNOW) {
        na["level"] = LogLevel::ToString(a.level);
      }

      if (!a.formatter.empty()) {
        na["formatter"] = a.formatter;
      }

      n["appenders"].push_back(na);
    }
    std::stringstream ss;
    ss << n;
    return ss.str();
  }
};
static Sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    Sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");


struct LogIniter {
  LogIniter() {
    g_log_defines->addListener([](const std::set<LogDefine> &old_value,
                                  const std::set<LogDefine> &new_value) {
      // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
      for (auto &i : new_value) {
        // std::cout << i.name << "   i" << std::endl;
        auto it = old_value.find(i);
        Sylar::Logger::ptr logger;
        if (it == old_value.end()) {
          // 新增logger
          logger = SYLAR_LOG_NAME(i.name);
        } else {
          if (!(i == *it)) {
            // 修改的logger
            logger = SYLAR_LOG_NAME(i.name);
          } else {
            continue;
          }
        }
        logger->setLevel(i.level);
        // std::cout << "** " << i.name << " level=" << i.level
        //<< "  " << logger << std::endl;

        logger->clearAppenders();
        for (auto &a : i.appenders) {
          Sylar::LogAppender::ptr ap;
          if (a.type == 1) {
            ap.reset(new FileLogAppender(a.file));
          } else if (a.type == 2) {
            ap.reset(new StdoutLogAppender);
          }
          ap->setLevel(a.level);
          if (!a.formatter.empty()) {
            std::cout << " fhwiehgiwehfwehigiuwgiweigew" << std::endl;
            LogFormatter::ptr fmt(new LogFormatter(a.formatter));
            if (!fmt->isError()) {
              ap->setFormatter(fmt);
            } else {
              std::cout << "log.name=" << i.name << " appender type=" << a.type
                        << " formatter=" << a.formatter << " is invalid"
                        << std::endl;
            }
          }
          logger->addAppender(ap);
        }
        if (!i.formatter.empty()) {
          logger->setFormatter(i.formatter);
        }
      }

      for (auto &i : old_value) {
        auto it = new_value.find(i);
        if (it == new_value.end()) {
          // 删除logger
          auto logger = SYLAR_LOG_NAME(i.name);
          logger->setLevel((LogLevel::Level)0);
          logger->clearAppenders();
        }
      }
    });
  }
};

static LogIniter __log_init;

std::string LoggerManager::toJsonString() {
  json node;
  for (auto &i : m_loggers) {
    node.push_back(json::parse(i.second->toJsonString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

void LoggerManager::init() {

  std::ifstream i("/home/muqiu0614/桌面/sylar/src/log.json");
  json root = json::parse(i);
  Sylar::Config::LoadFromJson(root);
}

} // namespace Sylar