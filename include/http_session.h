#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H
#include "http.h"
#include "socket_stream.h"

class HttpSession : public SocketStream {
public:
  using ptr = std::shared_ptr<HttpSession>;

  HttpSession(Socket::sptr sock, bool owner = true);

  HttpRequest::ptr recvRequest();

  int sendResponse(HttpResponse::ptr rsp);
};

#endif

