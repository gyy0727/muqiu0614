/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-11 21:04:06
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-12 19:32:46
 * @FilePath: /sylar/include/Mutex.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "Noncopyable.h"
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <thread>
namespace Sylar {

class Semaphore {
public:
  Semaphore(uint32_t count = 0);
  ~Semaphore();
  void wait();
  void notify();

private:
  sem_t m_semaphore;
};
/**
 * @description: 作用域锁
 * @return {*}
 */
template <class T> class ScopedLockImpl {
public:
  ScopedLockImpl(T &mutex) : m_mutex(mutex) {
    m_mutex.lock();
    m_locked = true;
  }
  ~ScopedLockImpl() { unlock(); }
  void lock() {
    if (!m_locked) {
      m_mutex.lock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  T &m_mutex;
  bool m_locked = false;
};
template <class T> struct ReadScopedLockImpl {
public:
  ReadScopedLockImpl(T &mutex) : m_mutex(mutex) {
    m_mutex.rdlock();
    m_locked = true;
  }

  ~ReadScopedLockImpl() { unlock(); }

  void lock() {
    if (!m_locked) {
      m_mutex.rdlock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  T &m_mutex;

  bool m_locked;
};
template <class T> struct WriteScopedLockImpl {
public:
  WriteScopedLockImpl(T &mutex) : m_mutex(mutex) {
    m_mutex.wrlock();
    m_locked = true;
  }

  ~WriteScopedLockImpl() { unlock(); }

  void lock() {
    if (!m_locked) {
      m_mutex.wrlock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// Mutex
  T &m_mutex;
  /// 是否已上锁
  bool m_locked;
};
class Mutex : Noncopyable {
public:
  /// 局部锁
  typedef ScopedLockImpl<Mutex> Lock;

  Mutex() { pthread_mutex_init(&m_mutex, nullptr); }

  ~Mutex() { pthread_mutex_destroy(&m_mutex); }

  void lock() { pthread_mutex_lock(&m_mutex); }

  void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
  /// mutex
  pthread_mutex_t m_mutex;
};
class RWMutex : Noncopyable {
public:
  /// 局部读锁
  typedef ReadScopedLockImpl<RWMutex> ReadLock;

  /// 局部写锁
  typedef WriteScopedLockImpl<RWMutex> WriteLock;

  RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }

  ~RWMutex() { pthread_rwlock_destroy(&m_lock); }

  void rdlock() { pthread_rwlock_rdlock(&m_lock); }

  void wrlock() { pthread_rwlock_wrlock(&m_lock); }

  void unlock() { pthread_rwlock_unlock(&m_lock); }

private:
  /// 读写锁
  pthread_rwlock_t m_lock;
};
class Spinlock : Noncopyable {
public:
  /// 局部锁
  typedef ScopedLockImpl<Spinlock> Lock;

  Spinlock() { pthread_spin_init(&m_mutex, 0); }

  ~Spinlock() { pthread_spin_destroy(&m_mutex); }

  void lock() { pthread_spin_lock(&m_mutex); }

  void unlock() { pthread_spin_unlock(&m_mutex); }

private:
  /// 自旋锁
  pthread_spinlock_t m_mutex;
};
class CASLock : Noncopyable {
public:
  /// 局部锁
  typedef ScopedLockImpl<CASLock> Lock;

  CASLock() { m_mutex.clear(); }

  ~CASLock() {}

  void lock() {
    while (std::atomic_flag_test_and_set_explicit(&m_mutex,
                                                  std::memory_order_acquire))
      ;
  }

  void unlock() {
    std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
  }

private:
  /// 原子状态
  volatile std::atomic_flag m_mutex;
};
} // namespace Sylar