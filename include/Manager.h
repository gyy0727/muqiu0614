/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-10 21:16:13
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 21:36:27
 * @FilePath: /sylar/include/Manager.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Config.h"
#include "Log.h"
namespace Sylar {
class ManagerLog {
  public:
  
  void init();
  Logger::ptr operator()(const char* name) {
    m_name = name;
    init();
    return m_logger;
  }
  Logger::ptr m_logger;
  LoggerMgr m_manager;
  const char* m_name;
};
} // namespace Sylar
