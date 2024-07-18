#include "../include/hook.h"
#include <dlfcn.h>
#include "../include/Fiber.h"
#include "../include/IoManager.h"
#include "../include/Log.h"
#include "../include/fdmanager.h"
#include "../include/macro.h"
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static uint64_t tcp_connect_timeout = 5000;

static thread_local bool t_hook_enable = false;

//*hook对应函数
#define HOOK_FUN(XX)                                                           \
  XX(sleep)                                                                    \
  XX(usleep)                                                                   \
  XX(nanosleep)                                                                \
  XX(socket)                                                                   \
  XX(connect)                                                                  \
  XX(accept)                                                                   \
  XX(read)                                                                     \
  XX(readv)                                                                    \
  XX(recv)                                                                     \
  XX(recvfrom)                                                                 \
  XX(recvmsg)                                                                  \
  XX(write)                                                                    \
  XX(writev)                                                                   \
  XX(send)                                                                     \
  XX(sendto)                                                                   \
  XX(sendmsg)                                                                  \
  XX(close)                                                                    \
  XX(fcntl)                                                                    \
  XX(ioctl)                                                                    \
  XX(getsockopt)                                                               \
  XX(setsockopt)

void hook_init() {
  static bool is_inited = false;
  if (is_inited) {
    return;
  }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
  HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;

struct _HOOKIniter {
  _HOOKIniter() {
    hook_init();
    s_connect_timeout = tcp_connect_timeout;
  }
};

static _HOOKIniter
    s_hook_initer; //*通过静态变量初始化实现在main函数之前执行代码

bool is_hook_enable() { return t_hook_enable; }

void set_hook_enable(bool flag) { t_hook_enable = flag; }

struct timer_info {
  int cancelled = 0;
};

template <class OriginFUn, class... Args>
static ssize_t do_io(int fd, OriginFUn fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args) {
  //*若是没设置可hook,则使用原函数
  if (!t_hook_enable) {
    return fun(fd, std::forward<Args>(args)...);
  }
  //*先从fdmanager取出对应的fd的信息
  FdCtx::ptr ctx = FdMgr::getInstance()->get(fd);
  //*如果不存在对应的fd
  if (!ctx) {
    return fun(fd, std::forward<Args>(args)...);
  }
  //*如果文件描述符已经被关闭或者说还没启用
  if (ctx->isClose()) {
    errno = EBADF;
    return -1;
  }
  //*如果不是socket_fd或者由用户设置的非阻塞文件描述符
  if (!ctx->isSocket() || ctx->getUserNonblock()) {
    return fun(fd, std::forward<Args>(args)...);
  }
  //*拿出对应的超时时间
  uint64_t to = ctx->getTimeout(timeout_so);
  //*条件定时器的条件
  std::shared_ptr<timer_info> tinfo(new timer_info);
retry:
  //*执行传入的回调函数
  ssize_t n = fun(fd, std::forward<Args>(args)...);
  //*被系统中断或者出现错误
  while (n == -1 && errno == EINTR) {
    n = fun(fd, std::forward<Args>(args)...);
  }
  //*非阻塞文件描述符读写到末尾
  if (n == -1 && errno == EAGAIN) {
    IOManager *iom = IOManager::getThis();
    Timer::ptr timer;
    std::weak_ptr<timer_info> winfo(tinfo);
    //*超时时间是有效的
    if (to != (uint64_t)-1) {
      timer = iom->addConditionTimer(
          to,
          [winfo, fd, iom, event]() {
            auto t = winfo.lock();
            if (!t || t->cancelled) {
              return;
            }
            //*设置标志位为超时
            t->cancelled = ETIMEDOUT;
            //*触发并取消事件
            iom->cancelEvent(fd, (IOManager::Event)(event));
          },
          winfo);
    }
    //*添加对应事件
    int rt = iom->addEvent(fd, (IOManager::Event)(event));
    //*添加失败
    if (UNLIKELY(rt)) {
      SYLAR_LOG_ERROR(g_logger)
          << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
      if (timer) {
        //*取消定时器,因为 走到这里证明还没超时就完成了
        timer->cancel();
      }
      return -1;
    } else {
      Fiber::getThis()->swapOut();
      if (timer) {
        //*同理如上
        timer->cancel();
      }
      //*如果是超时了
      if (tinfo->cancelled) {
        errno = tinfo->cancelled;
        return -1;
      }
      //*数据来了,因为走到这里证明是事件被触发了,所以重新执行,connect_with_timeout
      //*不用重新执行是因为他是一次性的操作,走到最后证明肯定连接上了或者说连接失败超时了
      //*而do_io则是证明数据到了,他的操作过程是之前写了一部分没写完,或者说有新的了,所以说要再来一次
      goto retry;
    }
  }

  return n;
}

extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
  if (!t_hook_enable) {
    return sleep_f(seconds);
  }

  Fiber::ptr fiber = Fiber::getThis();
  IOManager *iom = IOManager::getThis();
  //*强转,我的理解是因为Scheduler里面有两个schdule函数,得强制确定是哪一个
  iom->addTimer(seconds * 1000,
                std::bind((void(Scheduler::*)(Fiber::ptr, int thread)) &
                              IOManager::schedule,
                          iom, fiber, -1));
  Fiber::getThis()->swapOut();
  return 0;
}

