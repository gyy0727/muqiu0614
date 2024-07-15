#include "../include/IoManager.h"
#include "../include/Log.h"
#include "../include/macro.h"
#include <assert.h>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>
Logger::ptr g_logger = SYLAR_LOG_NAME("system");
enum EpollCtlOp {

};
static int createEventFd() {
  int evefd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evefd < 0) {
    SYLAR_LOG_ERROR(g_logger) << "m_wakeupfd create err " << errno;
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
    ctx.scheduler->schedule(ctx.cb);

  } else {
    ctx.scheduler->schedule(ctx.fiber);
  }
  ctx.scheduler = nullptr;
  return;
}

IOManager::IOManager(size_t threads, const std::string &name)
    : Scheduler(threads, name) {
  m_epollfd = epoll_create(5000);
  assert(m_epollfd > 0);
  m_wakeupfd = createEventFd();
  assert(m_wakeupfd);

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
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(1.5 * fd);
    fd_ctx = m_fdContexts[fd];
  }
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
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
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
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
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
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
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();
  FdContext::MutexType::Lock lock2(fd_ctx->mutex);

  if (!fd_ctx->events) {
    return false;
  }

  int op = EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
  if (rt) {
    SYLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epollfd << ", " << (EpollCtlOp)op << ", " << fd
        << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno
        << ") (" << strerror(errno) << ")";
    return false;
  }

  if (fd_ctx->events & READ) {
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }
  if (fd_ctx->events & WRITE) {
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }

  assert(fd_ctx->events == 0);
  return true;
}

IOManager *IOManager::getThis() {
  return dynamic_cast<IOManager *>(Scheduler::GetThis());
}

void IOManager::tickle() {
  if (!hasIdleThreads()) {
    return;
  }

  uint64_t one = 1;
  ssize_t n = write(m_wakeupfd, &one, sizeof one);
  if (n != sizeof one) {
    SYLAR_LOG_FATAL(g_logger) << "m_wakeupfd write err " << errno;
  }
}

//*引用是为了idle()里面直接传入修改next_timeout
bool IOManager::stopping(uint64_t &timeout) {
  timeout = getNextTimer();
  return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::stopping();
}
bool IOManager::stopping() {
  uint64_t timeout = 0;
  return stopping(
      timeout); //*没有定时器&&没有待执行的任务&&最重要一点(m_autostop为true)
}

void IOManager::idle() {
  SYLAR_LOG_INFO(g_logger) << "idle";
  const uint64_t MAX_EVENTS = 256;
  epoll_event *events = new epoll_event[MAX_EVENTS]();
  std::shared_ptr<epoll_event> shared_events(
      events, [](epoll_event *ptr) { delete[] ptr; });

  while (true) {
    uint64_t next_timeout = 0;
    if (UNLIKELY(stopping(next_timeout))) {
      SYLAR_LOG_INFO(g_logger) << "name = " << getName() << "idle stoping exit";
      break;
    }
    int rt = 0;
    do {
      static const int MAX_TIMEOUT = 3000;
      if (next_timeout != ~0ull) { //*即定时器容器不为空
        SYLAR_LOG_INFO(g_logger) << "there has timmer";
        next_timeout =
            (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
      } else {
        SYLAR_LOG_INFO(g_logger) << "there is no timmer";
        next_timeout = MAX_TIMEOUT;
      }
      rt = epoll_wait(m_epollfd, events, MAX_EVENTS, (int)next_timeout);
      if (rt < 0 && errno == EINTR) { //*系统中断

      } else {
        break;
      }
    } while (true);

    std::vector<std::function<void()>> cbs;
    SYLAR_LOG_INFO(g_logger) << "listExpiredCb start";
    listExpiredCb(cbs); //*将过期的定时器的任务存入其中

    SYLAR_LOG_INFO(g_logger) << "listExpiredCb end";
    if (!cbs.empty()) {
      schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }
    for (int i = 0; i < rt; ++i) {
      epoll_event &event = events[i];
      if (event.data.fd == m_wakeupfd) {
        uint64_t dummy[256];
        while (read(m_wakeupfd, dummy, sizeof(dummy)) > 0)
          ; //*先将内容全部读取
        continue;
      }
      FdContext *fd_ctx = (FdContext *)event.data.ptr;
      FdContext::MutexType::Lock lock(fd_ctx->mutex);
      if (event.events &
          (EPOLLERR |
           EPOLLHUP)) { //*意思是如果触发的事件包含了系统中断或者一系列错误,就要重新设置该epollfd的感兴趣事件
        event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
      }

      //*以下逻辑是用于判断具体发生了什么感兴趣的事件
      int real_events = NONE;
      if (event.events & EPOLLIN) {
        real_events |= READ;
      }
      if (event.events & EPOLLOUT) {
        real_events |= WRITE;
      }

      if ((fd_ctx->events & real_events) == NONE) {
        continue;
      }
      int left_events =
          (fd_ctx->events &
           ~real_events); //*剔除已经发生的感兴趣事件,然后剩下的没发生的事件
      int op = left_events ? EPOLL_CTL_MOD
                           : EPOLL_CTL_DEL; //*如果还有没发生的感兴趣事件
      event.events = EPOLLET | left_events;
      int rt2 = epoll_ctl(m_epollfd, op, fd_ctx->fd, &event);
      if (rt2) {
        SYLAR_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epollfd << ", " << (EpollCtlOp)op << ", "
            << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):" << rt2
            << " (" << errno << ") (" << strerror(errno) << ")";
        continue;
      }
      if (real_events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
      }
      if (real_events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
      }
    }
    //*这里使用裸指针是因为swapout之后当前函数相当于是暂停,不
    //会析构,可以导致智能指针对象无法减少引用计数,所以要自己手动减少
    Fiber::ptr cur = Fiber::getThis();
    auto raw_ptr = cur.get();
    cur.reset();

    raw_ptr->swapOut();
  }
}
void IOManager::onTimerInsertAtFront() { tickle(); }
