#ifndef SERVLET_H
#define SERVLET_H
#include "PThread.h"
#include "http.h"
#include "http_session.h"
#include "util.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class Servlet {
public:
  using ptr = std::shared_ptr<Servlet>;
  Servlet(const std::string &name) : m_name(name) {}
  virtual ~Servlet() {}
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) = 0;
  const std::string &getName() const { return m_name; }

protected:
  std::string m_name;
};
class FunctionServlet : public Servlet {
public:
  /// 智能指针类型定义
  typedef std::shared_ptr<FunctionServlet> ptr;
  /// 函数回调类型定义
  typedef std::function<int32_t(HttpRequest::ptr request,
                                HttpResponse::ptr response,
                                HttpSession::ptr session)>
      callback;

  /**
   * @brief 构造函数
   * @param[in] cb 回调函数
   */
  FunctionServlet(callback cb);
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;

private:
  /// 回调函数
  callback m_cb;
};
/**
 * @brief Servlet分发器
 */
class ServletDispatch : public Servlet {
public:
  /// 智能指针类型定义
  typedef std::shared_ptr<ServletDispatch> ptr;
  /// 读写锁类型定义
  typedef RWMutex RWMutexType;

  /**
   * @brief 构造函数
   */
  ServletDispatch();
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;

  /**
   * @brief 添加servlet
   * @param[in] uri uri
   * @param[in] slt serlvet
   */
  void addServlet(const std::string &uri, Servlet::ptr slt);

  /**
   * @brief 添加servlet
   * @param[in] uri uri
   * @param[in] cb FunctionServlet回调函数
   */
  void addServlet(const std::string &uri, FunctionServlet::callback cb);

  /**
   * @brief 添加模糊匹配servlet
   * @param[in] uri uri 模糊匹配 /sylar_*
   * @param[in] slt servlet
   */
  void addGlobServlet(const std::string &uri, Servlet::ptr slt);

  /**
   * @brief 添加模糊匹配servlet
   * @param[in] uri uri 模糊匹配 /sylar_*
   * @param[in] cb FunctionServlet回调函数
   */
  void addGlobServlet(const std::string &uri, FunctionServlet::callback cb);


  void delServlet(const std::string &uri);

  /**
   * @brief 删除模糊匹配servlet
   * @param[in] uri uri
   */
  void delGlobServlet(const std::string &uri);

  /**
   * @brief 返回默认servlet
   */
  Servlet::ptr getDefault() const { return m_default; }

  /**
   * @brief 设置默认servlet
   * @param[in] v servlet
   */
  void setDefault(Servlet::ptr v) { m_default = v; }

  /**
   * @brief 通过uri获取servlet
   * @param[in] uri uri
   * @return 返回对应的servlet
   */
  Servlet::ptr getServlet(const std::string &uri);

  /**
   * @brief 通过uri获取模糊匹配servlet
   * @param[in] uri uri
   * @return 返回对应的servlet
   */
  Servlet::ptr getGlobServlet(const std::string &uri);

  /**
   * @brief 通过uri获取servlet
   * @param[in] uri uri
   * @return 优先精准匹配,其次模糊匹配,最后返回默认
   */
  Servlet::ptr getMatchedServlet(const std::string &uri);


private:
  /// 读写互斥量
  std::shared_mutex m_mutex;
  /// 精准匹配servlet MAP
  /// uri(/sylar/xxx) -> servlet
  std::unordered_map<std::string, Servlet::ptr> m_datas;
  /// 模糊匹配servlet 数组
  /// uri(/sylar/*) -> servlet
  std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
  /// 默认servlet，所有路径都没匹配到时使用
  Servlet::ptr m_default;
};

/**
 * @brief NotFoundServlet(默认返回404)
 */
class NotFoundServlet : public Servlet {
public:
  /// 智能指针类型定义
  typedef std::shared_ptr<NotFoundServlet> ptr;
  /**
   * @brief 构造函数
   */
  NotFoundServlet(const std::string &name);
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;

private:
  std::string m_name;
  std::string m_content;
};

#endif
