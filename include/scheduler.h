#ifndef _SCHEDULER_H
#define _SCHEDULER_H
#include "Fiber.h"
#include "PThread.h"
#include <cstddef>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
using namespace Sylar;
class Scheduler {
public:
  using ptr = std::shared_ptr<Scheduler>;
  //*用于创建协程任务
  Scheduler(size_t threads = 1, const std::string &name = "");
  virtual ~Scheduler();
  const std::string &getName() const { return m_name; }
  static Scheduler *GetThis();  //*返回当前协程调度器指针
  static Fiber *GetMainFiber(); //*返回调度主协程指针
  void start();                 //*开启协程调度
  void stop();                  //*关闭协程调度
  template <class FiberOrCb> void scheduler(FiberOrCb fc, int thread_id = -1) {
    bool needtikle = false;
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      needtikle = scheduleNoLock(fc, thread_id);
    }

    if (needtikle) {
      tickle();
    }
  }
  template <class InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      while (begin != end) {
        need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
        ++begin;
      }
    }
    if (need_tickle) {
      tickle();
    }
  }


protected:
  virtual void tickle();
  void run();
  virtual bool stopping();
  virtual void idle();
  void setThis();
  bool hasIdleThreads() { return m_activeThreadCount > 0; }

private:
  template <class FiberOrCb> bool scheduleNoLock(FiberOrCb fc, int thread) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(fc, thread);
    if (ft.fiber || ft.cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }
  struct FiberAndThread {
    Fiber::ptr fiber;
    std::function<void()> cb;
    int thread_id;
    FiberAndThread(Fiber::ptr fiber, int thread_id)
        : fiber(fiber), thread_id(thread_id) {}
    FiberAndThread(std::function<void()> cb, int thread_id)
        : cb(cb), thread_id(thread_id) {}
    FiberAndThread() : thread_id(-1) {}
    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread_id = -1;
    }
  };
  std::mutex m_mutex;                   //*互斥锁
  std::vector<PThread::ptr> m_threads; //*线程池
  std::list<FiberAndThread> m_fibers;   //*待执行的任务列表
  Fiber::ptr m_rootFiber;               //*调度器的调度协程
  std::string m_name;                   //*协程调度器的名字

protected:
  std::vector<int> m_threadIds;                   //*线程池的线程ID集合
  size_t m_threadCount;                          // *线程池线程数量
  std::atomic<size_t> m_activeThreadCount = {0}; //*活跃线程的数量
  std::atomic<size_t> m_idleThreadCount = {0};   //*空闲线程数量
  bool m_stopping = true;                        //*是否已经停止
  bool m_autoStop = false;                       //*是否是自动停止的
  int m_rootThread = 0;                          //*主线程的id
};

#endif
