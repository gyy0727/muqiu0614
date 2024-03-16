/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-13 14:14:29
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-16 20:13:48
 * @FilePath: /sylar/include/Fiber.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <sys/ucontext.h>
#include <type_traits>
#include <ucontext.h>
namespace Sylar {
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
  using ptr = std::shared_ptr<Fiber>;
  enum STATE {

    INIT, // *初始化状态

    HOLD, // * 挂起状态

    RUNING, // * 执行中状态

    TERM, // * 结束状态

    READY, // * 可执行状态

    EXCEPT // * 异常状态
  };
  //*主协程,负责调度
  Fiber();
  Fiber(std::function<void()> cb, size_t stacksize = 0,
        bool use_caller = true);
  ~Fiber();
  void reset(std::function<void()> &cb);
  void resume();
  void yield();
  uint64_t getId() const { return m_id; }

  STATE getState() const { return m_state; }
  void setState(STATE state) { m_state = state; }

public:
  static void SetThis(Fiber *f);

  static Fiber::ptr GetThis();
  static Fiber::ptr GetMainFiber();

  // static void YieldToReady();

  // static void YieldToHold();

  static uint64_t TotalFibers();

  static void MainFunc();

  // static void CallerMainFunc();

  static uint64_t GetFiberId();

private:
  uint64_t m_id = 0;          //*协程id
  uint32_t m_stacksize = 0;   //*栈大小
  STATE m_state = INIT;       //*协程状态
  ucontext_t m_ucp;           //*上下文信息
  void *m_stack = nullptr;    //*栈空间起始地址
  std::function<void()> m_cb; //*协程函数
  bool m_runInScheduler;      //* 本协程是否参与调度器调度
};

} // namespace Sylar