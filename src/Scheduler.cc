/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 17:08:49
 * @FilePath     : /muqiu0614/src/Scheduler.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */
#include "../include/Fiber.h"
#include "../include/Log.h"
#include "../include/hook.h"
#include "../include/scheduler.h"
#include <functional>
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static thread_local Scheduler *t_scheduler = nullptr; //*协程调度器指针
static thread_local Fiber *t_scheduler_fiber = nullptr; //* 调度器的调度协程指针

Scheduler::Scheduler(size_t threads, const std::string &name)
    : m_name(name), m_stopping(true), m_autoStop(false) {
  m_threadCount = threads;
  setThis();
}
Scheduler::~Scheduler() {
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

//*主调度器
Scheduler *Scheduler::GetThis() { return t_scheduler; }

//*主调度协程
Fiber *Scheduler::GetMainFiber() { return t_scheduler_fiber; }

void Scheduler::start() {
  std::unique_lock<std::mutex> lock(m_mutex);
  if (!m_stopping) {
    return;
  }
  m_stopping = false;
  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; i++) {
    m_threads[i].reset(new PThread(std::bind(&Scheduler::run, this),
                                   m_name + " " + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
}

//*检查是否可以停止运行
void Scheduler::stop() {
  m_autoStop = true;
  m_stopping = true;

  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }
  std::vector<PThread::ptr> threads;
  {

    std::unique_lock<std::mutex> lock(m_mutex);
    threads.swap(m_threads);
  }
  for (auto &i : threads) {
    i->join();
  }
}

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::run() {
  setThis();
  SYLAR_LOG_INFO(g_logger) << m_name << "run";
  // set_hook_enable(true);
  // if (GetThreadId() != m_rootThreadId) {
  t_scheduler_fiber = Fiber::getThis().get();
  assert(t_scheduler_fiber && t_scheduler);
  //}
  Fiber::ptr idle_fiber(new Fiber(
      std::bind(&Scheduler::idle,
                this))); //*idle coroutine , call this func while no task
  Fiber::ptr cb_fiber;
  FiberAndThread ft; // * kind of tasks
  while (1) {
    ft.reset();
    bool tickle_me = false; // * need wakeup or not
    bool is_active = false; // * running ot not
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      auto it = m_fibers.begin();
      while (it != m_fibers.end()) { //* what this func for ? || to fonud the
                                     // direction thread
        if (it->thread_id != -1 && it->thread_id != GetThreadId()) {
          ++it;
          tickle_me = true;
          continue;
        }
        assert(it->fiber || it->cb);
        if (it->fiber && it->fiber->getState() == Fiber::RUNING) {
          ++it;
          continue;
        }
        ft = *it;
        m_fibers.erase(it++);
        ++m_activeThreadCount;
        is_active = true;
        break;
      }
      tickle_me |= it != m_fibers.end();
    }
    if (tickle_me) {
      tickle(); //*取出任务tickle
    }
    if (ft.fiber && (ft.fiber->getState() != Fiber::TERM &&
                     ft.fiber->getState() != Fiber::EXCEPT)) {
      ft.fiber->swapIn(); // *执行任务
      --m_activeThreadCount;
      if (ft.fiber->getState() == Fiber::READY) {
        scheduler(ft.fiber);
      } else if (ft.fiber->getState() != Fiber::TERM &&
                 ft.fiber->getState() != Fiber::EXCEPT) {
        ft.fiber->setState(Fiber::STATE::HOLD);
      }
      ft.reset();
    } else if (ft.cb) {
      if (cb_fiber) {
        cb_fiber->reset(ft.cb);
      } else {
        cb_fiber.reset(new Fiber(ft.cb));
      }
      ft.reset();
      cb_fiber->swapIn();
      --m_activeThreadCount;
      if (cb_fiber->getState() == Fiber::READY) {
        scheduler(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber->getState() == Fiber::EXCEPT ||
                 cb_fiber->getState() == Fiber::TERM) {
        cb_fiber->reset(nullptr);

      } else {
        cb_fiber->setState(Fiber::HOLD);
        cb_fiber.reset();
      }
    } else {
      if (is_active) {
        --m_activeThreadCount;
        continue;
      }
      if (idle_fiber->getState() == Fiber::TERM) {
        SYLAR_LOG_INFO(g_logger) << "idle fiber term";
        break;
      }
      ++m_idleThreadCount;
      idle_fiber->swapIn();
      --m_idleThreadCount;
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->setState(Fiber::HOLD);
      }
    }
  }
}

void Scheduler::tickle() { SYLAR_LOG_INFO(g_logger) << "tickle"; }

bool Scheduler::stopping() {
  std::unique_lock<std::mutex> lock(m_mutex);
  return m_autoStop && m_stopping && m_activeThreadCount == 0 &&
         m_fibers.empty();
}

void Scheduler::idle() {
  SYLAR_LOG_DEBUG(g_logger) << "idle";
  while (!stopping()) {
    Fiber::getThis()->swapOut(); // * it means the new task is come
  }
}
