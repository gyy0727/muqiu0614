/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-12 14:57:00
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-12 14:57:23
 * @FilePath: /sylar/include/Noncopyable.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
namespace Sylar {


class Noncopyable {
public:
    
    Noncopyable() = default;


    ~Noncopyable() = default;


    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};

}
