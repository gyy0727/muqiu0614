#include "../include/Log.h"
#include "../include/timer.h"
#include "../include/util.h"
#include <mutex>
#include <shared_mutex>
#include <shared_mutex>
#include <thread>
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
bool Timer::cancel() {
  std::unique_lock<std::shared_mutex> lock(m_manager->m_mutex);
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    m_manager->m_timers.erase(it);
    return true;
  }
  return false;
}
bool Timer::refresh() {

  std::unique_lock<std::shared_mutex> lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }
  //*这里之所以要删除重新加入是为了重新构造小顶堆
  m_manager->m_timers.erase(it);
  m_next = Sylar::GetCurrentMS() + m_ms;
  m_manager->m_timers.insert(shared_from_this());
  return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
  if (ms == m_ms && !from_now) {
    return true;
  }
  std::unique_lock<std::shared_mutex> lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }
  m_manager->m_timers.erase(it);
  uint64_t start = 0;
  if (from_now) {
    start = Sylar::GetCurrentMS();
  } else {
    start = m_next - m_ms;
  }
  m_ms = ms;
  m_next = start + m_ms;
  m_manager->addTimer(shared_from_this(), lock);
  return true;
}

Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring,
             TimerManager *manager)
    : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager) {
  m_next = Sylar::GetCurrentMS() + ms; //*可以理解为定时器什么时候执行
}
Timer::Timer(uint64_t next)
    : m_recurring(false), m_ms(0), m_next(next), m_cb(nullptr),
      m_manager(nullptr) {}

bool Timer::Comparator::operator()(const Timer::ptr &lhs,
                                   const Timer::ptr &rhs) const {
  if (!lhs && !rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }
  if (lhs->m_next < rhs->m_next) {
    return true;
  }
  if (rhs->m_next < lhs->m_next) {
    return false;
  }
  return lhs.get() < rhs.get();
}

TimerManager::TimerManager() : m_tickled(false) {
  m_previousTime = Sylar::GetCurrentMS(); //*上一次执行的时间
}
TimerManager::~TimerManager() {}
Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb,
                                  bool recurring) {
  Timer::ptr timer(new Timer(ms, cb, recurring, this));
  std::unique_lock<std::shared_mutex> lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

static void onTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}
Timer::ptr TimerManager::addConditionTimer(uint64_t ms,
                                           std::function<void()> cb,
                                           std::weak_ptr<void> weak_cond,
                                           bool recurring) {
  return addTimer(ms, std::bind(&onTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
  std::shared_lock<std::shared_mutex> lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return ~0ull; //*无限大的时间
  }
  const Timer::ptr &next = *m_timers.begin();
  uint64_t now_ms = Sylar::GetCurrentMS();
  if (now_ms == next->m_next) {
    return 0;
  } else {
    return next->m_next - now_ms; //*返回距离下一次定时器触发所需时间
  }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs) {
  uint64_t now_ms = Sylar::GetCurrentMS();
  std::vector<Timer::ptr> expired; //* 已经触发的定时器任务集合
  {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_timers.empty()) {
      return;
    }
  }

  std::unique_lock<std::shared_mutex> lock(m_mutex);
  if (m_timers.empty()) {

    return;
  }

  bool rollover = detectColckRollover(now_ms); //*判断是否发生了时钟回滚
  if (!rollover && ((*m_timers.begin())->m_next > now_ms)) {
    return; //*如果没有发生时钟回滚,且最小的定时器还没过期
  }
  Timer::ptr now_timer(new Timer(now_ms));
  auto it =
      rollover
          ? m_timers.end()
          : m_timers.lower_bound(
                now_timer); //*如果发生了时间回滚,那就把容器里面所有定时器设为过期
  while (it != m_timers.end() && (*it)->m_next == now_ms) {
    ++it;
  }
  expired.insert(expired.begin(), m_timers.begin(), it);
  m_timers.erase(m_timers.begin(), it);
  cbs.reserve(expired.size());

  for (auto &timer : expired) {
    cbs.push_back(timer->m_cb);
    if (timer->m_recurring) {
      timer->m_next = now_ms + timer->m_ms;
      m_timers.insert(timer);
    } else {
      timer->m_cb = nullptr;
    }
  }
}
bool TimerManager::hasTimer() {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
  return !m_timers.empty();
}

void TimerManager::addTimer(Timer::ptr val,std::unique_lock<std::shared_mutex> &lock) {
  auto it = m_timers.insert(val).first;
  bool at_front = (it == m_timers.begin()) && !m_tickled;
  if (at_front) {
    m_tickled = true;
  }
  lock.unlock();

  if (at_front) {
    onTimerInsertAtFront();
  }
}
bool TimerManager::detectColckRollover(uint64_t now_ms) {
  bool rollover = false;
  if (now_ms < m_previousTime && now_ms < (m_previousTime - 60 * 60 * 1000)) {
    rollover = true;
  }
  m_previousTime = now_ms;
  return rollover;
}
