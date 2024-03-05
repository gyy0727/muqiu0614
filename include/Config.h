/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-03 12:28:07
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-04 19:59:45
 * @FilePath: /sylar/include/Config.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "Log.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <future>
#include <map>
#include <memory>
#include <sstream>
#include <string>
namespace Syalr {
class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;
  ConfigVarBase(const std::string &name, const std::string &description = "")
      : m_name(name), m_description(description) {}

  virtual ~ConfigVarBase() {}
  const std::string &getName() const { return m_name; }
  const std::string &getDescription() const { return m_description; }
  virtual std::string toString() = 0;
  virtual bool fromString(const std::string &val) = 0;

protected:
  std::string m_name;
  std::string m_description;
};
template <class T> class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar>;
  ConfigVar(const std::string &name, const T &defautl_value,
            const std::string &description = "")
      : ConfigVarBase(name, description), m_val(defautl_value) {}
  //*将基本类型转换成string
  std::string toString() override {
    try {
      return boost::lexical_cast<std::string>(m_val);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::toString exception" << e.what() << "convert"
          << typeid(m_val).name() << "to String";
    }
    return "";
  }
  bool fromString(const std::string &val) override {
    try {
      m_val = boost::lexical_cast<T>(val);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::toString exception" << e.what()
          << " convert: string to " << typeid(m_val).name();
    }
    return false;
  }
  ~ConfigVar() {}
  const T getValue() { return m_val; }

  void setValue(const T &v) { m_val = v; }

private:
  T m_val;
};
class Config {
public:
  using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;
  template <class T>
  static typename ConfigVar<T>::ptr
  Lookup(const std::string &name, const T &default_value,
         const std::string &description = "") {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LOOKUP first";
    auto tmp = Lookup<T>(name);

    if (tmp) {
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LOOKUP name= " << name << " exists";
      return tmp;
    }
    if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") !=
        std::string::npos) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }

    typename ConfigVar<T>::ptr v(
        new ConfigVar<T>(name, default_value, description));
    GetDatas()[name] = v;
    return v;
  }
  template <class T>
  static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LOOKUP second";
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }

  static ConfigVarMap &GetDatas() {
    static ConfigVarMap s_datas;
    return s_datas;
  }

private:
};
} // namespace Syalr