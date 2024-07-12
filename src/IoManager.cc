#include "../include/IoManager.h"
#include "../include/Log.h"
#include "../include/macro.h"
#include <cassert>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
Logger::ptr g_logger = SYLAR_LOG_NAME("system");
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

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
  FdContext *fd_ctx = nullptr;
  rwlock::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    rwlock::WriteLock lock2(m_mutex);
    contextResize(1.5 * fd);
    fd_ctx = m_fdContexts[fd];
  }
  std::unique_lock<std::mutex> lock2(fd_ctx->mutex);
  if (UNLIKELY(fd_ctx->events & event)) {
    SYLAR_LOG_ERROR(g_logger)
        << "addEvent assert fd=" << fd << " event=" << (EPOLL_EVENTS)event
        << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
  }
  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;
  int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epollfd << ", " << (EpollCtlOp)op << ", " << fd
        << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno
        << ") (" << strerror(errno)
        << ") fd_ctx->events=" << (EPOLL_EVENTS)fd_ctx->events;
    return -1;
  }
  ++m_pendingEventCount;
  fd_ctx->events = (Event)(fd_ctx->events | event);
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  assert(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);
  event_ctx.scheduler = Scheduler::GetThis();
  //*如果用户提供了任务就执行该任务,没有就执行当前在运行的协程
  if (cb) {
    event_ctx.cb.swap(cb);

  } else {
    // event_ctx.fiber = Fiber::getThis();
    // assert(event_ctx.fiber->getState() == Fiber::RUNING,
    //      "state=" << event_ctx.fiber->getState());
  }
  return 0;
}

bool IOManager::delEvent(int fd, Event event) {
  rwlock::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();
  std::unique_lock<std::mutex> lock2(fd_ctx->mutex);
  if (UNLIKELY(!(fd_ctx->events & event))) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epollfd << ", " << (EpollCtlOp)op << ", " << fd
        << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno
        << ") (" << strerror(errno) << ")";
    return false;
  }

  --m_pendingEventCount;
  fd_ctx->events = new_events;
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}











bool IOManager::cancelEvent(int fd, Event event) {
  rwlock::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();
  std::unique_lock<std::mutex> lock2(fd_ctx->mutex);
  if (UNLIKELY(!(fd_ctx->events & event))) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epollfd << ", " << (EpollCtlOp)op << ", " << fd
        << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno
        << ") (" << strerror(errno) << ")";
    return false;
  }

  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}




bool IOManager::cancelAll(int fd) {
    rwlock::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    std::unique_lock<std::mutex> lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
    if(rt) {
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    assert(fd_ctx->events == 0);
    return true;
}



IOManager* IOManager::getThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}





void IOManager::tickle() {
    if(!hasIdleThreads()) {
        return;
    }
    int rt = write(m_wakeupfd, "T", 1);
    assert(rt == 1);
}

bool IOManager::stopping(uint64_t timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();

}
bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}

































































