/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-03 12:28:07
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 17:44:01
 * @FilePath: /sylar/include/Config.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "Log.h"
#include "json.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <exception>
#include <execution>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using json = nlohmann::json;
namespace Sylar {
class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;

  ConfigVarBase(const std::string &name, const std::string &description = "")
      : m_name(name), m_description(description) {
    std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
  }

  virtual ~ConfigVarBase() {}

  const std::string &getName() const { return m_name; }

  const std::string &getDescription() const { return m_description; }

  virtual std::string toString() = 0;

  virtual bool fromString(const std::string &val) = 0;

protected:
  std::string m_name;

  std::string m_description;
};

/**
 * @description: 基本类型的转换
 * @param {return} boost
 * @return {*}
 */
template <class F, class T> class LexicalCast {
public:
  T operator()(const F &v) { return boost::lexical_cast<T>(v); }
};

/**
 * @description: 将 string类型转换成vector<T>
 * @param {string} &val
 * @return {vector<T>}
 */
template <class T> class LexicalCast<std::string, class std::vector<T>> {
public:
  std::vector<T> operator()(const std::string &val) {
    json j = json::parse(val);
    typename std::vector<T> vec;
    for (auto &element : j) {
      vec.push_back(LexicalCast<std::string, T>()(element.dump(4)));
    }
    return vec;
  }
};
/**
 * @description: 将 vector<T>类型转 string类型
 * @param {vector<T>} vec
 * @param {return} j
 * @return {*}
 */
template <class T> class LexicalCast<std::vector<T>, std::string> {
public:
  std::string operator()(std::vector<T> vec) {
    json j;
    for (auto &i : vec) {
      j.push_back(LexicalCast<T, std::string>()(i));
    }
    return j.dump(4);
  }
};
/**
 * @description: 将string转换成list
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::string, class std::list<T>> {
public:
  std::list<T> operator()(const std::string &val) {
    json j = json::parse(val);
    typename std::list<T> vec;
    for (auto &element : j) {
      vec.push_back(LexicalCast<std::string, T>()(element.dump(4)));
    }
    return vec;
  }
};
/**
 * @description: 将list转换成string
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::list<T>, std::string> {
public:
  std::string operator()(std::list<T> vec) {
    json j;
    for (auto &i : vec) {
      j.push_back(LexicalCast<T, std::string>()(i));
    }
    return j.dump(4);
  }
};
/**
 * @description: 将string转换成set
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::string, class std::set<T>> {
public:
  std::set<T> operator()(const std::string &val) {

    json j = json::parse(val);
    // std::cout << j << std::endl;
    typename std::set<T> vec;
    for (auto &element : j) {
      // std::cout << "elementelementelementelementelementelementelementelementele"
      //              "mentelement"
      //           << std::endl;
      // std::cout << element << std::endl;
      // std::cout << typeid(T).name() << std::endl;
      // auto it = element["name"];

      // std::cout << it << std::endl;
      vec.insert(LexicalCast<std::string, T>()(element.dump(4)));
    }
    return vec;
  }
};
/**
 * @description: 将set转换成string
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::set<T>, std::string> {
public:
  std::string operator()(std::set<T> vec) {
    json j;
    for (auto &i : vec) {
      j.push_back(LexicalCast<T, std::string>()(i));
    }
    return j.dump(4);
  }
};
/**
 * @description: 将string转换成unordered_set
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::string, class std::unordered_set<T>> {
public:
  std::unordered_set<T> operator()(const std::string &val) {
    json j = json::parse(val);
    typename std::unordered_set<T> vec;
    for (auto &element : j) {
      vec.insert(LexicalCast<std::string, T>()(element.dump(4)));
    }
    return vec;
  }
};
/**
 * @description: 将unordered_set转换成string
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::unordered_set<T>, std::string> {
public:
  std::string operator()(std::unordered_set<T> vec) {
    json j;
    for (auto &i : vec) {
      j.push_back(LexicalCast<T, std::string>()(i));
    }
    return j.dump(4);
  }
};
/**
 * @description: 将string转换成 std::map<std::string,T>
 * @param {string} &val
 * @return {*}
 */
