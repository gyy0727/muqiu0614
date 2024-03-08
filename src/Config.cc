/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-03 12:28:14
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-08 12:42:09
 * @FilePath: /sylar/src/Config.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Config.h"
#include <iostream>
namespace Sylar {

static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Sylar::ConfigVarBase::ptr Sylar::Config::LookupBase(const std::string &name) {
  auto it = GetDatas().find(name);
  return it == GetDatas().end() ? nullptr : it->second;
}

/**
 * @description: 将json中的数据存入列表
 * @param {string&} prefix
 * @param {json} &j
 * @param {  } std
 * @return {*}
 */
static void
ListAllMember(const std::string &prefix, const json &j,
              std::list<std::pair<std::string, const json>> &output) {
  if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
      std::string::npos) {
    SYLAR_LOG_ERROR(g_logger)
        << "Config invalid name: " << prefix << " : " << j;
    return;
  }
  output.push_back(std::make_pair(prefix, j));
  if (j.is_object()) {
    for (const auto &item : j.items()) {

      ListAllMember(prefix.empty() ? item.key() : prefix + "." + item.key(),
                    item.value(), output);
    }
  // } else if (j.is_array()) {
  //   for (size_t i = 0; i < j.size(); ++i) {

  //     ListAllMember(prefix.empty() ? "" : prefix, j[i], output);
  //   }
  }
}

void Sylar::Config::LoadFromJson(const json &root) {
  std::list<std::pair<std::string, const json>> all_jsons;
  ListAllMember("", root, all_jsons);
  for (auto &i : all_jsons) {
    std::string key = i.first;
    if (key.empty()) {
      continue;
    }
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::ptr var = LookupBase(key);

    if (var) {
      if (!i.second.is_object() && !i.second.is_array()) {
        var->fromString(i.second.dump());

      } else {
        std::stringstream ss;
        ss << i.second.dump();
        var->fromString(ss.str());
      }
    }
  }
} // namespace Sylar
} // namespace Sylar