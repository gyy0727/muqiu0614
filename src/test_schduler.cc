#include "../include/Log.h"
#include "../include/scheduler.h"
#include <iostream>
using namespace Sylar;
Logger::ptr g_logger = SYLAR_LOG_NAME("system");
void test() { SYLAR_LOG_INFO(g_logger) << "test"; }
int main() {
  test();
  return 0;
}
