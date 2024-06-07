/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-06 17:26:54
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 18:27:49
 * @FilePath     : /muqiu0614/src/test_fiber.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */
#include "../include/Fiber.h"
#include "../include/PThread.h"
#include <iostream>
#include <unistd.h>
void test_fiber1() {
  std::cout << "子协程出现" << std::endl;
  Fiber::getThis()->swapOut();
  std::cout << "子协程再次出现" << std::endl;
  // Fiber::getThis()->swapOut();
}
void test_fiber2() {}
void test_Thread() {
  std::cout << "thread_test " << std::endl;
  std::cout << "主协程创建" << std::endl;
  Fiber::ptr f1(new Fiber());
  std::cout << "子协程创建" << std::endl;
  Fiber::ptr f2(new Fiber(&test_fiber1, 1024 * 128));
  f2->swapIn();
  std::cout << "切换到主协程" << std::endl;
  f2->swapIn();

}

int main() {

  Sylar::PThread thread_(&test_Thread, "thread_1");
  sleep(10);
  return 0;
}
