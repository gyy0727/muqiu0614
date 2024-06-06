/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-06 14:14:32
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 18:45:42
 * @FilePath     : /muqiu0614/src/Fiber.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */

#include "../include/Fiber.h"
#include "../include/Log.h"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <ucontext.h>
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id{1};     //*协程id
static std::atomic<uint64_t> s_fiber_counts{0}; //*当前在运行的协程数量
static thread_local Fiber *t_fiber = nullptr; //*当前在运行的协程指针
static thread_local Fiber *t_thread_fiber = nullptr; //*调度协程

class MallocStackAllocator {
public:
  static void *Alloc(size_t stackSize) { return malloc(stackSize); }
  static void delloc(void *stack) { free(stack); }
};
//*主协程的初始化
Fiber::Fiber() {
  m_id = s_fiber_id;
  SYLAR_LOG_DEBUG(g_logger) << "主协程" << m_id << "创建";
  m_state = RUNING; //*调整状态为执行中
  t_fiber = this;
  if (t_thread_fiber) {
    SYLAR_LOG_ERROR(g_logger) << "重复的调度协程";
    throw std::logic_error("bad t_thread_fiber");
  }
  t_thread_fiber = this;
  s_fiber_id++;
  s_fiber_counts++;
  getcontext(&m_context);
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool usecaller)
    : m_id(s_fiber_id++), m_stackSize(stacksize), m_state(RUNING),
      m_user_caller(usecaller) {
  s_fiber_counts++;
  m_stack = MallocStackAllocator::Alloc(m_stackSize);
  m_cb = std::move(cb);
  getcontext(&m_context);
  m_context.uc_link = nullptr;
  m_context.uc_stack.ss_sp = m_stack;
  m_context.uc_stack.ss_size = m_stackSize;
  SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
  SYLAR_LOG_DEBUG(g_logger)
      << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_counts;
  makecontext(&m_context, &Fiber::mainFunc, 0);
}
Fiber::~Fiber() {
  m_state = TERM;
  --s_fiber_counts;
  if (m_stack) {
    MallocStackAllocator::delloc(m_stack);
  } else {
    Fiber *cur = t_fiber;
    if (cur == this) {
      setThis(nullptr);
    }
  }
}
void Fiber::reset(std::function<void()> cb) {
  m_cb = cb;
  getcontext(&m_context);
  m_context.uc_link = nullptr;
  m_context.uc_stack.ss_size = m_stackSize;
  m_context.uc_stack.ss_sp = m_stack;
  makecontext(&m_context, &Fiber::mainFunc, 0);
}

void Fiber::swapIn() {
  std::cout << "--" << std::endl;
  setThis(this);
  Fiber *cur = t_thread_fiber;
  if (cur) {
    std::cout << "not empty" << std::endl;
  }
  m_state = RUNING;
  cur->m_state = HOLD;
  if (m_user_caller) {
    std::cout << "swap in " << std::endl;
    swapcontext(&cur->m_context, &m_context);

  } else {
    std::cout << "swap in " << std::endl;
    swapcontext(&cur->m_context, &m_context);
    std::cout << "swap in " << std::endl;
  }
}
void Fiber::swapOut() {

  m_state = TERM;
  if (m_user_caller) {
    setThis(t_thread_fiber);
    std::cout << "swap out " << std::endl;
    swapcontext(&m_context, &t_thread_fiber->m_context);

  } else {
    swapcontext(&m_context, &t_thread_fiber->m_context);
  }
}

Fiber::ptr Fiber::getThis() {

  if (t_fiber) {
    return t_fiber->shared_from_this();
  } else {
    SYLAR_LOG_ERROR(g_logger) << "t_fiber 为空";
    return nullptr;
  }
}

uint64_t Fiber::totalFibers() { return s_fiber_counts; }

void Fiber::setThis(Fiber *f) { t_fiber = f; }
void Fiber::mainFunc() {
  Fiber::ptr cur = getThis();
  cur->m_cb();
  cur->m_cb = nullptr;
  cur->m_state = TERM;
  // try {
  //   cur->m_cb();
  //   cur->m_cb = nullptr;
  //   cur->m_state = TERM;

  // } catch (std::exception &ex) {
  //   cur->m_state = EXCEPT;
  //   SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
  //                             << " fiber_id=" << cur->getId();
  // }
  auto ptr = cur.get();

  cur.reset();
  std::cout << "执行完成" << std::endl;
  ptr->swapOut();
}

uint64_t Fiber::getFiberId() {
  if (t_fiber) {
    return t_fiber->m_id;
  } else {
    return 0;
  }
}
