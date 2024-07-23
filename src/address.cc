#include "../include/address.h"
#include "../include/Log.h"
#include "../include/ntmlgb.h"
#include <cstddef>
#include <ifaddrs.h>
#include <netdb.h>
#include <sstream>
#include <stddef.h>
using namespace sylar;
// using sylar::detail::byteswapOnBigEndian;
// using sylar::detail::byteswapOnLittleEndian;


static Sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("name");
//*求子网掩码,根据前缀长度创建掩码
template <class T> static T CreateMask(uint32_t bits) {
  //* 将1 左移 (sizeof(T)*8-bits)位再减一
  return (1 << (sizeof(T) * 8 - bits)) - 1;
}

//*根据掩码计算前缀长度
template <class T> static uint32_t CountBytes(T value) {
  uint32_t result = 0;
  for (; value; ++result) {
    value &= value - 1;
  }
  return result;
}

//*根据host返回任意ip
Address::ptr Address::LookupAny(const std::string &host, int family, int type,
                                int protocol) {
  std::vector<Address::ptr> result;
  if (lookUp(result, host, family, type, protocol)) {
    return result[0];
  }
  return nullptr;
}

//*根据host返回任意IPAddress
IPAddress::ptr Address::LookupAnyIPAddress(const std::string &host, int family,
                                           int type, int protocol) {
  std::vector<Address::ptr> result;
  if (lookUp(result, host, family, type, protocol)) {
    for (auto &i : result) {
      IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
      if (v) {
        return v;
      }
    }
  }
  return nullptr;
}

/*
* struct addrinfo {
*    int             ai_flags; 例如AI_PASSIVE、AI_CANONNAME等，控制解析行为
*   int             ai_family;       如 AF_INET (IPv4) 或 AF_INET6(IPv6)
*   int             ai_socktype;     如 SOCK_STREAM (TCP) 或 SOCK_DGRAM (UDP)
*   int             ai_protocol;     协议，如 IPPROTO_TCP 或 IPPROTO_UDP
*   size_t          ai_addrlen;      套接字地址的长度
*    struct sockaddr *ai_addr;        指向包含实际网络地址的sockaddr结构体的指针
*    char           *ai_canonname;    服务位置的规范主机名
*    struct addrinfo *ai_next;        指向链表中下一个 addrinfo 结构体的指针
};
*/

//*其实就是根据给定host,通过getaddrinfo()函数解析出该host关联的ip:port
//,存入对应的vector容器
bool Address::lookUp(std::vector<Address::ptr> &result, const std::string &host,
                     int family, int type, int protocol) {

  addrinfo hints, *results, *next;
  hints.ai_flags = 0;
  hints.ai_family = family;
  hints.ai_socktype = type;
  hints.ai_protocol = protocol;
  hints.ai_addrlen = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  std::string node; //*ip

  const char *service = NULL; //* port
  if (!host.empty() && host[0] == '[') {
    //*endipv6 指向ipv6的结尾[2001:db8::8:800:200C:417A]:80
    const char *endipv6 =
        (const char *)memchr(host.c_str() + 1, ']', host.size() - 1);
    if (endipv6) {
      if (*(endipv6 + 1) == ':') {
        //*判断是否包含端口号
        service = endipv6 + 2;
      }
      //*提取ipv6地址
      node = host.substr(1, endipv6 - host.c_str() - 1);
    }
  }

  //*证明不是ipv6,现在开始找ipv4
  if (node.empty()) {
    service = (const char *)memchr(host.c_str(), ':', host.size());
    if (service) {
      if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
        node = host.substr(0, service - host.c_str());
        ++service; //*指向端口号开始的地方
      }
    }
  }
  if (node.empty()) {
    node = host;
  }
  int error = getaddrinfo(node.c_str(), service, &hints, &results);
  if (error) {
    SYLAR_LOG_DEBUG(g_logger)
        << "Address::Lookup getaddress(" << host << ", " << family << ", "
        << type << ") err=" << error << " errstr=" << gai_strerror(error);
    return false;
  }
  next = results;
  while (next) {
    result.push_back(Address::create(next->ai_addr, next->ai_addrlen));
    next = next->ai_next;
  }

  freeaddrinfo(results); //* free the memory apply by getaddrinfo()
  return !result.empty();
}

