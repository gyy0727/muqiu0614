/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 14:21:27
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
    TERM, //* 终止
    READY, //*待执行
    EXCEPT//*异常
  };
  Fiber();
  Fiber(std::function<void()> cb, size_t stacksize = 0);
  ~Fiber();
  void reset(std::function<void()> cb);
  void swapIn();
    void setState(Fiber::STATE state){
        m_state = state;
    }
  void swapOut();
  uint64_t getId() const { return m_id; }
  STATE getState() const { return m_state; }
  static void setThis(Fiber *f);

  static Fiber::ptr getThis();

  static uint64_t totalFibers();

  static void mainFunc();

  static uint64_t getFiberId();

private:
  uint64_t m_id; //* 协程id 
  uint32_t m_stackSize = 0;//*协程栈大小
  STATE m_state = INIT;//*协程状态
  ucontext_t m_context; //*协程上线文
  void *m_stack; //*协程栈指针
  std::function<void()> m_cb; //*协程函数
};
#endif
