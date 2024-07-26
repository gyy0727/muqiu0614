#include "../include/Log.h"
#include "../include/http_parser.h"
#include "../include/http_session.h"
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
HttpSession::HttpSession(Socket::sptr sock, bool owner)
    : SocketStream(sock, owner) {}

HttpRequest::ptr HttpSession::recvRequest() {
  HttpRequestParser::ptr parser(new HttpRequestParser);
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size],
                               [](char *ptr) { delete[] ptr; });
  char *data = buffer.get();
  //*指向当前缓冲区可写位置的起点
  int offset = 0;

  do {

    //*先读取满一整个缓冲区
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      close();
      SYLAR_LOG_INFO(g_logger) << "读取请求失败--read" << " error = " <<len ;
      return nullptr;
    }
    //*要解析的数据长度
    len += offset;
    //*解析数据,返回已经解析的数据的长度
    //*// execute会将data向前移动nparse个字节，nparse为已经成功解析的字节数
    size_t nparse = parser->execute(data, len);
    if (parser->hasError()) {

      close();
      SYLAR_LOG_INFO(g_logger) << "解析过程存在错误--hasError";
      return nullptr;
    }
    //*指向要数据的末尾
    offset = len - nparse;
    //*偏移量等于缓冲区大小,证明数据过大,还没isFinished()就已经满了,可能是恶意攻击
    if (offset == (int)buff_size) {
      close();
      SYLAR_LOG_INFO(g_logger) << "缓冲区已满--offset==buffer_size";
      return nullptr;
    }
    if (parser->isFinished()) {
      break;
    }
  } while (1);
  //*这里是在处理之前解析的http请求不完整的情况,所以要自己把body set进去
  int64_t length = parser->getContentLength();
  if (length > 0) {
    std::string body;
    body.resize(length);
    int len = 0;
    if (length >= offset) {
      memcpy(&body[0], data, offset);
      len = offset;
    } else {
      memcpy(&body[0], data, length);
      len = length;
    }
    length -= offset;
    //*还有没取出来的数据,主要处理length>=offset的情况
    //*大概就是之前解析的数据不是一个完整的http请求,所以要继续读取数据,直到等于content-length
    if (length > 0) {
      if (readFixSize(&body[len], length) <= 0) {
        close();
        SYLAR_LOG_INFO(g_logger) << "无法获得完整的请求--length>0";
        return nullptr;
      }
    }
    parser->getData()->setBody(body);
  }
  parser->getData()->init();
  return parser->getData();
}
int HttpSession::sendResponse(HttpResponse::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}
