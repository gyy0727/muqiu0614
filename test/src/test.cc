/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-27 19:24:26
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-02-28 15:43:29
 * @FilePath: /桌面/sylar/test/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "log.h"
#include <climits>
#include <iostream>
#include <memory>
#include <sstream>
using namespace sylar;
using namespace std;
class b;
class b {
  public:
  b() {}
  stringstream &content() { return ss; }
  string get() { return ss.str(); }
  stringstream ss;
};
class a {
  public:
  ~a() { cout << "fewhfioewhof" << d.get(); }
  b d;
};

class c {};
int main() {
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender ::ptr(new sylar
    ::StdoutLogAppender)); sylar::FileLogAppender::ptr file_appender(
        new sylar ::FileLogAppender("./log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar ::LogFormatter("dTpmn"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel ::ERROR);
    logger->addAppender(file_appender);

    std::cout << "hello sylar log" << std::endl;
    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";

    // sylar::Logger::ptr logger1(new sylar::Logger);
    // logger1->addAppender(sylar::LogAppender::ptr(new
    // sylar::StdoutLogAppender)); sylar::LogEvent::ptr event(new
    // sylar::LogEvent(logger1, sylar::LogLevel::DEBUG,
    //                                                __FILE__, __LINE__, 0,
    //                                                100, 1000, time(0),
    //                                                "name"));
    // logger1->log(sylar::LogLevel::DEBUG, event);
  // a A;
  
  // A.d.content() << "sss";
 
  return 0;
}