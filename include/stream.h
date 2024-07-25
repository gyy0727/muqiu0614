#ifndef STREAM_H
#define STREAM_H
#include "bytearray.h"
#include <memory>

class Stream {
public:
  typedef std::shared_ptr<Stream> ptr;
  virtual ~Stream() {}
  //*读取数据到buffer
  virtual int read(void *buffer, size_t length) = 0;
  //*读取数据到缓冲区ba
  virtual int read(ByteArray::ptr ba, size_t length) = 0;
  //*读取指定长度的数据到buffer,否则阻塞
  virtual int readFixSize(void *buffer, size_t length);
  //*读取指定长度的数据到ba,否则阻塞
  virtual int readFixSize(ByteArray::ptr ba, size_t length);
  //*写入数据到buffer
  virtual int write(const void *buffer, size_t length) = 0;
  //*写入数据到缓冲区
  virtual int write(ByteArray::ptr ba, size_t length) = 0;
  //*写入固定长度数据到buffer
  virtual int writeFixSize(const void *buffer, size_t length);
  //*写入固定长度数据到缓冲区ba
  virtual int writeFixSize(ByteArray::ptr ba, size_t length);
  virtual void close() = 0;
};

#endif
