/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-05 12:44:43
 * @FilePath     : /muqiu0614/include/Fiber.h
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */
#ifndef _FIBER_H
#define _FIBER_H
#include "Log.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <sys/ucontext.h>
#include <ucontext.h>
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
  using ptr = std::shared_ptr<Fiber>;
  enum STATE {
    INIT,   //*初始化状态
    HOLD,   //*挂起
    RUNING, //*运行中
    TERM,
    READY,
    EXCEPT
  };
  Fiber();
  Fiber(std::function<void()> cb, size_t stacksize = 0, bool usecaller = false);
  ~Fiber();
  void reset(std::function<void()> cb);
  void swapIn();
  void swapOut();
  void call();
  void back();
  uint64_t getId() const { return m_id; }
  STATE getState() const { return m_state; }
  static void setThis(Fiber *f);

  static Fiber::ptr getThis();

  static uint64_t totalFibers();

  static void mainFunc();

  static void callerMainFunc();
  static uint64_t getFiberId();

private:
  uint64_t m_id;
  uint32_t m_stackSize = 0;
  STATE m_state = INIT;
  ucontext_t m_context;
  void *m_stack;
  std::function<void()> m_cb;
};
#endif
