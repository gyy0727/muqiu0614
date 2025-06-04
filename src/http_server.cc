#include "../include/Log.h"
#include "../include/http_server.h"
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, IOManager *worker, IOManager *io_worker,
                       IOManager *accept_worker)
    : TcpServer(worker, io_worker, accept_worker), m_isKeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);

  m_type = "http";
}

void HttpServer::handleClient(Socket::sptr client) {
  HttpSession::ptr session(new HttpSession(client));
  do {
    auto req = session->recvRequest();
    if (!req) {
      SYLAR_LOG_DEBUG(g_logger)
          << "recv http request fail, errno=" << errno
          << " errstr=" << strerror(errno) << " cliet:" << *client
          << " keep_alive=" << m_isKeepalive;
      break;
    }

    HttpResponse::ptr rsp(
        new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
    rsp->setHeader("Server", getName());
    m_dispatch->handle(req, rsp, session);
    session->sendResponse(rsp);

    if (!m_isKeepalive || req->isClose()) {

      SYLAR_LOG_INFO(g_logger) << "not keepalive" << " " << m_isKeepalive << " is close " << req->isClose() ;
      break;
    }
    SYLAR_LOG_INFO(g_logger) << "keepalive";
  } while (true);
  session->close();
}

void HttpServer::setName(const std::string &v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}
