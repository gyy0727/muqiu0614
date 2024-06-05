/*
 * @Author       : Gyy0727 3155833132@qq.com
 * @Date         : 2024-06-05 12:32:01
 * @LastEditors  : Gyy0727 3155833132@qq.com
 * @LastEditTime : 2024-06-05 12:44:36
 * @FilePath     : /muqiu0614/src/Fiber.cc
 * @Description  :
 * Copyright (c) 2024 by Gyy0727 email: 3155833132@qq.com, All Rights Reserved.
 */
#include "../include/Fiber.h"
#include "../include/Log.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber* t_thread_fiber = nullptr;

class MallocStackAllocator {
public:
	static void* Alloc(size_t stackSize)
	{
		return malloc(stackSize);
	}

	
};
