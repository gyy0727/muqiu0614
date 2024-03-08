/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-02 12:07:17
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-08 12:05:57
 * @FilePath: /sylar/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "../include/json.hpp"
#include <codecvt>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
// void print_json(const nlohmann::json& j, const std::string& prefix = "") {
//     // 判断JSON数据的类型
//     if (j.is_object()) {
//         for (const auto& item : j.items()) {
//           // 对于JSON对象，打印键

//             std::cout << prefix << item.key() << "{item}: ";
//           //   // 递归调用以打印值
//             print_json(item.value(), prefix + "  ");

//         }
//     } else if (j.is_array()) {
//         for (size_t i = 0; i < j.size(); ++i) {
//           // // 对于数组，打印索引

//             std::cout << prefix << "[" << i << "]: ";
//           //   // 递归调用以打印每个元素
//             print_json(j[i], prefix + "  ");

//         }
//     } else {
//       // 对于基本类型，直接打印值
//       // std::cout <<"---------------------------" << std::endl;
//         std::cout << j.dump() << " (普通元素)" << std::endl;
//     }
// }
void test() {
  // auto config_json = R"({"A" : "a", "B" : "b", "Pi" : 1234 })"_json;

  // // parse explicitly
  // std::string info = R"({"A" : "a", "B" : "b", "Pi" : 1234 })";
  // auto config_json1 = nlohmann::json::parse(info);
  // std::cout << config_json << std::endl;


    json jsonArray = json::array({"apple", "banana", "cherry"});

    // 输出JSON数组
    std::cout << jsonArray.dump() << std::endl;

  
}
// int main() {
//   // 读取一个json文件，nlohmann会自动解析其中数据
//   // std::ifstream i("/home/muqiu0614/桌面/sylar/src/log.json");
//   // json j = json::parse(i);
//   // print_json(j);
//   test();
//   return 0; // 成功退出
// }