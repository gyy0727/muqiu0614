/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-11 21:03:56
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-15 14:23:14
 * @FilePath: /sylar/src/PThread.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/PThread.h"
#include <pthread.h>
namespace Sylar {
//*当前线程的指针,因为构造函数私有
static thread_local PThread *t_thread = nullptr;

static thread_local std::string t_thread_name = "UNKNOW";

PThread::PThread(Func cb, const std::string &name) : m_name(name), m_cb(cb) {
  if (m_name.empty()) {
    m_name = t_thread_name;
  }
  t_thread_name = m_name;
  
  int pt = pthread_create(&m_Pthread, nullptr, &PThread::run, this);
  if (pt) {
    SYLAR_LOG_ERROR(Sylar::ManagerLog()("system")) << "pthread create fail";
  }
  m_semaphore.wait();
}

const std::string &PThread::GetName() { return t_thread_name; }

void PThread::SetName(const std::string &name) {
  if (name.empty()) {
    return;
  }
  if (t_thread) {
    t_thread->m_name = name;
  }
  t_thread_name = name;
}

PThread *PThread::GetThis() { return t_thread; }

PThread::~PThread() {
  if (m_Pthread) {
    
      pthread_detach(m_Pthread);
      
    }
}

void PThread::join() {
    if(m_Pthread) {
        int rt = pthread_join(m_Pthread, nullptr);
        if(rt) {
            SYLAR_LOG_ERROR(Sylar::ManagerLog()("system")) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_Pthread = 0;
    }
}

void *PThread::run(void *arg) {
  
    PThread* thread = (PThread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = Sylar::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    thread->m_semaphore.notify();

    cb();
    return 0;
}

} // namespace Sylar