// GetInterfaceAddresses（返回本机所有网卡的<网卡名, 地址, 子网掩码位数>）
/*
 *struct ifaddrs {
 *    struct ifaddrs *ifa_next;         //指向下一个结构体
 *    char *ifa_name;                   //接口名称,网卡名称
 *    u_int ifa_flags;                  //网卡状态标志
 *    struct sockaddr *ifa_addr;        //网卡的IP地址
 *    struct sockaddr *ifa_netmask;     //子网掩码
 *    struct sockaddr *ifa_dstaddr;     //目的地址
 *    void *ifa_data;                   //存储驱动程序特定数据的指针
 *    };
 */
bool Address::GetInterfaceAddresses(
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result,
    int family) {
  struct ifaddrs *next, *results;
  if (getifaddrs(&results) != 0) {
    SYLAR_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
                                 " err="
                              << errno << " errstr=" << strerror(errno);
    return false;
  }
  try {
    for (next = results; next; next = next->ifa_next) {
      Address::ptr addr;         //* ip
      uint32_t prefix_len = ~0u; //* 子网掩码前缀长度
      // AF_UNSPEC 代表未指定或全部
      //*即当前接口的类型不符合预期所需
      if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
        continue;
      }
      switch (next->ifa_addr->sa_family) {
      case AF_INET: {
        addr = create(next->ifa_addr, sizeof(sockaddr_in));
        uint32_t netmask = ((sockaddr_in *)next->ifa_netmask)
                               ->sin_addr.s_addr; //*ip的二进制形式
        prefix_len = CountBytes(netmask);
      } break;
      case AF_INET6: {
        addr = create(next->ifa_addr, sizeof(sockaddr_in6));
        in6_addr &netmask = ((sockaddr_in6 *)next->ifa_netmask)->sin6_addr;
        prefix_len = 0;
        //*ipv6的IP地址不是单纯的128位二进制数
        //*而是多个8位的数组组成
        for (int i = 0; i < 16; ++i) {
          prefix_len += CountBytes(netmask.s6_addr[i]);
        }

      } break;
      default:
        break;
      }
      if (addr) {
        result.insert(
            std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
      }
    }
  } catch (...) {
    SYLAR_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
  }
  freeifaddrs(results);
  return false;
}

// GetInterfaceAddresses（获取指定网卡的地址和子网掩码位数）
bool Address::GetInterfaceAddresses(
    std::vector<std::pair<Address::ptr, uint32_t>> &result,
    const std::string &iface, int family) {
  //*没指定网卡或者任意一个网卡都行
  if (iface.empty() || iface == "*") {
    //*属于ipv4或者任意一个都行
    if (family == AF_INET || family == AF_UNSPEC) {
      //*任意的ipv4
      result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
    }
    //*任意的ipv6
    if (family == AF_INET6 || family == AF_UNSPEC) {
      result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
    }
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;

    if (!GetInterfaceAddresses(results, family)) {
      return false;
    }
    //*返回一个区间,所有和iface有关联的键值对
    //*first指向区间的第一个,second指向区间的最后一个元素的下一位
    auto its = results.equal_range(iface);
    for (; its.first != its.second; ++its.first) {
      result.push_back(its.first->second);
    }
  }
  //*要得到的是所有指定网卡的信息
  return !result.empty();
}

int Address::getFamily() const { return getAddr()->sa_family; }

