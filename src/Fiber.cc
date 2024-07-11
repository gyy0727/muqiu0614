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
#include "../include/scheduler.h"
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
static std::atomic<uint64_t> s_fiber_id{0};     //*协程id
static std::atomic<uint64_t> s_fiber_counts{0}; //*当前在运行的协程数量
static thread_local Fiber *t_fiber = nullptr; //*当前在运行的协程指针
static thread_local Fiber::ptr t_thread_fiber = nullptr; //*调度协程指针

class MallocStackAllocator {
public:
  static void *Alloc(size_t stackSize) { return malloc(stackSize); }
  static void delloc(void *stack) { free(stack); }
};
//*主协程的初始化
Fiber::Fiber() : m_id(s_fiber_id++), m_state(RUNING) {
  if (t_thread_fiber) {
    SYLAR_LOG_ERROR(g_logger) << "重复的调度协程";
    //* throw std::logic_error("bad t_thread_fiber");
    //*异常这种操作太重了,还是不抛为妙
    //*构造函数抛出异常即使被catch,也可能导致构造失败
    exit(1);
  }
  SYLAR_LOG_INFO(g_logger) << "主协程" << m_id << "创建";
  getcontext(&m_context);
  s_fiber_counts++;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
    : m_id(s_fiber_id++), m_stackSize(stacksize), m_state(RUNING) {
  setThis(this);
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
  m_cb = std::move(cb);
  getcontext(&m_context);
  m_context.uc_link = nullptr;
  m_context.uc_stack.ss_size = m_stackSize;
  m_context.uc_stack.ss_sp = m_stack;
  makecontext(&m_context, &Fiber::mainFunc, 0);
  m_state = INIT;
}

void Fiber::swapIn() {
  /*setThis(this);*/
  /*assert(t_fiber && t_thread_fiber);*/
  /*assert(Scheduler::GetMainFiber());*/
  /*swapcontext(&Scheduler::GetMainFiber()->m_context, &m_context);*/
  setThis(this);
  assert(t_fiber && t_thread_fiber);
  assert(Scheduler::GetMainFiber());

  // 增加日志记录

  assert(m_context.uc_stack.ss_sp);
  // 检查上下文是否已经初始化

  swapcontext(&Scheduler::GetMainFiber()->m_context, &m_context);
}
void Fiber::swapOut() {
  setThis(Scheduler::GetMainFiber());
  assert(t_fiber && t_thread_fiber);
  assert(Scheduler::GetMainFiber());
  swapcontext(&m_context, &Scheduler::GetMainFiber()->m_context);
}


Fiber::ptr Fiber::getThis() {

  if (t_fiber) {
    return t_fiber->shared_from_this();
  } else {
    Fiber::ptr main_fiber(new Fiber());
    setThis(main_fiber.get());
    t_thread_fiber = main_fiber;
    return t_fiber->shared_from_this();
  }
}

uint64_t Fiber::totalFibers() {

  return s_fiber_counts; //*返回当前的协程数
}

void Fiber::setThis(Fiber *f) {
  t_fiber = f; //*设置运行时协程指针
}

void Fiber::mainFunc() {
  Fiber::ptr cur = getThis();

  //*这里使用try-catch
  //*我的个人理解是,作为一个协程,函数对象可能是用户穿进来的,要防止因为用户抛出异常导致程序崩掉
  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = TERM;

  } catch (std::exception &ex) {
    cur->m_state = EXCEPT;
    SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
                              << " fiber_id=" << cur->getId();
  }

  SYLAR_LOG_INFO(g_logger) << "执行完成";
  cur->swapOut();
}

uint64_t Fiber::getFiberId() {
  if (t_fiber) {
    return t_fiber->m_id;
  } else {
    return 0;
  }
}
