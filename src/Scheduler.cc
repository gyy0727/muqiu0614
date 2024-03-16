/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-16 14:01:30
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-17 01:04:36
 * @FilePath: /sylar/src/Scheduler.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Scheduler.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

namespace Sylar {
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static thread_local Scheduler *t_scheduler = nullptr;   //*当前协程调度器
static thread_local Fiber *t_scheduler_fiber = nullptr; //*线程主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_name(name) {
  if (use_caller) {
    Fiber::GetMainFiber();
    --threads;
    t_scheduler = this;
    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, false));
    Sylar::PThread::SetName(m_name);
    t_scheduler_fiber = m_rootFiber.get();
    m_rootThread = Sylar::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = threads;
}

Scheduler::~Scheduler() {
  //*必须是调度器本身调用析构函数
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}
//*返回当前协程调度器
Scheduler *Scheduler::GetThis() { return t_scheduler; }
//*返回线程主协程
Fiber *Scheduler::GetMainFiber() { return t_scheduler_fiber; }

void Scheduler::start() {

  MutexType::Lock lock(m_mutex);
  if (!m_stopping) {
    //*已经启动
    return;
  }
  m_stopping = false;
  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i].reset(new PThread(std::bind(&Scheduler::run, this),
                                   m_name + "_" + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
  //*若是在这里启动caller线程的调度协程,会出现无法再start后提交任务的情况,
  // *因为caller协程会切换到调度协程和子协程
  // if(m_rootFiber) {
  //    //m_rootFiber->swapIn();
  //    m_rootFiber->call();
  //    SYLAR_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
  //}
  SYLAR_LOG_INFO(g_logger) << "start lock free ";
}

void Scheduler::stop() {
  m_autoStop = true;
  //* 使用use_caller,并且只有一个线程，并且主协程的状态为结束或者初始化
  if (m_rootFiber && m_threadCount == 0 &&
      (m_rootFiber->getState() == Fiber::TERM ||
       m_rootFiber->getState() == Fiber::INIT)) {
    SYLAR_LOG_INFO(g_logger) << this->m_name << " sheduler stopped";
    // *停止状态为true
    m_stopping = true;

    // *若达到停止条件则直接return
    if (stopping()) {
      //*因为只有一个caller线程,所以只需要等他自己执行完结束
      return;
    }
  }
  m_stopping = true;
  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }

  if (m_rootFiber) {
    tickle();
  }
  if (m_rootFiber) {
    // while(!stopping()) {
    //     if(m_rootFiber->getState() == Fiber::TERM
    //             || m_rootFiber->getState() == Fiber::EXCEPT) {
    //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0,
    //         true)); SYLAR_LOG_INFO(g_logger) << " root fiber is term, reset";
    //         t_fiber = m_rootFiber.get();
    //     }
    //     m_rootFiber->call();
    // }
    if (!stopping()) {
      m_rootFiber->resume();
    }
  }

  std::vector<PThread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex);
    thrs.swap(m_threads);
  }

  for (auto &i : thrs) {
    i->join();
  }

}

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::run() {
  // SYLAR_LOG_INFO(g_logger) << "ggggggggggggggggggggggggggggggg";

  setThis();
  //*证明当前不是在caller线程中,所以this就是目前线程的调度线程
  if (Sylar::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = Fiber::GetMainFiber().get();
  }
  //*idle协程
  Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
  //*任务协程
  Fiber::ptr cb_fiber;
  //*要处理的任务
  FiberAndThread ft;
  while (true) {
    //*每次循环前都要先重置
    ft.reset();
    //*是否要执行tikle()函数通知其他线程有任务
    bool is_tickle = false;
    bool is_active = false;
    // if (m_fibers.empty()) {
    //   SYLAR_LOG_INFO(g_logger)
    //       << "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk";
    // } else {
    //   SYLAR_LOG_INFO(g_logger)
    //       << "lllllllllllllllllllllllllllllllllll  " << m_fibers.size();
    // }
    {
      // SYLAR_LOG_INFO(ManagerLog()("system")) << "run lock get ";
      MutexType::Lock lock(m_mutex);
      auto it = m_fibers.begin();
      while (it != m_fibers.end()) {
        //*检查该任务是否要求在指定线程上处理
        // if (it->thread != -1 && it->thread != Sylar::GetThreadId()) {
        //   is_tickle = true;
        //   continue;
        // }
        //*判断该任务是不是已经在执行
        if (it->fiber && it->fiber->getState() == Fiber::RUNING) {
          continue;
        }
        ft = *it;

        //*it ++可以解决迭代器失效的问题
        m_fibers.erase(it++);
        //*活跃线程数加一
        ++m_activeThreadCount;
        is_active = true;

        break;
      }
      is_tickle |= it != m_fibers.end();

      // SYLAR_LOG_INFO(ManagerLog()("system"))
      //     << GetThreadId() << " 正在处理任务";
    }
    if (is_tickle) {
      //*证明还有任务没处理,去通知其他线程
      tickle();
    }
    if (ft.fiber && ft.fiber->getState() != Fiber::TERM &&
        ft.fiber->getState() != Fiber::EXCEPT) {
      ft.fiber->resume();
      //*执行完成,工作线程数减一
      --m_activeThreadCount;
      ft.reset();
    } else if (ft.cb) {
      if (cb_fiber) {
        cb_fiber->reset(ft.cb);
      } else {
        cb_fiber.reset(new Fiber(ft.cb));
      }
      ft.reset();
      cb_fiber->resume();
      --m_activeThreadCount;
      cb_fiber.reset();
    } else {
      //*来到这里,证明没有任务了
      if (is_active) {
        --m_activeThreadCount;
        continue;
      }
      if (idle_fiber->getState() == Fiber::TERM) {
        //*如果调度器没有调度任务，那么idle协程会不停地resume/yield，不会结束，如果idle协程结束了，那一定是调度器停止了
        SYLAR_LOG_INFO(ManagerLog()("system")) << "idle fiber term";
        break;
      }

      ++m_idleThreadCount;
      idle_fiber->resume();
      --m_idleThreadCount;
      // if (idle_fiber->getState() != Fiber::TERM &&
      //     idle_fiber->getState() != Fiber::EXCEPT) {
      //   idle_fiber->setState(Fiber::HOLD);
      // }
    }
  }
  SYLAR_LOG_DEBUG(ManagerLog()("system")) << "Scheduler::run() exit";
}

void Scheduler::tickle() { // SYLAR_LOG_INFO(ManagerLog()("system")) <<
                           // "tickle";
                           }

bool Scheduler::stopping() {

  MutexType::Lock lock(m_mutex);

  return m_autoStop && m_stopping && m_fibers.empty() &&
         m_activeThreadCount == 0;
}

void Scheduler::idle() {
  SYLAR_LOG_INFO(ManagerLog()("system")) << "idle";
  sleep(0.5);
}

// void Scheduler::switchTo(int thread) {}

std::ostream &Scheduler::dump(std::ostream &os) {
  os << "[Scheduler name=" << m_name << " size=" << m_threadCount
     << " active_count=" << m_activeThreadCount
     << " idle_count=" << m_idleThreadCount << " stopping=" << m_stopping
     << " ]" << std::endl
     << "    ";
  for (size_t i = 0; i < m_threadIds.size(); ++i) {
    if (i) {
      os << ", ";
    }
    os << m_threadIds[i];
  }
  return os;
}

} // namespace Sylar
