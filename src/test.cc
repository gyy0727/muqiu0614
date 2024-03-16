/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-02 12:07:17
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 21:51:37
 * @FilePath: /sylar/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "../include/Config.h"
#include "../include/Log.h"
#include "../include/Singleton.h"
#include "../include/json.hpp"
#include <codecvt>
#include <fstream>
#include <iostream>
#include "../include/Manager.h"
#ifdef a
int main() {
  static Sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");
  static Sylar::Logger::ptr system_log2 = Sylar::ManagerLog()("工业园");
   static Sylar::Logger::ptr system_log1 = Sylar::ManagerLog()("工业园");
   SYLAR_LOG_INFO(system_log2) << "hello system";
   SYLAR_LOG_INFO(system_log1) << "hello system";
  // std::cout << Sylar::LoggerMgr::getInstance()->toJsonString() << std::endl;
  // std::ifstream i("/home/muqiu0614/桌面/sylar/src/log.json");
  // json root = json::parse(i);
  // Sylar::Config::LoadFromJson(root);
  // std::cout << "=============" << std::endl;
  // std::cout << Sylar::LoggerMgr::getInstance()->toJsonString() << std::endl;
  // std::cout << "=============" << std::endl;
  // std::cout << root << std::endl;
  // SYLAR_LOG_WARN(system_log) << "after hello system" ;
  SYLAR_LOG_INFO(system_log) << "after hello system";
  // std::cout << "=============" << std::endl;
  // system_log->setFormatter("%d - %m%n");
  // SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
  // auto it = Sylar::Config::GetDatas();
  //  std::cout <<
  //  "=================================================================================="
  //  << std::endl;
  // for (auto itt : it) {
  //   std::cout << itt.first << std::endl;
  //   std::cout << itt.second->toString() << std::endl;
  // }
  return 0;
}
#endif