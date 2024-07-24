#include "../include/tcp_server.h"

#include "../include/Log.h"
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
TcpServer::TcpServer(IOManager *worker, IOManager *io_worker,
                     IOManager *accept_worker)
    : m_worker(worker), m_ioWorker(io_worker), m_acceptWorker(accept_worker),
      m_recvTimeout(60 * 1000 * 2), m_name("sylar/1.0.0"), m_isStop(true) {}

TcpServer::~TcpServer() {
  for (auto &i : m_socks) {
    i->close();
  }
  m_socks.clear();
}

bool TcpServer::bind(Address::ptr addr) {
  std::vector<Address::ptr> addrs;
  std::vector<Address::ptr> fails;
  addrs.push_back(addr);
  return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr> &addrs,
                     std::vector<Address::ptr> &fails) {
  for (auto &addr : addrs) {
    Socket::sptr sock = Socket::CreateTCP(addr);
    if (!sock->bind(addr)) {
      SYLAR_LOG_ERROR(g_logger)
          << "bind fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->toString() << "]";
      fails.push_back(addr);
      continue;
    }
    if (!sock->listen()) {
      SYLAR_LOG_ERROR(g_logger)
          << "listen fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->toString() << "]";
      fails.push_back(addr);
      continue;
    }
    m_socks.push_back(sock);
  }
  if (!fails.empty()) {
    m_socks.clear();
    return false;
  }
  for (auto &i : m_socks) {
    SYLAR_LOG_INFO(g_logger) << "type=" << m_type << " name=" << m_name
                             << " server bind success: " << *i;
  }
  return true;
}

bool TcpServer::start() {
  if (!m_isStop) {
    return true;
  }
  m_isStop = false;
  for (auto &sock : m_socks) {
    m_acceptWorker->schedule(
        std::bind(&TcpServer::startAccept, shared_from_this(), sock));
  }
  return true;
}
void TcpServer::stop() {
  m_isStop = true;
  auto self = shared_from_this();
  m_acceptWorker->schedule([this, self]() {
    for (auto &sock : m_socks) {
      sock->cancelAll();
      sock->close();
    }
    m_socks.clear();
  });
}
void TcpServer::handleClient(Socket::sptr client) {
  SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
}
void TcpServer::startAccept(Socket::sptr sock) {
  while (!m_isStop) {
    Socket::sptr client = sock->accept();
    if (client) {
      client->setRecvTimeout(m_recvTimeout);
      m_ioWorker->schedule(
          std::bind(&TcpServer::handleClient, shared_from_this(), client));
    } else {
      SYLAR_LOG_ERROR(g_logger)
        << "accept errno=" << errno << " errstr=" << strerror(errno);
    }
  }
}

std::string TcpServer::toString(const std::string &prefix) {
  std::stringstream ss;
  ss << prefix << "[type=" << m_type << " name=" << m_name
     << " worker=" << (m_worker ? m_worker->getName() : "")
     << " accept=" << (m_acceptWorker ? m_acceptWorker->getName() : "")
     << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
  std::string pfx = prefix.empty() ? "    " : prefix;
  for (auto &i : m_socks) {
    ss << pfx << pfx << *i << std::endl;
  }
  return ss.str();
}