template <class T>
class LexicalCast<std::string, class std::map<std::string, T>> {
public:
  std::map<std::string, T> operator()(const std::string &val) {
    json j = json::parse(val);
    typename std::map<std::string, T> vec;
    for (auto &element : j.items()) {
      vec.insert(std::make_pair(element.key(), LexicalCast<std::string, T>()(
                                                   element.value().dump())));
    }
    return vec;
  }
};
/**
 * @description: 将 std::map<std::string,T>转换成string
 * @param {string} &val
 * @return {*}
 */
template <class T> class LexicalCast<std::map<std::string, T>, std::string> {
public:
  std::string operator()(std::map<std::string, T> vec) {
    json j;
    for (auto &i : vec) {
      j[i.first] = LexicalCast<T, std::string>()(i.second);
    }
    return j.dump(4);
  }
};
/**
 * @description: 将string转换成 std::unordered_map<std::string,T>
 * @param {string} &val
 * @return {*}
 */
template <class T>
class LexicalCast<std::string, class std::unordered_map<std::string, T>> {
public:
  std::unordered_map<std::string, T> operator()(const std::string &val) {
    json j = json::parse(val);
    typename std::unordered_map<std::string, T> vec;
    for (auto &element : j.items()) {
      vec.insert(std::make_pair(element.key(), LexicalCast<std::string, T>()(
                                                   element.value().dump())));
    }
    return vec;
  }
};
/**
 * @description: 将 std::unordered_map<std::string,T>转换成string
 * @param {string} &val
 * @return {*}
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
  std::string operator()(std::unordered_map<std::string, T> vec) {
    json j;
    for (auto &i : vec) {
      j[i.first] = LexicalCast<T, std::string>()(i.second);
    }
    return j.dump(4);
  }
};

template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar>;
  using on_change_cb =
      std::function<void(const T &old_value, const T &new_value)>;
  ConfigVar(const std::string &name, const T &defautl_value,
            const std::string &description = "")
      : ConfigVarBase(name, description), m_val(defautl_value), m_cdId(0) {}

  //*将基本类型转换成string
  std::string toString() override {
    try {
      // return boost::lexical_cast<std::string>(m_val);
      return ToStr()(m_val);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::toString exception" << e.what() << "convert"
          << typeid(m_val).name() << "to String";
    }
    return "";
  }

  bool fromString(const std::string &val) override {
    try {
    
      setValue(FromStr()(val));
      return true;
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
          << "ConfigVar::toString exception" << e.what()
          << " convert: string to " << typeid(m_val).name();
    }
    return false;
  }
  uint64_t addListener(on_change_cb cb) {
    static uint64_t s_fun_id = 0;
    ++s_fun_id;
    m_cbs[s_fun_id] = cb;
    return s_fun_id;
  }

  void delListener(uint64_t key) { m_cbs.erase(key); }

  on_change_cb getListener(uint64_t key) {
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

  void clearListener() { m_cbs.clear(); }
  ~ConfigVar() {}

  const T getValue() { return m_val; }
  const uint64_t getcbId() { return m_cdId; }
  void setcbId(uint64_t id) { m_cdId = id; }

  void setValue(const T &v) {
    {
      // SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
      if (v == m_val) {
        return;
      }
      for (auto &i : m_cbs) {
        i.second(m_val, v);
      }
    }

    m_val = v;
  }

private:
  T m_val;
  uint64_t m_cdId;
  std::map<uint64_t, on_change_cb> m_cbs;
};
class Config {
public:
  using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

  template <class T>

  static typename ConfigVar<T>::ptr

  Lookup(const std::string &name, const T &default_value,

         const std::string &description = "") {
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
  static ConfigVarBase::ptr LookupBase(const std::string &name);
  template <class T>
  static typename ConfigVar<T>::ptr Lookup(const std::string &name) {

    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }
  static void LoadFromConfDir(const std::string &path, bool force = false);

  static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
  static ConfigVarMap &GetDatas() {
    static ConfigVarMap s_datas;
    return s_datas;
  }

  static void LoadFromJson(const json &j);
};
} // namespace Sylar