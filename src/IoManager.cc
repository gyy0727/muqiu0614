#include "../include/IoManager.h"
#include "../include/Log.h"
#include <cassert>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
enum EpollCtlOp {

};
int createEventFd() {
  int evefd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evefd < 0) {
    SYLAR_LOG_ERROR(g_logger) << "createEventFd fail";
  }
  return evefd;
}

static std::ostream &operator<<(std::ostream &os, const EpollCtlOp &op) {
  switch ((int)op) {
#define XX(ctl)                                                                \
  case ctl:                                                                    \
    return os << #ctl;
    XX(EPOLL_CTL_ADD);
    XX(EPOLL_CTL_MOD);
    XX(EPOLL_CTL_DEL);
  default:
    return os << (int)op;
  }
#undef XX
}

static std::ostream &operator<<(std::ostream &os, EPOLL_EVENTS events) {
  if (!events) {
    return os << "0";
  }
  bool first = true;
#define XX(E)                                                                  \
  if (events & E) {                                                            \
    if (!first) {                                                              \
      os << "|";                                                               \
    }                                                                          \
    os << #E;                                                                  \
    first = false;                                                             \
  }
  XX(EPOLLIN);
  XX(EPOLLPRI);
  XX(EPOLLOUT);
  XX(EPOLLRDNORM);
  XX(EPOLLRDBAND);
  XX(EPOLLWRNORM);
  XX(EPOLLWRBAND);
  XX(EPOLLMSG);
  XX(EPOLLERR);
  XX(EPOLLHUP);
  XX(EPOLLRDHUP);
  XX(EPOLLONESHOT);
  XX(EPOLLET);
#undef XX
  return os;
}
IOManager::FdContext::EventContext &
IOManager::FdContext::getContext(IOManager::Event event) {
  switch (event) {
  case IOManager::READ:
    return read;
  case IOManager::WRITE:
    return write;
  default:
    SYLAR_LOG_INFO(g_logger) << "getContext event is null";
  }
  throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(
    IOManager::FdContext::EventContext &ctx) {
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
  return;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
  events = (Event)(events & ~event);
  EventContext &ctx = getContext(event);
  if (ctx.cb) {
    ctx.scheduler->scheduler(&ctx.cb);

  } else {
    ctx.scheduler->scheduler(ctx.fiber);
  }
  ctx.scheduler = nullptr;
  return;
}
IOManager::IOManager(size_t threads, const std::string &name)
    : Scheduler(threads, name) {
  m_epollfd = epoll_create(5000);
  assert(m_epollfd > 0);
  m_wakeupfd = createEventFd();
  assert(!m_wakeupfd);

  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_wakeupfd;
  int rt = fcntl(m_wakeupfd, F_SETFL, O_NONBLOCK);
  assert(!rt);
  rt = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_wakeupfd, &event);
  assert(!rt);
  contextResize(32);
  start();
}
IOManager::~IOManager() {
  stop();
  close(m_epollfd);
  close(m_wakeupfd);
  for (size_t i = 0; i < m_fdContexts.size(); i++) {
    if (m_fdContexts[i]) {
      delete m_fdContexts[i];
    }
  }
}

void IOManager::contextResize(size_t size) {
  m_fdContexts.resize(size);
  for (size_t i = 0; i < m_fdContexts.size(); ++i) {
    if (!m_fdContexts[i]) {
      m_fdContexts[i] = new FdContext;
      m_fdContexts[i]->fd = i;
    }
  }
}
