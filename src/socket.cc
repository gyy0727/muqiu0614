#include "../include/socket.h"
#include "../include/IoManager.h"
#include "../include/Log.h"
#include "../include/fdmanager.h"
#include "../include/hook.h"
#include "../include/macro.h"
#include <limits.h>
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

//*==============================创建对应的socket对象
Socket::sptr Socket::CreateTCP(Address::ptr address) {
  Socket::sptr sock(new Socket(address->getFamily(), TYPE::TCP, 0));
  return sock;
}

Socket::sptr Socket::CreateUDP(Address::ptr address) {
  Socket::sptr sock(new Socket(address->getFamily(), TYPE::UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}
Socket::sptr Socket::CreateTCPIPV4() {
  Socket::sptr sock(new Socket(IPV4, TYPE::TCP, 0));
  return sock;
}
Socket::sptr Socket::CreateUDPIPV4() {
  Socket::sptr sock(new Socket(IPV4, TYPE::UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}
Socket::sptr Socket::CreateTCPIPV6() {
  Socket::sptr sock(new Socket(IPV6, TYPE::TCP, 0));
  return sock;
}
Socket::sptr Socket::CreateUDPIPV6() {
  Socket::sptr sock(new Socket(IPV6, TYPE::UDP, 0));
  sock->newSock();
  sock->m_isConnected = true;
  return sock;
}

Socket::sptr Socket::CreateUnixTCP() {

  Socket::sptr sock(new Socket(UNIX, TYPE::TCP, 0));
  return sock;
}

Socket::sptr Socket::CreateUnixUDP() {
  Socket::sptr sock(new Socket(UNIX, TYPE::UDP, 0));
  return sock;
}
//*==========================================================
//
Socket::Socket(int family, int type, int protocol)
    : m_sock(-1), m_family(family), m_type(type), m_protocol(protocol),
      m_isConnected(false), m_localAddress(nullptr), m_remoteAddress(nullptr) {}

Socket::~Socket() {
  //*释放资源
  close();
}

//*========================================================
//*setsockopt函数被hook了
int64_t Socket::getSendTimeout() {
  FdCtx::ptr ctx = FdMgr::getInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_SNDTIMEO);
  }
  return -1;
}

void Socket::setSendTimeout(int64_t v) {
  struct timeval tv {
    int(v / 1000), int(v % 1000 * 1000)
  };
  setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::getRecvTimeout() {
  FdCtx::ptr ctx = FdMgr::getInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_RCVTIMEO);
  }
  return -1;
}

void Socket::setRecvTimeout(int64_t v) {
  struct timeval tv {
    int(v / 1000), int(v % 1000 * 1000)
  };
  setOption(SOL_SOCKET, SO_RCVTIMEO, tv); //*hook过的,会自动设置到fdctx
}
//*===================================================================
bool Socket::getOption(int level, int option, void *result, socklen_t *len) {

  int rt = getsockopt(m_sock, level, option, result, (socklen_t *)len);
  if (rt) {
    SYLAR_LOG_DEBUG(g_logger)
        << "getOption sock=" << m_sock << " level=" << level
        << " option=" << option << " errno=" << errno
        << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}

bool Socket::setOption(int level, int option, const void *result,
                       socklen_t len) {
  if (setsockopt(m_sock, level, option, result, (socklen_t)len)) {
    SYLAR_LOG_DEBUG(g_logger)
        << "setOption sock=" << m_sock << " level=" << level
        << " option=" << option << " errno=" << errno
        << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}
//*==================================================================

Socket::sptr Socket::accept() {
  //*先创建一个对应的socket对象
  Socket::sptr sock(new Socket(m_family, m_type, m_protocol));
  //*建立连接成功返回的通信描述符
  //* accept函数:
  //*(listenfd,sockaddr(传出参数,记录了客户端信息,不感兴趣可以设置为NUll),
  // sizef(addr)) *newsock 通信用的文件描述符
  int newsock = ::accept(m_sock, nullptr, nullptr);
  //*accept失败
  if (newsock == -1) {
    SYLAR_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno=" << errno
                              << " errstr=" << strerror(errno);
    return nullptr;
  }
  //*成功,用该通信文件描述符去初始化sock
  //*可以理解为每个socket连接主要就一个m_socketfd成员,其他的都是为他服务
  if (sock->init(newsock)) {
    return sock;
  }
  return nullptr;
}

//*绑定到本地端口
bool Socket::bind(const Address::ptr addr) {
  //其实就是m_sock是否初始化,只有已经初始化的才能被操作
  if (!isValid()) {
    newSock();
    if (UNLIKELY((!isValid()))) {
      return false;
    }
  }
  //*判断协议簇是否相同,不可能你一个ipv4的socket对象要绑定一个设置了ipv6的本地监听地址
  if (UNLIKELY(addr->getFamily() != m_family)) {
    SYLAR_LOG_ERROR(g_logger)
        << "bind sock.family(" << m_family << ") addr.family("
        << addr->getFamily() << ") not equal, addr=" << addr->toString();
    return false;
  }
  //*一切正常,开始真正的bind
  if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
    SYLAR_LOG_ERROR(g_logger)
        << "bind error errrno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  //*绑定完,就把对应的被绑定到的本地地址信息获取,并初始化自己的m_localAddress
  getLocalAddress();
  return true;
}

//*用于客户端,连接远程服务器
bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
  m_remoteAddress = addr;
  if (!isValid()) {
    newSock();
    if (UNLIKELY(!isValid())) {
      return false;
    }
  }
  //*判断协议簇
  if (UNLIKELY(addr->getFamily() != m_family)) {
    SYLAR_LOG_ERROR(g_logger)
        << "connect sock.family(" << m_family << ") addr.family("
        << addr->getFamily() << ") not equal, addr=" << addr->toString();
    return false;
  }
  //*判断用户设置的超时时间是否有效
  if (timeout_ms == (uint64_t)-1) {
    if (::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
      SYLAR_LOG_ERROR(g_logger)
          << "sock=" << m_sock << " connect(" << addr->toString()
          << ") error errno=" << errno << " errstr=" << strerror(errno);
      //*connect出错,所以要释放资源
      close();
      return false;
    }
  } else {
    if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(),
                               timeout_ms)) {
      SYLAR_LOG_ERROR(g_logger)
          << "sock=" << m_sock << " connect(" << addr->toString()
          << ") timeout=" << timeout_ms << " error errno=" << errno
          << " errstr=" << strerror(errno);
      close();
      return false;
    }
  }
  m_isConnected = true;
  //*客户端连接远程服务器,少了个bind 的过程,所以要一次性初始化
  //*m_remoteAddress和m_localAddress
  getRemoteAddress();
  getLocalAddress();
  return true;
}

