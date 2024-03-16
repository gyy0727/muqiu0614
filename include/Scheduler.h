/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-14 18:44:05
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-16 21:00:36
 * @FilePath: /sylar/include/Scheduler.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "Fiber.h"
#include "Mutex.h"
#include "PThread.h"
#include <boost/exception/exception.hpp>
namespace Sylar {
class Scheduler {
public:
  using ptr = boost::shared_ptr<Scheduler>;
  using MutexType = Mutex;
  //*线程数量,是否使用当前线程作为调度线程,调度线程名字
  Scheduler(size_t threads = 1, bool use_caller = true,
            const std::string &name = "");

  virtual ~Scheduler();

  const std::string &getName() const { return m_name; }
  //*获取调度线程
  static Scheduler *GetThis();
  //*获取调度协程
  static Fiber *GetMainFiber();
  //*开启调度
  void start();
  //*停止调度
  void stop();

  //*封装了协程对应的线程
  class FiberAndThread {
  public:
    Fiber::ptr fiber; //*协程

    std::function<void()> cb; //* 协程执行函数

    int thread; //* 线程id

    FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}

    FiberAndThread(Fiber::ptr *f, int thr) : thread(thr) { fiber.swap(*f); }

    FiberAndThread(std::function<void()> f, int thr) : cb(f), thread(thr) {}

    FiberAndThread(std::function<void()> *f, int thr) : thread(thr) {
      cb.swap(*f);
    }

    FiberAndThread() : thread(-1) {}

    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread = -1;
    }
  };

  //*调度协程 协程或函数 协程执行的线程id,-1标识任意线程

  template <class FiberOrCb> void submitTask(FiberOrCb fc, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = submit(fc, thread);
    }

    if (need_tickle) {
      tickle();
    }
  }

  //*批量调度协程 协程数组的开始 协程数组的结束

  template <class InputIterator>
  void submitTask(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      while (begin != end) {
        need_tickle = submit(&*begin, -1) || need_tickle;
        ++begin;
      }
    }
    if (need_tickle) {
      tickle();
    }
  }

  void switchTo(int thread = -1);
  std::ostream &dump(std::ostream &os);

protected:
  //* 通知协程调度器有任务了

  virtual void tickle();
  //*协程调度函数

  void run();

  //* 返回是否可以停止

  virtual bool stopping();

  //*协程无任务可调度时执行idle协程

  virtual void idle();

  //* 设置当前的协程调度器

  void setThis();
  // void switchTo(int thread = -1);
  //*是否有空闲线程

  bool hasIdleThreads() { return m_idleThreadCount > 0; }

private:
  template <class FiberOrCb> bool submit(FiberOrCb fc, int thread) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(fc, thread);
    if (ft.fiber || ft.cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

private:
  MutexType m_mutex; //* Mutex

  std::vector<PThread::ptr> m_threads; //* 线程池

  std::list<FiberAndThread> m_fibers; //* 待执行的协程队列

  Fiber::ptr m_rootFiber; //* use_caller为true时有效, 调度协程

  std::string m_name; //* 协程调度器名称
protected:
  std::vector<int> m_threadIds; //* 协程下的线程id数组

  size_t m_threadCount = 0; //* 线程数量

  std::atomic<size_t> m_activeThreadCount = {0}; //* 工作线程数量

  std::atomic<size_t> m_idleThreadCount = {0}; //* 空闲线程数量

  bool m_stopping = true; //* 是否正在停止

  bool m_autoStop = false; //* 是否自动停止

  int m_rootThread = 0; //* 主线程id(use_caller)
};
// class SchedulerSwitcher : public Noncopyable {
// public:
//   SchedulerSwitcher(Scheduler *target = nullptr);
//   ~SchedulerSwitcher();

// private:
//   Scheduler *m_caller;
// };

} // namespace Sylar
