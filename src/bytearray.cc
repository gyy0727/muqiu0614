#include "../include/Log.h"
#include "../include/bytearray.h"
#include "../include/ntmlgb.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>
using namespace sylar;
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

//*链表的初始化
ByteArray::Node::Node(size_t size)
    : ptr(new char[size]), next(nullptr), size(size) {}

ByteArray::Node::Node() : ptr(nullptr), next(nullptr), size(0) {}

//*释放资源避免内存泄漏
ByteArray::Node::~Node() {
  if (ptr) {
    delete[] ptr;
  }
}

//*缓冲区初始化
ByteArray::ByteArray(size_t base_size)
    : m_baseSize(base_size), m_position(0), m_capacity(base_size), m_size(0),
      m_endian(SYLAR_BIG_ENDIAN), m_root(new Node(base_size)), m_cur(m_root) {
  //*base_size  缓冲区大小
  //*m_position 缓冲区起始数据指针
  //*m_capacity 缓冲区容量
  //*m_size     当前数据占据的大小
  //*m_endian   缓冲区使用的字节序
  //*m_root     缓冲区指针
}

//*释放m_root
ByteArray::~ByteArray() {
  Node *tmp = m_root;
  while (tmp) {
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
}

//*返回缓冲区是否是小端字节序
bool ByteArray::isLittleEndian() const {
  return m_endian == SYLAR_LITTLE_ENDIAN;
}

//*根据val的值设置当前的字节序
void ByteArray::setIsLittleEndian(bool val) {
  if (val) {
    m_endian = SYLAR_LITTLE_ENDIAN;
  } else {
    m_endian = SYLAR_BIG_ENDIAN;
  }
}

void ByteArray::writeFint8(int8_t value) { write(&value, sizeof(value)); }

void ByteArray::writeFuint8(uint8_t value) { write(&value, sizeof(value)); }

void ByteArray::writeFint16(int16_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}
void ByteArray::writeFint32(int32_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

//*把有符号的转换成无符号类型,其实就是把负数映射到了奇数,整数映射到了偶数
static uint32_t EncodeZigzag32(const int32_t &v) {
  if (v < 0) {
    return ((uint32_t)(-v)) * 2 - 1;
  } else {
    return v * 2;
  }
}

static uint64_t EncodeZigzag64(const int64_t &v) {
  if (v < 0) {
    return ((uint64_t)(-v)) * 2 - 1;
  } else {
    return v * 2;
  }
}

//*把无符号转换成有符号类型
static int32_t DecodeZigzag32(const uint32_t &v) { return (v >> 1) ^ -(v & 1); }

static int64_t DecodeZigzag64(const uint64_t &v) { return (v >> 1) ^ -(v & 1); }

void ByteArray::writeInt32(int32_t value) {
  writeUint32(EncodeZigzag32(value));
}
//*压缩数据,将整数转换成变长编码
//*按七位一组划分
//*例如7,00001111,那第一组就是10001111,最高位用来标识后续有没有分组
//*这里为了特点鲜明,设为1,本来是0
void ByteArray::writeUint32(uint32_t value) {
  uint8_t tmp[5];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  write(tmp, i);
}

//*先转换成无符号再输入
void ByteArray::writeInt64(int64_t value) {
  writeUint64(EncodeZigzag64(value));
}

//*之所以设置为10个分组,显而易见64位要分成64/7=9....1
void ByteArray::writeUint64(uint64_t value) {
  uint8_t tmp[10];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  write(tmp, i);
}

void ByteArray::writeFloat(float value) {
  uint32_t v;
  memcpy(&v, &value, sizeof(value));
  writeFuint32(v);
}

void ByteArray::writeDouble(double value) {
  uint64_t v;
  memcpy(&v, &value, sizeof(value));
  writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string &value) {
  writeFuint16(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string &value) {
  writeFuint32(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string &value) {
  writeFuint64(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string &value) {
  writeUint64(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string &value) {
  write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
  int8_t v;
  read(&v, sizeof(v));
  return v;
}

uint8_t ByteArray::readFuint8() {
  uint8_t v;
  read(&v, sizeof(v));
  return v;
}

#define XX(type)                                                               \
  type v;                                                                      \
  read(&v, sizeof(v));                                                         \
  if (m_endian == SYLAR_BYTE_ORDER) {                                          \
    return v;                                                                  \
  } else {                                                                     \
    return byteswap(v);                                                        \
  }

int16_t ByteArray::readFint16() { XX(int16_t); }
uint16_t ByteArray::readFuint16() { XX(uint16_t); }

int32_t ByteArray::readFint32() { XX(int32_t); }

uint32_t ByteArray::readFuint32() { XX(uint32_t); }

int64_t ByteArray::readFint64() { XX(int64_t); }

uint64_t ByteArray::readFuint64() { XX(uint64_t); }

#undef XX

int32_t ByteArray::readInt32() { return DecodeZigzag32(readUint32()); }

uint32_t ByteArray::readUint32() {
  uint32_t result = 0;
  for (int i = 0; i < 32; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= ((uint32_t)b) << i;
      break;
    } else {
      result |= (((uint32_t)(b & 0x7f)) << i);
    }
  }
  return result;
}

int64_t ByteArray::readInt64() { return DecodeZigzag64(readUint64()); }

uint64_t ByteArray::readUint64() {
  uint64_t result = 0;
  for (int i = 0; i < 64; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= ((uint64_t)b) << i;
      break;
    } else {
      result |= (((uint64_t)(b & 0x7f)) << i);
    }
  }
  return result;
}

float ByteArray::readFloat() {
  uint32_t v = readFuint32();
  float value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

double ByteArray::readDouble() {
  uint64_t v = readFuint64();
  double value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

std::string ByteArray::readStringF16() {
  uint16_t len = readFuint16();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringF32() {
  uint32_t len = readFuint32();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringF64() {
  uint64_t len = readFuint64();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

std::string ByteArray::readStringVint() {
  uint64_t len = readUint64();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

void ByteArray::clear() {
  m_position = m_size = 0;
  m_capacity = m_baseSize;
  Node *tmp = m_root->next;
  while (tmp) {
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
  m_cur = m_root;
  m_root->next = NULL;
}

void ByteArray::write(const void *buf, size_t size) {
  if (size == 0) {
    return;
  }
  //*增加数据大小
  addCapacity(size);
  //*当前块的偏移量
  size_t npos = m_position % m_baseSize;
  //*计算当前块中从偏移位置到块末尾的剩余容量
  size_t ncap = m_cur->size - npos;
  //*已经写入的数据大小
  size_t bpos = 0;
  //*剩余的没写入的字节数大于0
  while (size > 0) {
    //*当前块有足够的空间存入数据
    if (ncap >= size) {
      memcpy(m_cur->ptr + npos, (const char *)buf + bpos, size);
      if (m_cur->size == (npos + size)) {
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    } else {
      //写满这个块写下一个块
      memcpy(m_cur->ptr + npos, (const char *)buf + bpos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }

  if (m_position > m_size) {
    m_size = m_position;
  }
}

void ByteArray::read(void *buf, size_t size) {
  //*要读取的字节数比可读的还大
  if (size > getReadSize()) {
    throw std::out_of_range("not enough len");
  }
  //*计算块内偏移
  size_t npos = m_position % m_baseSize;
  //*计算块内剩余容量
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;
  while (size > 0) {
    if (ncap >= size) {
      memcpy((char *)buf + bpos, m_cur->ptr + npos, size);
      if (m_cur->size == (npos + size)) {
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy((char *)buf + bpos, m_cur->ptr + npos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }
}

void ByteArray::read(void *buf, size_t size, size_t position) const {
  //*要读取的数据和实际的数据大小匹配不上
  if (size > (m_size - position)) {
    throw std::out_of_range("not enough len");
  }
  //*计算块内偏移量
  size_t npos = position % m_baseSize;
  //*当前块剩余容量
  size_t ncap = m_cur->size - npos;
  //*已经读取的大小
  size_t bpos = 0;

  Node *cur = m_cur;
  //*还有没读取的字节
  while (size > 0) {
    if (ncap >= size) {
      memcpy((char *)buf + bpos, cur->ptr + npos, size);
      if (cur->size == (npos + size)) {
        cur = cur->next;
      }
      position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy((char *)buf + bpos, cur->ptr + npos, ncap);
      position += ncap;
      bpos += ncap;
      size -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
  }
}

void ByteArray::setPosition(size_t v) {
  if (v > m_capacity) {
    throw std::out_of_range("set_position out of range");
  }
  m_position = v;
  if (m_position > m_size) {
    m_size = m_position;
  }
  m_cur = m_root;
  while (v > m_cur->size) {
    v -= m_cur->size;
    m_cur = m_cur->next;
  }
  if (v == m_cur->size) {
    m_cur = m_cur->next;
  }
}

bool ByteArray::writeToFile(const std::string &name) const {
  std::ofstream ofs;
  ofs.open(name, std::ios::trunc | std::ios::binary);
  if (!ofs) {
    SYLAR_LOG_ERROR(g_logger)
        << "writeToFile name=" << name << " error , errno=" << errno
        << " errstr=" << strerror(errno);
    return false;
  }

  int64_t read_size = getReadSize();
  int64_t pos = m_position;
  Node *cur = m_cur;

  while (read_size > 0) {
    int diff = pos % m_baseSize;
    int64_t len =
        (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
    ofs.write(cur->ptr + diff, len);
    cur = cur->next;
    pos += len;
    read_size -= len;
  }

  return true;
}

bool ByteArray::readFromFile(const std::string &name) {
  std::ifstream ifs;
  ifs.open(name, std::ios::binary);
  if (!ifs) {
    SYLAR_LOG_ERROR(g_logger)
        << "readFromFile name=" << name << " error, errno=" << errno
        << " errstr=" << strerror(errno);
    return false;
  }

  std::shared_ptr<char> buff(new char[m_baseSize],
                             [](char *ptr) { delete[] ptr; });
  while (!ifs.eof()) {
    ifs.read(buff.get(), m_baseSize);
    write(buff.get(), ifs.gcount());
  }
  return true;
}
//*检查是否装得下,装不下就扩展链表
void ByteArray::addCapacity(size_t size) {
  if (size == 0) {
    return;
  }
  size_t old_cap = getCapacity();
  if (old_cap >= size) {
    return;
  }

  size = size - old_cap;
  size_t count = ceil(1.0 * size / m_baseSize);
  Node *tmp = m_root;
  while (tmp->next) {
    tmp = tmp->next;
  }

  Node *first = NULL;
  for (size_t i = 0; i < count; ++i) {
    tmp->next = new Node(m_baseSize);
    if (first == NULL) {
      first = tmp->next;
    }
    tmp = tmp->next;
    m_capacity += m_baseSize;
  }

  if (old_cap == 0) {
    m_cur = first;
  }
}

std::string ByteArray::toString() const {
  std::string str;
  str.resize(getReadSize());
  if (str.empty()) {
    return str;
  }
  read(&str[0], str.size(), m_position);
  return str;
}

std::string ByteArray::toHexString() const {
  std::string str = toString();
  std::stringstream ss;

  for (size_t i = 0; i < str.size(); ++i) {
    if (i > 0 && i % 32 == 0) {
      ss << std::endl;
    }
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i]
       << " ";
  }

  return ss.str();
}
//*struct iovec {
//*    void  *iov_base;  // 指向内存缓冲区的指针
//*    size_t iov_len;   // 缓冲区的长度（字节数）
//*};

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers,
                                   uint64_t len) const {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  struct iovec iov;
  Node *cur = m_cur;

  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len,
                                   uint64_t position) const {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = position % m_baseSize;
  size_t count = position / m_baseSize;
  Node *cur = m_root;
  while (count > 0) {
    cur = cur->next;
    --count;
  }

  size_t ncap = cur->size - npos;
  struct iovec iov;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec> &buffers, uint64_t len) {
  if (len == 0) {
    return 0;
  }
  addCapacity(len);
  uint64_t size = len;

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  struct iovec iov;
  Node *cur = m_cur;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;

      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}
