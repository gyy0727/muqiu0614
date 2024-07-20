#ifndef SOCKET_H
#define SOCKET_H

#include "Noncopyable.h"
#include "address.h"
#include <memory>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>

class Socket : public std::enable_shared_from_this<Socket>, Sylar::Noncopyable {
public:
  using sptr = std::shared_ptr<Socket>;
  using wptr = std::weak_ptr<Socket>;

  enum TYPE { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

  enum FAMILY { IPV4 = AF_INET, IPV6 = AF_INET6, UNIX = AF_UNIX };
  //*创建tcpsocket
  static Socket::sptr CreateTCP(Address::ptr address);
  //*创建udpsocket
  static Socket::sptr CreateUDP(Address::ptr address);
  //*创建ipv4的tcpsocket
  static Socket::sptr CreateTCPIPV4();
  //*创建ipv4的udpsocket
  static Socket::sptr CreateUDPIPV4();
  //*创建ipv6的tcpsocket
  static Socket::sptr CreateTCPIPV6();
  //*创建ipv6的udpsocket
  static Socket::sptr CreateUDPIPV6();
  //*创建unix tcpsocket
  static Socket::sptr CreateUnixTCP();
  //*创建unix udpsocket
  static Socket::sptr CreateUnixUDP();
  Socket(int family, int type, int protocol = 0);
  virtual ~Socket();
  //*获取发送超时时间
  int64_t getSendTimeout();
  //*设置发送超时时间
  void setSendTimeout(int64_t v);
  //*获取读取超时时间
  int64_t getRecvTimeout();
  //*设置读取超时时间
  void setRecvTimeout(int64_t v);
  //*获取socket选项
  bool getOption(int level, int option, void *result, socklen_t *len);

  template <class T> bool getOption(int level, int option, T &result) {
    socklen_t length = sizeof(T);
    return getOption(level, option, &result, &length);
  }
  //*设置socket选项
  bool setOption(int level, int option, const void *result, socklen_t len);

  template <class T> bool setOption(int level, int option, const T &value) {
    return setOption(level, option, &value, sizeof(T));
  }
  //*封装原始的accept函数
  virtual Socket::sptr accept();
  //*封装原始的bind函数
  virtual bool bind(const Address::ptr addr);
  //*封装原始的connect函数
  virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
  //*connect失败重新连接
  virtual bool reconnect(uint64_t timeout_ms = -1);
  //*封装原始的listen函数
  virtual bool listen(int backlog = SOMAXCONN);
  //*释放资源,关闭socket fd
  virtual bool close();
  //*发送数据
  virtual int send(const void *buffer, size_t length, int flags = 0);
  virtual int send(const iovec *buffers, size_t length, int flags = 0);
  //*零拷贝
  virtual int sendTo(const void *buffer, size_t length, const Address::ptr to,
                     int flags = 0);
  virtual int sendTo(const iovec *buffers, size_t length, const Address::ptr to,
                     int flags = 0);
  //*发送数据
  virtual int recv(void *buffer, size_t length, int flags = 0);
  virtual int recv(iovec *buffers, size_t length, int flags = 0);
  virtual int recvFrom(void *buffer, size_t length, Address::ptr from,
                       int flags = 0);

  virtual int recvFrom(iovec *buffers, size_t length, Address::ptr from,
                       int flags = 0);
  //*获取远端IP地址信息
  Address::ptr getRemoteAddress();
  //*获取本地ip地址信息
  Address::ptr getLocalAddress();
  //*获取协议簇信息
  int getFamily() const { return m_family; }
  //*获取类型
  int getType() const { return m_type; }
  //*获取协议
  int getProtocol() const { return m_protocol; }
  //*是否已连接
  bool isConnected() const { return m_isConnected; }
  //*是否非法[<32;170;40M]
  bool isValid() const;
  //*获取错误信息
  int getError();
  //*tostring,输入到字符串输出流
  virtual std::ostream &dump(std::ostream &os) const;

  virtual std::string toString() const;
  //*返回socket文件描述符
  int getSocket() const { return m_sock; }
  //*取消读事件
  bool cancelRead();
  //*取消写事件
  bool cancelWrite();
  //*取消accept事件
  bool cancelAccept();
  //*取消所有事件
  bool cancelAll();

protected:
  //*初始化
  void initSock();
  //*新建一个socketfd
  void newSock();
  //*是否已经初始化
  virtual bool init(int sock);

protected:
  /// socket句柄
  int m_sock;
  /// 协议簇
  int m_family;
  /// 类型
  int m_type;
  /// 协议
  int m_protocol;
  /// 是否连接
  bool m_isConnected;
  /// 本地地址
  Address::ptr m_localAddress;
  /// 远端地址
  Address::ptr m_remoteAddress;
};

//class SSLSocket : public Socket {
//public:
//  typedef std::shared_ptr<SSLSocket> ptr;
//
//  static SSLSocket::ptr CreateTCP(Address::ptr address);
//  static SSLSocket::ptr CreateTCPSocket();
//  static SSLSocket::ptr CreateTCPSocket6();
//
//  SSLSocket(int family, int type, int protocol = 0);
//  virtual Socket::sptr accept() override;
//  virtual bool bind(const Address::ptr addr) override;
//  virtual bool connect(const Address::ptr addr,
//                       uint64_t timeout_ms = -1) override;
//  virtual bool listen(int backlog = SOMAXCONN) override;
//  virtual bool close() override;
//  virtual int send(const void *buffer, size_t length, int flags = 0) override;
//  virtual int send(const iovec *buffers, size_t length, int flags = 0) override;
//  virtual int sendTo(const void *buffer, size_t length, const Address::ptr to,
//                     int flags = 0) override;
//  virtual int sendTo(const iovec *buffers, size_t length, const Address::ptr to,
//                     int flags = 0) override;
//  virtual int recv(void *buffer, size_t length, int flags = 0) override;
//  virtual int recv(iovec *buffers, size_t length, int flags = 0) override;
//  virtual int recvFrom(void *buffer, size_t length, Address::ptr from,
//                       int flags = 0) override;
//  virtual int recvFrom(iovec *buffers, size_t length, Address::ptr from,
//                       int flags = 0) override;
//
//  bool loadCertificates(const std::string &cert_file,
//                        const std::string &key_file);
//  virtual std::ostream &dump(std::ostream &os) const override;
//
//protected:
//  virtual bool init(int sock) override;
//
//private:
//  std::shared_ptr<SSL_CTX> m_ctx;
//  std::shared_ptr<SSL> m_ssl;
//};
std::ostream &operator<<(std::ostream &os, const Socket &sock);



#endif
