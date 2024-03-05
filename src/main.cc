/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-02-26 20:50:11
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-04 20:13:43
 * @FilePath: /桌面/sylar/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Config.h"
#include "../include/Log.h"
#include "../include/util.h"
#include <cctype>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>
Syalr::ConfigVar<int>::ptr g_ =
    Syalr::Config::Lookup("sys.port", (int)8080, "sys port");
Syalr::ConfigVar<float>::ptr g_1 =
    Syalr::Config::Lookup("sys.value", (float)10.2f, "sys value");
void testYaml() {

  YAML::Node config;
  try {
    config = YAML::LoadFile("/home/muqiu0614/桌面/sylar/src/log.yaml");
  } catch (YAML::BadFile &e) {
    std::cout << "read error!" << std::endl;
    return;
  }
   for (const auto& log : config["logs"]) {
        std::cout << "Name: " << log["name"].as<std::string>() << std::endl;
        std::cout << "Level: " << log["level"].as<std::string>() << std::endl;
        std::cout << "Formatter: " << log["formatter"].as<std::string>() << std::endl;
        std::cout << "Appenders:" << std::endl;

        // 遍历appender列表
        for (const auto& appender : log["appender"]) {
            std::cout << "  Type: " << appender["type"].as<std::string>() << std::endl;
            std::cout << "  Path: " << appender["path"].as<std::string>() << std::endl;
        }

        std::cout << std::endl;  // 打印空行以分隔每个日志条目
    }

  // std::cout << "Node type " << config.Type() << std::endl;
  // std::cout << "skills type " << config["logs"].Type() << std::endl;

  // // // 可以用string类型作为下表，读取参数
  // // std::string age = "age";
  // // std::cout << "age when string is label:" << config[age].as<int>() <<
  // std::endl;

  // std::cout << "name:" << config["name"].as<std::string>() << std::endl;
  // // std::cout << "sex:" << config["sex"].as<std::string>() << std::endl;
  // std::cout << "age:" << config["age"].as<int>() << std::endl;

  // 读取不存在的node值，报YAML::TypedBadConversion异常
  // try {
  //   std::string label = config["label"].as<std::string>();}
  //  catch (YAML::TypedBadConversion<std::string> &e) {
  //   std::cout << "label node is NULL" << std::endl;
  // }

  // std::cout << "skills c++:" << config["skills"]["c++"].as<int>() <<
  // std::endl; std::cout << "skills java:" <<
  // config["skills"]["java"].as<int>() << std::endl; std::cout << "skills
  // android:" << config["skills"]["android"].as<int>() << std::endl; std::cout
  // << "skills python:" << config["skills"]["python"].as<int>() << std::endl;

  // for (YAML::const_iterator it = config["logs"].begin();
  //      it != config["logs"].end(); ++it) {
  //   std::cout << it->first.as<std::string>() << ":"  << std::endl;
  // }

  // YAML::Node test1 = YAML::Load("[1,2,3,4]");
  // std::cout << " Type: " << test1.Type() << std::endl;

  // YAML::Node test2 = YAML::Load("1");
  // std::cout << " Type: " << test2.Type() << std::endl;

  // YAML::Node test3 = YAML::Load("{'id':1,'degree':'senior'}");
  // std::cout << " Type: " << test3.Type() << std::endl;

  // // 保存config为yaml文件
  // std::ofstream ofs("./testconfig.yaml");
  // config["score"] = 99;
  // ofs << config;
  // ofs.close();
}

int main() {

  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_->getValue();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_->toString();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_1->getValue();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_1->toString();
  testYaml();

  return 0;
}