//*重试连接
bool Socket::reconnect(uint64_t timeout_ms) {
  //*远程地址未被初始化,证明未经过connect这个过程,何来reconnect之说
  if (!m_remoteAddress) {
    SYLAR_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
    return false;
  }
  //*远端地址不变,但是自己的端口可能会变化
  m_localAddress.reset();
  return connect(m_remoteAddress, timeout_ms);
}

//*监听
bool Socket::listen(int backlog) {
  //*判断初始化没,只有初始化了才能进行一系列操作
  if (!isValid()) {
    SYLAR_LOG_ERROR(g_logger) << "listen error sock=-1";
    return false;
  }
  //*开始监听
  if (::listen(m_sock, backlog)) {
    SYLAR_LOG_ERROR(g_logger)
        << "listen error errno=" << errno << " errstr=" << strerror(errno);
    //*这里是不是应该也要进行close()
    return false;
  }
  return true;
}
//*释放对应的文件描述符资源
bool Socket::close() {

  if (!m_isConnected && m_sock == -1) {
    return true;
  }
  m_isConnected = false;
  if (m_sock != -1) {
    ::close(m_sock);
    m_sock = -1;
  }
  return false;
}

//*发送普通数据
int Socket::send(const void *buffer, size_t length, int flags) {
  if (isConnected()) {
    return ::send(m_sock, buffer, length, flags);
  }
  return -1;
}

//*struct msghdr {
//*    void         *msg_name;       // 可选的地址信息
//*    socklen_t     msg_namelen;    // 地址长度
//*    struct iovec *msg_iov;        // 数据缓冲区数组
//*    int           msg_iovlen;     // 数据缓冲区数组长度
//*    void         *msg_control;    // 辅助数据缓冲区
//*    socklen_t     msg_controllen; // 辅助数据缓冲区长度
//*    int           msg_flags;      // 消息标志
//*};

