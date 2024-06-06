
#include "../include/Fiber.h"
#include "../include/Log.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <ucontext.h>
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id { 0 };

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber* t_thread_fiber = nullptr;

class MallocStackAllocator {
public:
	static void* Alloc(size_t stackSize)
	{
		return malloc(stackSize);
	}
	static void delloc(void* stack)
	{
		free(stack);
	}
};
//*主协程的初始化
Fiber::Fiber()
{
	t_fiber = this;
	getcontext(&m_context);
	s_fiber_id++;
}
Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool usecaller)
{
	m_cb = cb;
}
Fiber::~Fiber()
{
}
void Fiber::reset(std::function<void()> cb)
{
	m_cb=cb;
}
void Fiber::swapIn()
{
	
}
void Fiber::swapOut()
{
}
void Fiber::call()
{
}

Fiber::ptr Fiber::getThis()
{
}

uint64_t Fiber::totalFibers()
{
}

void Fiber::mainFunc()
{
}

void Fiber::callerMainFunc()
{
}
uint64_t Fiber::getFiberId()
{
}
