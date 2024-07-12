#ifndef  SYLAR_H
#define SYLAR_H
#include"./include/Log.h"
#include"./include/PThread.h"
#include"./include/hook.h"
#include"./include/util.h"
#include"./include/Fiber.h"
#include"./include/Mutex.h"
#include"./include/Singleton.h"
#include"./include/scheduler.h"
#include"./include/timer.h"

using namespace Sylar;
inline Logger::ptr g_logger=SYLAR_LOG_NAME("system");
#define LOG_INFO SYLAR_LOG_INFO(g_logger)
#endif // ! SYLAR_H

