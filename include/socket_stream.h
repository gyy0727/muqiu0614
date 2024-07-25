#ifndef SOCKET_STREAM_H
#define SOCKET_STREAM_H

#include "IoManager.h"
#include "socket.h"
#include "stream.h"
#include <memory>
#include <mutex>

class SocketStream : public Stream {
public:
  using ptr = std::shared_ptr<SocketStream>;

  SocketStream(Socket::sptr, bool owner = true);

  ~SocketStream();
  //*读取数据到buffer
  virtual int read(void *buffer, size_t length) override;
  //*读取数据到缓冲区ba
  virtual int read(ByteArray::ptr ba, size_t length) override;
  //*写入数据到buffer
  virtual int write(const void *buffer, size_t length) override;
  //*写入数据到缓冲区
  virtual int write(ByteArray::ptr ba, size_t length) override;
  virtual void close() override;
  Socket::sptr getSocket() const { return m_socket; };
  bool isConnected() const;
  Address::ptr getRemoteAddress();
  Address::ptr getLocalAddress();
  std::string getRemoteAddressString();
  std::string getLocalAddressString();

protected:
  Socket::sptr m_socket;
  bool m_owner;
};

#endif