int usleep(useconds_t usec) {
  if (!t_hook_enable) {
    return usleep_f(usec);
  }
  Fiber::ptr fiber = Fiber::getThis();
  IOManager *iom = IOManager::getThis();
  iom->addTimer(usec / 1000, std::bind((void(Scheduler::*)(Fiber::ptr, int)) &
                                           IOManager::schedule,
                                       iom, fiber, -1));
  fiber->swapOut();
  return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
  if (!t_hook_enable) {
    return nanosleep_f(req, rem);
  }

  int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
  Fiber::ptr fiber = Fiber::getThis();
  IOManager *iom = IOManager::getThis();
  iom->addTimer(timeout_ms,
                std::bind((void(Scheduler::*)(Fiber::ptr, int thread)) &
                              IOManager::schedule,
                          iom, fiber, -1));
  fiber->swapOut();
  return 0;
}

//*其实就是先创建然后将fd加入fdmanager
int socket(int domain, int type, int protocol) {
  if (!t_hook_enable) {
    return socket_f(domain, type, protocol);
  }
  int fd = socket_f(domain, type, protocol);
  if (fd == -1) {
    return fd;
  }
  FdMgr::getInstance()->get(fd, true);
  return fd;
}

int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen,
                         uint64_t timeout_ms) {
  if (t_hook_enable) {
    return connect_f(fd, addr, addrlen);
  }
  //*从文件描述符管理器获取对应的属性信息
  FdCtx::ptr ctx = FdMgr::getInstance()->get(fd);
  if (!ctx || ctx->isClose()) {
    errno = EBADF;
    return -1;
  }
  //*文件描述符是否是scoket_fd
  if (!ctx->isSocket()) {
    return connect_f(fd, addr, addrlen);
  }
  //*是否是用户设置的非阻塞
  if (ctx->getUserNonblock()) {
    return connect_f(fd, addr, addrlen);
  }
  //*套接字是非阻塞的，并且连接操作不能立即完成
  //*这里的作用是先连接,然后如果无法立刻建立连接
  //*在后面加入定时器
  int n = connect_f(fd, addr, addrlen);
  if (n == 0) {
    return 0;
  } else if (n != -1 || errno != EINPROGRESS) {
    return n;
  }

  IOManager *iom = IOManager::getThis();
  Timer::ptr timer;
  std::shared_ptr<timer_info> tinfo(new timer_info);
  std::weak_ptr<timer_info> winfo(tinfo);
  //*如果用户设置的time_out是有效的,那就添加定时器
  if (timeout_ms != (uint64_t)-1) {
    //*添加条件定时器,条件就是tinfo析构了没
    timer = iom->addConditionTimer(
        timeout_ms,
        [winfo, fd, iom]() {
          //*先判断tinfo析构了没
          auto t = winfo.lock();
          //*如果tinfo已经析构,或者已经被取消
          if (!t || t->cancelled) {
            return;
          }
          //*否则设置为超时
          t->cancelled = ETIMEDOUT;
          //*取消事件,并且会强制触发一次
          //但是添加事件的时候给他的回调函数是空指针,所以会回到这个函数之前的上下文
          iom->cancelEvent(fd, IOManager::WRITE);
        },
        winfo);
  }
  //*添加事件
  int rt = iom->addEvent(fd, IOManager::WRITE);
  //*添加事件成功
  if (rt == 0) {
    //*切出去,addEvent 那里,因为没设置回调函数,所以会把当前函数上下文作为回调
    Fiber::getThis()->swapOut();
    //*时间触发的时候会重新回来这里执行
    if (timer) {
      //*取消定时器,注意这里并不会触发定时器的回调
      timer->cancel();
    }
    //*超时了
    if (tinfo->cancelled) {
      errno = tinfo->cancelled;
      return -1;
    }
  } else {
    //*事件添加失败
    if (timer) {
      timer->cancel();
    }
    SYLAR_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
  }

  int error = 0;
  socklen_t len = sizeof(int);
  //*检查套接字是否出现错误
  if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
    return -1;
  }
  if (!error) {
    return 0;
  } else {
    errno = error;
    return -1;
  }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return connect_with_timeout(sockfd, addr, addrlen, s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  int fd =
      do_io(s, accept_f, "accept", IOManager::READ, SO_RCVTIMEO, addr, addrlen);
  if (fd >= 0) {
    FdMgr::getInstance()->get(fd, true);
  }
  return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
  return do_io(fd, read_f, "read", IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
  return do_io(fd, readv_f, "readv", IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
  return do_io(sockfd, recv_f, "recv", IOManager::READ, SO_RCVTIMEO, buf, len,
               flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
  return do_io(sockfd, recvfrom_f, "recvfrom", IOManager::READ, SO_RCVTIMEO,
               buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
  return do_io(sockfd, recvmsg_f, "recvmsg", IOManager::READ, SO_RCVTIMEO, msg,
               flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
  return do_io(fd, write_f, "write", IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
  return do_io(fd, writev_f, "writev", IOManager::WRITE, SO_SNDTIMEO, iov,
               iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
  return do_io(s, send_f, "send", IOManager::WRITE, SO_SNDTIMEO, msg, len,
               flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags,
               const struct sockaddr *to, socklen_t tolen) {
  return do_io(s, sendto_f, "sendto", IOManager::WRITE, SO_SNDTIMEO, msg, len,
               flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
  return do_io(s, sendmsg_f, "sendmsg", IOManager::WRITE, SO_SNDTIMEO, msg,
               flags);
}

int close(int fd) {
  if (!t_hook_enable) {
    return close_f(fd);
  }
  //*先获得对应的文件描述符属性
  FdCtx::ptr ctx = FdMgr::getInstance()->get(fd);
  if (ctx) {
    auto iom = IOManager::getThis();
    if (iom) {
      iom->cancelAll(fd);
    }
    FdMgr::getInstance()->del(fd);
  }
  return close_f(fd);
}

//*================================================用处不大,照抄源码=========================================
int fcntl(int fd, int cmd, ... /* arg */) {
  va_list va;
  va_start(va, cmd);
  switch (cmd) {
  case F_SETFL: {
    int arg = va_arg(va, int);
    va_end(va);
    FdCtx::ptr ctx = FdMgr::getInstance()->get(fd);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return fcntl_f(fd, cmd, arg);
    }
    ctx->setUserNonblock(arg & O_NONBLOCK);
    if (ctx->getSysNonblock()) {
      arg |= O_NONBLOCK;
    } else {
      arg &= ~O_NONBLOCK;
    }
    return fcntl_f(fd, cmd, arg);
  } break;
  case F_GETFL: {
    va_end(va);
    int arg = fcntl_f(fd, cmd);
    FdCtx::ptr ctx = FdMgr::getInstance()->get(fd);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return arg;
    }
    if (ctx->getUserNonblock()) {
      return arg | O_NONBLOCK;
    } else {
      return arg & ~O_NONBLOCK;
    }
  } break;
  case F_DUPFD:
  case F_DUPFD_CLOEXEC:
  case F_SETFD:
  case F_SETOWN:
  case F_SETSIG:
  case F_SETLEASE:
  case F_NOTIFY:
#ifdef F_SETPIPE_SZ
  case F_SETPIPE_SZ:
#endif
  {
    int arg = va_arg(va, int);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  } break;
  case F_GETFD:
  case F_GETOWN:
  case F_GETSIG:
  case F_GETLEASE:
#ifdef F_GETPIPE_SZ
  case F_GETPIPE_SZ:
#endif
  {
    va_end(va);
    return fcntl_f(fd, cmd);
  } break;
  case F_SETLK:
  case F_SETLKW:
  case F_GETLK: {
    struct flock *arg = va_arg(va, struct flock *);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  } break;
  case F_GETOWN_EX:
  case F_SETOWN_EX: {
    struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
    va_end(va);
    return fcntl_f(fd, cmd, arg);
  } break;
  default:
    va_end(va);
    return fcntl_f(fd, cmd);
  }
}

int ioctl(int d, unsigned long int request, ...) {
  va_list va;
  va_start(va, request);
  void *arg = va_arg(va, void *);
  va_end(va);

  if (FIONBIO == request) {
    bool user_nonblock = !!*(int *)arg;
    FdCtx::ptr ctx = FdMgr::getInstance()->get(d);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return ioctl_f(d, request, arg);
    }
    ctx->setUserNonblock(user_nonblock);
  }
  return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval,
               socklen_t *optlen) {
  return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen) {
  if (!t_hook_enable) {
    return setsockopt_f(sockfd, level, optname, optval, optlen);
  }
  if (level == SOL_SOCKET) {
    if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
      FdCtx::ptr ctx = FdMgr::getInstance()->get(sockfd);
      if (ctx) {
        const timeval *v = (const timeval *)optval;
        ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
      }
    }
  }
  return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}
