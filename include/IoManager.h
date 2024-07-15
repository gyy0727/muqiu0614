#ifndef IOMANAGER_H
#define IOMANAGER_H
#include "../include/Mutex.h"
#include "scheduler.h"
#include "timer.h"
#include <mutex>
#include <shared_mutex>
#include <thread>
using  namespace Sylar;
class IOManager : public Scheduler, public TimerManager {
public:
  using ptr = std::shared_ptr<IOManager>;
  using RWMutexType = RWMutex;

  enum Event { NONE = 0x0, READ = 0x1, WRITE = 0x4 };

private:
  struct FdContext {
    typedef Mutex MutexType;
    struct EventContext {
      Scheduler *scheduler = nullptr; //*所属的调度器
      Fiber::ptr fiber;               //*要执行的任务
      std::function<void()> cb;
    };
    EventContext &getContext(Event event);

    void resetContext(EventContext &ctx);

    void triggerEvent(Event event);

    EventContext read;
    EventContext write;
    int fd = 0;
    Event events = NONE;
    MutexType mutex;
  };

public:
  IOManager(size_t threads = -1, const std::string &name = " ");
  ~IOManager();

  int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

  bool delEvent(int fd, Event event);
  bool cancelEvent(int fd, Event event);
  bool cancelAll(int fd);
  static IOManager *getThis();

protected:
  void tickle() override;
  bool stopping() override;
  void idle() override;
  void onTimerInsertAtFront() override;
  void contextResize(size_t size);
  bool stopping(uint64_t &timeout);

private:
  int m_epollfd;
  int m_wakeupfd;
  std::atomic<size_t> m_pendingEventCount = {0}; //*待执行的事件数量
  RWMutexType m_mutex;
  std::vector<FdContext *> m_fdContexts;
};

#endif // !IOMANAGER_H