//*之前muduo使用的是write函数
int Socket::send(const iovec *buffers, size_t length, int flags) {
  if (isConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}

//*只要用于udp
int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to,
                   int flags) {
  if (isConnected()) {
    return ::sendto(m_sock, buffer, length, flags, to->getAddr(),
                    to->getAddrLen());
  }
  return -1;
}
//*Address::ptr to 目的地IP地址,用于无连接的udp
int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to,
                   int flags) {
  if (isConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->getAddr();
    msg.msg_namelen = to->getAddrLen();
    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}
//*===============================================================TCP和udp
int Socket::recv(void *buffer, size_t length, int flags) {
  if (isConnected()) {
    SYLAR_LOG_INFO(g_logger) << "read the sockfd";
    return ::recv(m_sock, buffer, length, flags);
  }
  return -1;
}

int Socket::recv(iovec *buffers, size_t length, int flags) {
  if (isConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}

int Socket::recvFrom(void *buffer, size_t length, Address::ptr from,
                     int flags) {
  if (isConnected()) {
    socklen_t len = from->getAddrLen();
    return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
  }
  return -1;
}

int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from,
                     int flags) {
  if (isConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = from->getAddr();
    msg.msg_namelen = from->getAddrLen();
    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}
//*========================================================
Address::ptr Socket::getRemoteAddress() {
  if (m_remoteAddress) {
    return m_remoteAddress;
  }

  Address::ptr result;
  switch (m_family) {
  case AF_INET:
    result.reset(new IPv4Address());
    break;
  case AF_INET6:
    result.reset(new IPv6Address());
    break;
  case AF_UNIX:
    result.reset(new UnixAddress());
    break;
  default:
    result.reset(new UnknownAddress(m_family));
    break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getpeername(m_sock, result->getAddr(), &addrlen)) {
    // SYLAR_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
    //     << " errno=" << errno << " errstr=" << strerror(errno);
    return Address::ptr(new UnknownAddress(m_family));
  }
  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_remoteAddress = result;
  return m_remoteAddress;
}

Address::ptr Socket::getLocalAddress() {
  if (m_localAddress) {
    return m_localAddress;
  }
  Address::ptr result;
  switch (m_family) {
  case AF_INET:
    result.reset(new IPv4Address());
    break;
  case AF_INET6:
    result.reset(new IPv4Address());
    break;
  case AF_UNIX:
    result.reset(new UnixAddress());
    break;
  default:
    result.reset(new UnknownAddress(m_family));
    break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getsockname(m_sock, result->getAddr(), &addrlen)) {
    SYLAR_LOG_ERROR(g_logger)
        << "getsockname error sock=" << m_sock << " errno=" << errno
        << " errstr=" << strerror(errno);
    return Address::ptr(new UnknownAddress(m_family));
  }

  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_localAddress = result;
  return m_localAddress;
}

bool Socket::isValid() const { return m_sock != -1; }

//*获取套接字的错误信息
int Socket::getError() {
  int error = 0;
  socklen_t len = sizeof(error);
  if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
    error = errno;
  }
  return error;
}
std::ostream &Socket::dump(std::ostream &os) const {
  os << "[Socket sock=" << m_sock << " is_connected=" << m_isConnected
     << " family=" << m_family << " type=" << m_type
     << " protocol=" << m_protocol;
  if (m_localAddress) {
    os << " local_address=" << m_localAddress->toString();
  }
  if (m_remoteAddress) {
    os << " remote_address=" << m_remoteAddress->toString();
  }
  os << "]";
  return os;
}

std::string Socket::toString() const {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}
bool Socket::cancelRead() {
  return IOManager::getThis()->cancelEvent(m_sock, IOManager::READ);
}
bool Socket::cancelWrite() {
  return IOManager::getThis()->cancelEvent(m_sock, IOManager::WRITE);
}
bool Socket::cancelAccept() {
  return IOManager::getThis()->cancelEvent(m_sock, IOManager::READ);
}

bool Socket::cancelAll() { return IOManager::getThis()->cancelAll(m_sock); }

void Socket::initSock() {
  int val = 1;
  setOption(SOL_SOCKET, SO_REUSEADDR, val);
  if (m_type == SOCK_STREAM) {
    setOption(IPPROTO_TCP, TCP_NODELAY, val);
  }
}

void Socket::newSock() {
  m_sock = socket(m_family, m_type, m_protocol);
  if (LIKELY(m_sock != -1)) {
    initSock();
  } else {
    SYLAR_LOG_ERROR(g_logger)
        << "socket(" << m_family << ", " << m_type << ", " << m_protocol
        << ") errno=" << errno << " errstr=" << strerror(errno);
  }
}

//*根据传入文件描述符初始化一个socket对象
bool Socket::init(int sock) {
  FdCtx::ptr ctx = FdMgr::getInstance()->get(sock);
  if (ctx && ctx->isSocket() && !ctx->isClose()) {
    m_sock = sock;
    m_isConnected = true;
    initSock();
    getLocalAddress();
    getRemoteAddress();
    return true;
  }
  return false;
}
std::ostream &operator<<(std::ostream &os, const Socket &sock) {
  return sock.dump(os);
}
