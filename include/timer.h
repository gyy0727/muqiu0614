#ifndef TIMER_H
#define TIMER_H
#include "PThread.h"
#include <memory>
#include <set>
#include"Mutex.h"
#include <stdint.h>
#include <vector>
class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
  friend class TimerManager;

public:
  using ptr = std::shared_ptr<Timer>;
  bool cancel();
  bool refresh();
  bool reset(uint64_t ms, bool from_now);

private:
  Timer(uint64_t ms, std::function<void()> cb, bool recurring,
        TimerManager *manager);
  Timer(uint64_t next);
  bool m_recurring;
  uint64_t m_ms;
  uint64_t m_next;
  std::function<void()> m_cb;
  TimerManager *m_manager;
  //*比较器
  struct Comparator {
    bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
  };
};

class TimerManager {
  friend class Timer;
    using rwLock=Sylar::RWMutex;
public:
TimerManager();
    virtual ~TimerManager();
    Timer::ptr addTimer(uint64_t ms ,std::function<void()> cb,bool recurring=false);
    Timer::ptr addConditionTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring=false);

    uint64_t getNextTimer();

    void listExpiredCb(std::vector<std::function<void()>> &cbs);
    bool hasTimer();

protected: 
    virtual void onTimerInsertAtFront()=0;

    void addTimer(Timer::ptr val,rwLock::WriteLock lock);
private:
    bool detectColckRollover(uint64_t now_ms);
    Sylar::RWMutex m_mutex;
    std::set<Timer::ptr,Timer::Comparator> m_timers;
    bool m_tickled;
    uint64_t m_previousTime;

};
#endif