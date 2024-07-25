#if 0
#include "../include/IoManager.h"
#include "../include/Log.h"
#include "../include/address.h"
#include "../include/bytearray.h"
#include "../include/tcp_server.h"

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

class EchoServer : public TcpServer {
public:
  EchoServer(int type) : m_type(type) {}
  void handleClient(Socket::sptr client) override {

    SYLAR_LOG_INFO(g_logger) << "handleClient " << *client;
        //*缓冲区指针
    ByteArray::ptr ba(new ByteArray);
        
    while (true) {
      ba->clear();
      std::vector<iovec> iovs;
            //*其实就是把缓冲区指定大小的空间取出来
      ba->getWriteBuffers(iovs, 1024);
            //*相当于把数据写入bytearray
      int rt = client->recv(&iovs[0], iovs.size());
      if (rt == 0) {
        SYLAR_LOG_INFO(g_logger) << "client close: " << *client;
        break;
      } else if (rt < 0) {
        SYLAR_LOG_INFO(g_logger)
            << "client error rt=" << rt << " errno=" << errno
            << " errstr=" << strerror(errno);
        break;
      }
            //*这里主要是为了设置m_size
      ba->setPosition(ba->getPosition() + rt);
            //*这里是为了tostring方法
      ba->setPosition(0);
      // SYLAR_LOG_INFO(g_logger) << "recv rt=" << rt << " data=" <<
      // std::string((char*)iovs[0].iov_base, rt);
      if (m_type == 1) {             // text
        std::cout << ba->toString(); // << std::endl;
      } else {
        std::cout << ba->toHexString(); // << std::endl;
      }
      std::cout.flush(); 
    }
  }

private:
  int m_type = 0;
};

int type = 1;

void run() {
  SYLAR_LOG_INFO(g_logger) << "server type=" << type;
  EchoServer::ptr es(new EchoServer(type));
  auto addr = Address::LookupAny("0.0.0.0:8020");
  while (!es->bind(addr)) {
    sleep(2);
  }
  es->start();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    SYLAR_LOG_INFO(g_logger)
        << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
    return 0;
  }

  if (!strcmp(argv[1], "-b")) {
    type = 2;
  }

  IOManager iom(2);
  iom.schedule(run);
  return 0;
}
#endif
