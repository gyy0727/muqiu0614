/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 17:09:22
 * @FilePath     : /muqiu0614/src/test _scheduler.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */
/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-27 19:24:26
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-17 01:39:05
 * @FilePath: /桌面/sylar/test/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Log.h"
#include "../include/Manager.h"
#include "../include/scheduler.h"
#if 0
static Sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
  int a;
  for (int i = 0; i < 20000; i++) {
    a = a + i;
  }
  SYLAR_LOG_INFO(Sylar::ManagerLog()("system"))
      << Sylar::GetThreadId() << "a= " << a;
  //   sleep(5);
}

int main() {
  SYLAR_LOG_INFO(Sylar::ManagerLog()("system")) << "main";
  Sylar::Scheduler sc(3, false, "test");
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.submitTask(&test_fiber);
  sc.start();
  sleep(2);
  SYLAR_LOG_INFO(Sylar::ManagerLog()("system")) << "schedule";

  sc.stop();
  SYLAR_LOG_INFO(Sylar::ManagerLog()("system")) << "over";
  return 0;
}
#endif
