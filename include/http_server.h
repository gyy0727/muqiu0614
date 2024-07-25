#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http_session.h"
#include "tcp_server.h"
class HttpServer : public TcpServer {

public:
  using ptr = std::shared_ptr<HttpServer>;
  HttpServer(bool keepalive = false, IOManager *worker = IOManager::getThis(),
             IOManager *io_worker = IOManager::getThis(),
             IOManager *accept_worker = IOManager::getThis());
protected:
    virtual void handleClient(Socket::sptr client)override;
private:
    bool m_isKeepalive;
};

#endif
