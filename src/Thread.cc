/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-12 19:27:33
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-13 13:48:53
 * @FilePath: /sylar/src/Thread.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Thread.h"
#include <memory>
#include <thread>
namespace ThreadC11 {
static std::shared_ptr<std::thread> thread_;

Thread::Thread(std::string &name, ThreadFunc func)
    : m_id(Sylar::GetThreadId()), m_name(name), m_func(func) {}

uint64_t Thread::getId() { return m_id; }

void Thread::start() {
  std::thread t(m_func);
  thread_=std::make_shared<std::thread>(std::move(t));
}

Thread::~Thread() {
  
}
} // namespace std
