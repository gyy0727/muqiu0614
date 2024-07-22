#include "../include/http.h"
#include "../include/Log.h"

void test_request() {
    HttpRequest::ptr req(new HttpRequest);
    req->setHeader("host" , "www.baidu.com");
    req->setBody("hello sylar");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    HttpResponse::ptr rsp(new HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setStatus((HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}

