#include "../include/Log.h"
#include "../include/scheduler.h"
#include <cstdint>
#include <functional>
#include <iostream>
using namespace Sylar;
void func();
int thread_cout = 2;
std::string name = "调度器";

Scheduler *s = new Scheduler(thread_cout, name);
Logger::ptr g_logger = SYLAR_LOG_NAME("system");

void func() { SYLAR_LOG_INFO(g_logger) << "fiber is running"; }
void test() {
  SYLAR_LOG_INFO(g_logger) << "test";
  static int s_count = 5;
  for (int i = 0; i < s_count; i++) {
    s->scheduler(&func, -1);
  }
  s->start();
  sleep(5);
  s->stop();
}
int main() {
  test();
  sleep(10);
  SYLAR_LOG_INFO(g_logger) << "main term";
  return 0;
}
