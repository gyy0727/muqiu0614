/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-02 12:07:17
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-09 20:29:19
 * @FilePath: /sylar/src/test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "../include/json.hpp"
#include <codecvt>
#include <fstream>
#include <iostream>
#include "../include/Log.h"

int main() {
  Sylar::LogLevel::Level a = Sylar::LogLevel::FromString("DEBUG");
  std::cout << a << std::endl;
  return 0;
}