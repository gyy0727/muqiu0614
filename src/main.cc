/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-26 20:50:11
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-08 15:25:50
 S @FilePath: /sylar/src/main.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Config.h"
#include "../include/Log.h"
#include <iostream>
#include <iterator>
#include <memory>
using namespace Sylar;
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
Sylar::ConfigVar<int>::ptr g_int_value_config =
    Sylar::Config::Lookup("system.aaa", (int)888, "system port");
Sylar::ConfigVar<std::vector<int>>::ptr g_int_value_config__vector =
    Sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2},
                          "system int_vec");
Sylar::ConfigVar<std::list<int>>::ptr g_int_list_value_config_list =
    Sylar::Config::Lookup("system.int_list", std::list<int>{1, 2},
                          "system int list");
Sylar::ConfigVar<std::set<int>>::ptr g_int_set_value_config =
    Sylar::Config::Lookup("system.int_set", std::set<int>{1, 2},
                          "system int set");

Sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config =
    Sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2},
                          "system int uset");

Sylar::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config =
    Sylar::Config::Lookup("system.str_int_map",
                          std::map<std::string, int>{{"k", 2}},
                          "system str int map");

Sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr
    g_str_int_umap_value_config =
        Sylar::Config::Lookup("system.str_int_umap",
                              std::unordered_map<std::string, int>{{"k", 2}},
                              "system str int map");
// Sylar::ConfigVar<int>::ptr g_int_value_config1 =
//     Sylar::Config::Lookup("system.aaa", (int)880, "system port");
// Sylar::ConfigVar<int>::ptr g_int_value_config2 =
//     Sylar::Config::Lookup("system.aaa", (int)8080, "system port");
Sylar::ConfigVar<int>::ptr g_int_valuex_config =
    Sylar::Config::Lookup("system.value", (int)116, "system value");

int main() {
  //   auto it = Config::GetDatas();
  // //   it.clear();
  //   std::cout << it.empty() << std::endl;

  //   SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config->getValue();
  //   SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config->toString();

  //   SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config->getValue();
  //   SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config->toString();
  //   SYLAR_LOG_INFO(g_logger) << "before: " <<
  //   g_int_value_config1->getValue(); SYLAR_LOG_INFO(g_logger) << "before: "
  //   << g_int_value_config1->toString();
  // SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config2->getValue();
  // SYLAR_LOG_INFO(g_logger) << "before: " << g_int_value_config2->toString();
  auto v = g_str_int_umap_value_config->getValue();
//   for (auto it : v) {
//     SYLAR_LOG_INFO(g_logger) << "before: " << it << std::endl;
//     ;
//   }
  for (auto it : v) {
    SYLAR_LOG_INFO(g_logger) << "before: " << it.first << " " << it.second << std::endl;
    
  }
  SYLAR_LOG_INFO(g_logger) << "before: " << g_str_int_umap_value_config->toString();
  SYLAR_LOG_INFO(g_logger) << "before: "
                           << g_str_int_umap_value_config->getDescription();
  std::ifstream i("/home/muqiu0614/桌面/sylar/src/log.json");
  json j = json::parse(i);
  Sylar::Config::LoadFromJson(j);

  //   SYLAR_LOG_INFO(g_logger) << "after: " << g_int_value_config->getValue();
  //   SYLAR_LOG_INFO(g_logger) << "after: " << g_int_value_config->toString();
  //   SYLAR_LOG_INFO(g_logger) << "afterfloat: " <<
  //   g_int_value_config1->getValue(); SYLAR_LOG_INFO(g_logger) <<
  //   "afterfloat:" << g_int_value_config1->toString();
  //   SYLAR_LOG_INFO(g_logger) << "afterdouble: " <<
  //   g_int_value_config2->getValue();
  //   SYLAR_LOG_INFO(g_logger) << "afterdouble: "
  //                            << g_int_value_config2->toString();
  //   SYLAR_LOG_INFO(g_logger) << "afterint: " <<
  //   g_int_valuex_config->getValue(); SYLAR_LOG_INFO(g_logger) << "afterint:"
  //   << g_int_valuex_config->toString();
  auto v1 = g_str_int_umap_value_config->getValue();
//   for (auto it : v1) {
//     SYLAR_LOG_INFO(g_logger) << "after: " << it << std::endl;
//   }
  for (auto it : v1) {
     SYLAR_LOG_INFO(g_logger) << "before: " << it.first << " " << it.second << std::endl;
  }
  SYLAR_LOG_INFO(g_logger) << "after: " << g_str_int_umap_value_config->toString();
  SYLAR_LOG_INFO(g_logger) << "after: "
                           << g_str_int_umap_value_config->getDescription();

  // auto var = it.find("system.port");
  // if (var != it.end()) {
  //   // for (auto itt : it) {
  //   //   std::cout << itt.second->getName() << std::endl;
  //   //   std::cout << itt.second->getDescription() << std::endl;
  //   //   }
  //   auto ptr = std::dynamic_pointer_cast<ConfigVar<int>>(var->second);
  //   std::cout << ptr->getValue()
  //             << " VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
  //             << std::endl;
  // for (auto item : it) {
  //   std::cout << "itemitemitemitemitemitemitemitemitemitem" << std::endl;
  //    std::cout << item.second->getName()<< std::endl;

  // }
  // }
}
