#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "http_session.h"
#include "servlet.h"
#include "tcp_server.h"
class HttpServer : public TcpServer {

public:
  using ptr = std::shared_ptr<HttpServer>;
  HttpServer(bool keepalive = false, IOManager *worker = IOManager::getThis(),
             IOManager *io_worker = IOManager::getThis(),
             IOManager *accept_worker = IOManager::getThis());

  ServletDispatch::ptr getServletDispatch() const { return m_dispatch; };
  void setServeltDispatch(ServletDispatch::ptr v) { m_dispatch = v; };
  virtual void setName(const std::string &name) override;

protected:
  virtual void handleClient(Socket::sptr client) override;

private:
  bool m_isKeepalive;
  ServletDispatch::ptr m_dispatch;
};

#endif
