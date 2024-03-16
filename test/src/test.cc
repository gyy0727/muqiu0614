/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-27 19:24:26
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-15 16:22:12
 * @FilePath: /桌面/sylar/test/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Scheduler.h"
#include "../../include/Log.h"

static Sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 10;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    // sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->schedule(&test_fiber, Sylar::GetThreadId());
    }
}

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(10, false, "test");
    sc.start();
    // sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "over";
    return 0;
}
