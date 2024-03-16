/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-13 14:14:36
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-16 13:32:21
 * @FilePath: /sylar/src/Fiber.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Fiber.h"
#include "../include/Config.h"

#include "../include/Log.h"
#include "../include/Scheduler.h"
#include <algorithm>
#include <atomic>
#include <ucontext.h>

namespace Sylar {

static std::atomic<uint64_t> s_fiber_id{0};    //*协程id
static std::atomic<uint64_t> s_fiber_count{0}; //*当前线程运行的协程数量
static thread_local Fiber *t_fiber = nullptr; //*当前运行的协程指针
static thread_local Fiber::ptr t_threadFiber = nullptr; //*主协程指针

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator {
public:
  //*分配内存
  static void *Alloc(size_t size) { return malloc(size); }
  //*释放内存
  static void Dealloc(void *vp, size_t size) { return free(vp); }
};

//*主协程初始化,因为没有协程函数
Fiber::Fiber() {
  //*初始化即开始 执行
  m_state = RUNING;
  //*设置当前运行的协程指针
  SetThis(this);
  //*保存当前上下文
  getcontext(&m_ucp);
  s_fiber_count++;
}

//*创建新协程
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(s_fiber_id++), m_cb(cb), m_runInScheduler(use_caller) {
  SetThis(this);
  ++s_fiber_count;
  //*设置堆栈信息
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
  m_stack = MallocStackAllocator::Alloc(m_stacksize);
  //*保存当前上下文
  getcontext(&m_ucp);
  //*设置上下文指针
  m_ucp.uc_link = nullptr;
  //*设置堆栈信息
  m_ucp.uc_stack.ss_sp = m_stack;
  m_ucp.uc_stack.ss_size = m_stacksize;
  //*绑定协程函数
  if (!use_caller) {
    makecontext(&m_ucp, &Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ucp, &Fiber::CallerMainFunc, 0);
  }
}

Fiber::~Fiber() {
  --s_fiber_count;
  if (m_stack) {
    MallocStackAllocator::Dealloc(m_stack, m_stacksize);
  } else {
    Fiber *cur = t_fiber;
    //*判断当前执行的线程是不是就是要析构的线程
    if (cur == this) {
      SetThis(nullptr);
    }
  }
}

//* 重置协程函数，并重置状态
//* INIT，TERM, EXCEPT
void Fiber::reset(std::function<void()> &cb) {
  m_cb = std::move(cb);
  getcontext(&m_ucp);
  m_ucp.uc_link = nullptr;
  m_ucp.uc_stack.ss_sp = m_stack;
  m_ucp.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ucp, &Fiber::MainFunc, 0);
  m_state = INIT;
}
//*从主协程跳转到子协程
void Fiber::resume() {
  SetThis(this);
  m_state =RUNING;
  if (m_runInScheduler) {
    swapcontext(&Scheduler::GetMainFiber()->m_ucp, &m_ucp);
  } else {
    swapcontext(&t_threadFiber->m_ucp, &m_ucp);
  }
}
//*从子协程跳转到主协程
void Fiber::yield() {

  SetThis(t_threadFiber.get());
  if (m_state != TERM) {
        m_state = READY;
    }

  if (m_runInScheduler) {
    swapcontext(&m_ucp,&Scheduler::GetMainFiber()->m_ucp);
  } else {
    swapcontext( &m_ucp,&t_threadFiber->m_ucp);
  }
}

//*设置当前在运行协程的指针
void Fiber::SetThis(Fiber *f) { t_fiber = f; }

//* 返回当前协程
Fiber::ptr Fiber::GetThis() { return t_fiber->shared_from_this(); }
Fiber::ptr Fiber::GetMainFiber() {
  if (t_fiber) {
    return t_threadFiber->shared_from_this();
  } else {
    Fiber::ptr main_fiber(new Fiber);
    t_threadFiber = main_fiber;
    return t_threadFiber->shared_from_this();
  }
 }
//* 协程切换到后台，并且设置为Ready状态
void Fiber::YieldToReady() {}

//* 协程切换到后台，并且设置为Hold状态
void Fiber::YieldToHold() {}

//* 总协程数
uint64_t Fiber::TotalFibers() { return s_fiber_count; }

void Fiber::MainFunc() {
  Fiber::ptr cur=GetThis();
}

} // namespace Sylar