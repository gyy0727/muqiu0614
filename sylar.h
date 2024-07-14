#ifndef SYLAR_H
#define SYLAR_H
#include "./include/Fiber.h"
#include "./include/Log.h"
#include "./include/Mutex.h"
#include "./include/PThread.h"
#include "./include/Singleton.h"
#include "./include/hook.h"
#include "./include/scheduler.h"
#include "./include/timer.h"
#include "./include/util.h"

using namespace Sylar;
inline Logger::ptr g_logger1 = SYLAR_LOG_NAME("system");
#define LOG_INFO SYLAR_LOG_INFO(g_logger1)
#endif // ! SYLAR_H
