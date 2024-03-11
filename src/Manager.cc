/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-10 21:16:21
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 21:36:33
 * @FilePath: /sylar/src/Manager.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../include/Manager.h"
void Sylar::ManagerLog::init() {

  auto a = m_manager.getInstance();
  a->init();
  m_logger=a->getLogger(m_name);
  
}