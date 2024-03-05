/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-01 13:53:19
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-01 18:29:56
 * @FilePath: /sylar/include/util.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <stdint.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace Sylar {


/**
 * @brief 返回当前线程的ID
 */
pid_t GetThreadId();

/**
 * @brief 返回当前协程的ID
 */
uint32_t GetFiberId();


}