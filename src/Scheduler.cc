#include "../include/Scheduler.h"
#include <iostream>
#include <utility>

namespace Sylar {

static thread_local Scheduler *t_scheduler = nullptr;   //*当前协程调度器
static thread_local Fiber *t_scheduler_fiber = nullptr; //*线程主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_name(name) {
  
    }

Scheduler::~Scheduler() {}
//*返回当前协程调度器
Scheduler *Scheduler::GetThis() { return t_scheduler; }
//*返回线程主协程
Fiber *Scheduler::GetMainFiber() { return t_scheduler_fiber; }

void Scheduler::start() {}

void Scheduler::stop() {}

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::run() {}

void Scheduler::tickle() { SYLAR_LOG_INFO(ManagerLog()("system")) << "tickle"; }

bool Scheduler::stopping() {}

void Scheduler::idle() {}

void Scheduler::switchTo(int thread) {}

std::ostream &Scheduler::dump(std::ostream &os) { return std::cout; }

} // namespace Sylar
