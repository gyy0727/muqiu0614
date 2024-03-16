/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-15 15:51:10
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-15 15:53:44
 * @FilePath: /sylar/test/src/Mutex.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Mutex.h"

#include "../include/Scheduler.h"

namespace sylar {

Semaphore::Semaphore(uint32_t count) {
  if (sem_init(&m_semaphore, 0, count)) {
    throw std::logic_error("sem_init error");
  }
}

Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }

void Semaphore::wait() {
  if (sem_wait(&m_semaphore)) {
    throw std::logic_error("sem_wait error");
  }
}

void Semaphore::notify() {
  if (sem_post(&m_semaphore)) {
    throw std::logic_error("sem_post error");
  }
}

FiberSemaphore::FiberSemaphore(size_t initial_concurrency)
    : m_concurrency(initial_concurrency) {}

FiberSemaphore::~FiberSemaphore() { m_waiters.empty(); }

bool FiberSemaphore::tryWait() {

  {
    MutexType::Lock lock(m_mutex);
    if (m_concurrency > 0u) {
      --m_concurrency;
      return true;
    }
    return false;
  }
}

void FiberSemaphore::wait() {
  
  {
    MutexType::Lock lock(m_mutex);
    if (m_concurrency > 0u) {
      --m_concurrency;
      return;
    }
    m_waiters.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
  }
  Fiber::YieldToHold();
}

void FiberSemaphore::notify() {
  MutexType::Lock lock(m_mutex);
  if (!m_waiters.empty()) {
    auto next = m_waiters.front();
    m_waiters.pop_front();
    next.first->schedule(next.second);
  } else {
    ++m_concurrency;
  }
}

} // namespace sylar