std::string Address::toString() const {
  std::stringstream ss;
  insert(ss);
  return ss.str();
}
Address::ptr Address::create(const sockaddr *addr, socklen_t addrlen) {
  if (addr == nullptr) {
    return nullptr;
  }
  Address::ptr result;
  switch (addr->sa_family) {
  case AF_INET:
    //*解引用是因为sockaddr是一个联合体
    //*而且构造函数接受的是引用类型,而不是指针类型
    result.reset(new IPv4Address(*(const sockaddr_in *)addr));
    break;
  case AF_INET6:
    result.reset(new IPv6Address(*(const sockaddr_in6 *)addr));
    break;
  default:
    result.reset(new UnknownAddress(*addr));
  }
  return result;
}

//*===================================比较两个地址=====================

bool Address::operator<(const Address &rhs) const {
  //*防止超出双方的地址范围
  socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
  //*memcmp 函数用于比较两个内存区域的前 minlen 字节。如果两段内存完
  //*全相同，则返回 0；如果当前对象的地址字节序列小于 rhs 的
  //*地址字节序列，则返回负值；如果大于，则返回正值。
  int result = memcmp(getAddr(), rhs.getAddr(), minlen);
  if (result < 0) {
    return true;
  } else if (result > 0) {

    return false;
  } else if (getAddrLen() < rhs.getAddrLen()) {
    return true;
  }
  return false;
}

//*长度,地址的每一个字节相等
bool Address::operator==(const Address &rhs) const {
  return getAddrLen() == rhs.getAddrLen() &&
         memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address &rhs) const { return !(*this == rhs); }

//*根据Address创建IPv4Address
IPAddress::ptr IPAddress::Create(const char *address, uint16_t port) {
  addrinfo hints, *results;
  memset(&hints, 0, sizeof(addrinfo));
  //*表示 getaddrinfo 应该期望主机名参数是数字形式的
  hints.ai_flags = AI_NUMERICHOST;
  //*表示协议簇是未指定或全部
  hints.ai_family = AF_UNSPEC;
  int error = getaddrinfo(address, NULL, &hints, &results);
  if (error) {
    SYLAR_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address << ", " << port
                              << ") error=" << error << " errno=" << errno
                              << " errstr=" << strerror(errno);
    return nullptr;
  }
  try {
    IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
        Address::create(results->ai_addr, (socklen_t)results->ai_addrlen));
    if (result) {
      result->setPort(port);
    }
    freeaddrinfo(results);

  } catch (...) {
    freeaddrinfo(results);
    return nullptr;
  }
  return nullptr;
}

IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port) {
  IPv4Address::ptr rt(new IPv4Address);
  //*将主机字节序转换成网络字节序
  rt->m_addr.sin_port = byteswapOnLittleEndian(port);
  //*将字符串形式的ip转换成二进制形式存储到rt
  int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
  if (result <= 0) {
    SYLAR_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", "
                              << port << ") rt=" << result << " errno=" << errno
                              << " errstr=" << strerror(errno);
    return nullptr;
  }
  return rt;
}

IPv4Address::IPv4Address(const sockaddr_in &address) { m_addr = address; }

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_port = byteswapOnLittleEndian(port);
  m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}
//*======================================================
//*都使用sockaddr作为返回值应该是为了兼容
sockaddr *IPv4Address::getAddr() { return (sockaddr *)&m_addr; }

const sockaddr *IPv4Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv4Address::getAddrLen() const { return sizeof(m_addr); }
//*===================================================
std::ostream &IPv4Address::insert(std::ostream &os) const {
  uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
  os << ((addr >> 24) & 0xff) << "." << ((addr >> 16) & 0xff) << "."
     << ((addr >> 8) & 0xff) << "." << (addr & 0xff);
  os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
  return os;
}

//*计算广播地址
IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr |=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::ptr(new IPv4Address(baddr));
}

//*计算网络地址,标识一个子网
IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr &=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::ptr(new IPv4Address(baddr));
}

//*根据子网掩码的长度计算出子网掩码并存到s_addr
IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
  sockaddr_in subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin_family = AF_INET;
  subnet.sin_addr.s_addr =
      ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const {
  return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v) {
  m_addr.sin_port = byteswapOnLittleEndian(v);
}

IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port) {
  IPv6Address::ptr rt(new IPv6Address);
  rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
  if (result <= 0) {
    SYLAR_LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", "
                              << port << ") rt=" << result << " errno=" << errno
                              << " errstr=" << strerror(errno);
    return nullptr;
  }
  return rt;
}

IPv6Address::IPv6Address() {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6 &address) { m_addr = address; }

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
  m_addr.sin6_port = byteswapOnLittleEndian(port);
  memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

sockaddr *IPv6Address::getAddr() { return (sockaddr *)&m_addr; }

const sockaddr *IPv6Address::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t IPv6Address::getAddrLen() const { return sizeof(m_addr); }

std::ostream &IPv6Address::insert(std::ostream &os) const {
  os << "[";
  uint16_t *addr = (uint16_t *)m_addr.sin6_addr.s6_addr;
  bool used_zeros = false;
  for (size_t i = 0; i < 8; ++i) {
    if (addr[i] == 0 && !used_zeros) {
      continue;
    }
    if (i && addr[i - 1] == 0 && !used_zeros) {
      os << ":";
      used_zeros = true;
    }
    if (i) {
      os << ":";
    }
    os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
  }

  if (!used_zeros && addr[7] == 0) {
    os << "::";
  }

  os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
  return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] |=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0xff;
  }
  return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] &=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0x00;
  }
  return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
  sockaddr_in6 subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin6_family = AF_INET6;
  subnet.sin6_addr.s6_addr[prefix_len / 8] =
      ~CreateMask<uint8_t>(prefix_len % 8);

  for (uint32_t i = 0; i < prefix_len / 8; ++i) {
    subnet.sin6_addr.s6_addr[i] = 0xff;
  }
  return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const {
  return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v) {
  m_addr.sin6_port = byteswapOnLittleEndian(v);
}
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sun_family = AF_UNIX;
  m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string &path) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sun_family = AF_UNIX;
  m_length = path.size() + 1;

  if (!path.empty() && path[0] == '\0') {
    --m_length;
  }

  if (m_length > sizeof(m_addr.sun_path)) {
    throw std::logic_error("path too long");
  }
  memcpy(m_addr.sun_path, path.c_str(), m_length);
  m_length += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::setAddrLen(uint32_t v) { m_length = v; }

sockaddr *UnixAddress::getAddr() { return (sockaddr *)&m_addr; }

const sockaddr *UnixAddress::getAddr() const { return (sockaddr *)&m_addr; }

socklen_t UnixAddress::getAddrLen() const { return m_length; }

std::string UnixAddress::getPath() const {
  std::stringstream ss;
  if (m_length > offsetof(sockaddr_un, sun_path) &&
      m_addr.sun_path[0] == '\0') {
    ss << "\\0"
       << std::string(m_addr.sun_path + 1,
                      m_length - offsetof(sockaddr_un, sun_path) - 1);
  } else {
    ss << m_addr.sun_path;
  }
  return ss.str();
}

std::ostream &UnixAddress::insert(std::ostream &os) const {
  if (m_length > offsetof(sockaddr_un, sun_path) &&
      m_addr.sun_path[0] == '\0') {
    return os << "\\0"
              << std::string(m_addr.sun_path + 1,
                             m_length - offsetof(sockaddr_un, sun_path) - 1);
  }
  return os << m_addr.sun_path;
}

UnknownAddress::UnknownAddress(int family) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr &addr) { m_addr = addr; }

sockaddr *UnknownAddress::getAddr() { return (sockaddr *)&m_addr; }

const sockaddr *UnknownAddress::getAddr() const { return &m_addr; }

socklen_t UnknownAddress::getAddrLen() const { return sizeof(m_addr); }

std::ostream &UnknownAddress::insert(std::ostream &os) const {
  os << "[UnknownAddress family=" << m_addr.sa_family << "]";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Address &addr) {
  return addr.insert(os);
}
