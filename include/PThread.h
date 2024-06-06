/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-06 17:02:47
 * @FilePath     : /muqiu0614/include/PThread.h
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */

#pragma once
#include "Config.h"
#include "Log.h"
#include "Manager.h"
#include "Mutex.h"
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <semaphore.h>
#include <semaphore>
#include <stdint.h>
#include <thread>
#include <unistd.h>
namespace Sylar {
class PThread {
public:
  using ptr = std::shared_ptr<PThread>;
  //*线程的任务函数
  using Func = std::function<void()>;

  PThread(Func cb, const std::string &name);

  ~PThread() ;

  pid_t getId() const { return m_id; }

  const std::string &getName() const { return m_name; }

  void join();

  static PThread *GetThis();

  static const std::string &GetName();

  static void SetName(const std::string &name);

private:
  static void *run(void *arg);
  pid_t m_id = -1;    //*线程id
  std::string m_name; //*线程名称

  pthread_t m_Pthread = 0; //*线程

  std::function<void()> m_cb; //*线程函数

  Semaphore m_semaphore; //*信号量
};

} // namespace Sylar