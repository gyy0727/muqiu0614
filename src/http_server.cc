#include "../include/Log.h"
#include "../include/http_server.h"
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, IOManager *worker, IOManager *io_worker,
                       IOManager *accept_worker)
    : TcpServer(worker, io_worker, accept_worker), m_isKeepalive(keepalive) {
  // m_dispatch.reset(new ServletDispatch);

  m_type = "http";
  // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
  // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));
}

void HttpServer::handleClient(Socket::sptr client) {
}
