/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-06 14:14:32
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 16:44:33
 * @FilePath     : /muqiu0614/src/Fiber.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */

#include "../include/Fiber.h"
#include "../include/Log.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
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
  m_state = RUNING; //*调整状态为执行中
  setThis(this);

  s_fiber_id++;
  s_fiber_counts++;
  getcontext(&m_context);
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool usecaller)
    : m_id(s_fiber_id++), m_stackSize(stacksize), m_state(RUNING),
      m_user_caller(usecaller) {
  s_fiber_counts++;
  m_stack = MallocStackAllocator::Alloc(m_stackSize);
  getcontext(&m_context);
  m_context.uc_link = nullptr;
  m_context.uc_stack.ss_sp = m_stack;
  m_context.uc_stack.ss_size = m_stackSize;
  makecontext(&m_context, &Fiber::mainFunc, 0);
  SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
  SYLAR_LOG_DEBUG(g_logger)
      << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_counts;
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
  setThis(this);
  Fiber *cur = t_thread_fiber;
  m_state = RUNING;
  cur->m_state = HOLD;
  if (m_user_caller) {
    swapcontext(&cur->m_context, &m_context);
  } else {
    swapcontext(&cur->m_context, &m_context);
  }
}
void Fiber::swapOut() {

  m_state = HOLD;
  if (m_user_caller) {
    setThis(t_thread_fiber);
    swapcontext(&t_thread_fiber->m_context, &m_context);
  } else {
    swapcontext(&t_thread_fiber->m_context, &m_context);
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
  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = TERM;

  } catch (std::exception &ex) {
    cur->m_state = EXCEPT;
    SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
                              << " fiber_id=" << cur->getId();
  }
  auto ptr = cur.get();
  cur.reset();
  ptr->swapOut();
}

uint64_t Fiber::getFiberId() {
  if (t_fiber) {
    return t_fiber->m_id;
  } else {
    return 0;
  }
}
