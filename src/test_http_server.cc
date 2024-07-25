#include "../include/Log.h"
#include "../include/http_server.h"

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

#define XX(...) #__VA_ARGS__

IOManager::ptr worker;
void run() {
  // http::HttpServer::ptr server(new sylar::http::HttpServer(true,
  // worker.get(), sylar::IOManager::GetThis()));
  HttpServer::ptr server(new HttpServer(true));
  Address::ptr addr = Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  server->start();
}

int main(int argc, char **argv) {
  IOManager iom(1, "main");
  worker.reset(new IOManager(3, "worker"));
  iom.schedule(run);
  return 0;
}
