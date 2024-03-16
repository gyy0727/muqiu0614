/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-13 14:01:42
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-15 16:16:39
 * @FilePath: /sylar/src/test_pthread.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/PThread.h"
#include <unistd.h>

using namespace Sylar;
static int a=0;
void print() {
    
  for (; a < 100; ) {
    std::cout << "a= " << a << std::endl;
    a++;
  }
}
// int main() {
//   std::function<void()> func = print;
//   PThread t(func, "print");
//   PThread t1(func, "print");
//   PThread t2(func, "print");
//   PThread t3(func, "print");
//   sleep(20);
//   return 0;
// }