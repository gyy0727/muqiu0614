/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-10 21:16:13
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-16 20:56:56
 * @FilePath: /sylar/include/Manager.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "Config.h"
#include "Log.h"
#include"Mutex.h"
namespace Sylar {
class ManagerLog {
  using Mutex=Spinlock;
  using ptr=ManagerLog;
  public:
  
  void init();
  Logger::ptr operator()(const char *name) {
    Mutex::Lock lock(m_mutex);
    m_name = name;
    init();
    return m_logger;
  }
  private:
  Mutex m_mutex;
  Logger::ptr m_logger;
  LoggerMgr m_manager;
  const char* m_name;
};
} // namespace Sylar
