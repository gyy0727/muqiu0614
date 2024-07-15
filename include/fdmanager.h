#ifndef FDMANAGER_H
#define FDMANAGER_H
#include "PThread.h"
#include "Singleton.h"
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
  using ptr = std::shared_ptr<FdCtx>;

  FdCtx(int fd);
  ~FdCtx();

  bool isInit() const { return m_isInit; }

  bool isSocket() const { return m_isSocket; }

  bool isClose() const { return m_isClosed; }
  void setUserNonblock(bool v) { m_userNonblock = v; }

  bool getUserNonblock() const { return m_userNonblock; }

  void setSysNonblock(bool v) { m_sysNonblock = v; }

  bool getSysNonblock() const { return m_sysNonblock; }

  void setTimeout(int type, uint64_t v);

  uint64_t getTimeout(int type);

private:
  bool init();
  //* 是否初始化
  bool m_isInit ;
  //* 是否socket
  bool m_isSocket;
  //* 是否hook非阻塞
  bool m_sysNonblock;
  //* 是否用户主动设置非阻塞
  bool m_userNonblock ;
  //* 是否关闭
  bool m_isClosed;
  //* 文件句柄
  int m_fd;
  //* 读超时时间毫秒
  uint64_t m_recvTimeout;
  //* 写超时时间毫秒
  uint64_t m_sendTimeout;
};

class FdManager {
public:
  typedef Sylar::RWMutex RWMutexType;
  FdManager();

  FdCtx::ptr get(int fd, bool auto_create = false);

  void del(int fd);

private:
  /// 读写锁
  RWMutexType m_mutex;
  /// 文件句柄集合
  std::vector<FdCtx::ptr> m_datas;
};

/// 文件句柄单例
typedef Sylar::Singleton<FdManager> FdMgr;
#endif // !FDMANAGER_H
;
