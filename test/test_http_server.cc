#include "../include/Log.h"
#include "../include/http_server.h"

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

#define XX(...) #__VA_ARGS__

// IOManager::ptr worker;
void run() {
  // HttpServer::ptr server(
  //   new HttpServer(true, worker.get(),
  //   IOManager::getThis(),IOManager::getThis()));
  HttpServer::ptr server(new HttpServer(true));
  Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  auto sd = server->getServletDispatch();
  sd->addServlet("/*", [](HttpRequest::ptr req, HttpResponse::ptr rsp,
                         HttpSession::ptr session) {
    rsp->setBody(req->toString() + R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>)rawliteral");
    return 0;
  });

  sd->addGlobServlet("/sylarx/*", [](HttpRequest::ptr req,
                                     HttpResponse::ptr rsp,
                                     HttpSession::ptr session) {
    rsp->setBody(
        XX(<html><head><title> 404 Not Found</ title></ head><body><center>
                   <h1> 404 Not Found</ h1></ center><hr><center>
                       nginx /
                   1.16.0 <
               / center > </ body></ html> < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- > < !--a padding to disable MSIE and
           Chrome friendly error page-- >));
    return 0;
  });

  server->start();
}

int main(int argc, char **argv) {
  IOManager iom(6, "main");
  //worker.reset(new IOManager(16, "worker"));
  iom.schedule(run);
  return 0;
}
