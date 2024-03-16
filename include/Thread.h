/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-12 19:27:41
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-13 13:49:42
 * @FilePath: /sylar/include/Thread.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <sys/types.h>
#include <thread>
#include <functional>
#include "Config.h"
#include "Log.h"
#include "Manager.h"
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
#include "Mutex.h"
#include "../include/util.h"

namespace ThreadC11 {

class Thread {
  public:
  using ThreadFunc = std::function<void()>; 
  Thread(std::string & name,ThreadFunc func);
  ~Thread();
  void start();
  uint64_t getId();

private:
  uint64_t m_id;
  std::shared_ptr<std::thread> m_thread;
  std::string m_name;
  ThreadFunc m_func;
};

}