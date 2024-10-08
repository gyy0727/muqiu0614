#include"../include/IoManager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#if 0
Logger::ptr g_logger2=SYLAR_LOG_NAME("IOManager");
int sock=0;
void test_fiber() {
    SYLAR_LOG_INFO(g_logger2) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        SYLAR_LOG_INFO(g_logger2) << "add event errno=" << errno << " " << strerror(errno);
        IOManager::getThis()->addEvent(sock, IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger2) << "read callback";
        });
        IOManager::getThis()->addEvent(sock, IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger2) << "write callback";
            //close(sock);
            IOManager::getThis()->cancelEvent(sock, IOManager::READ);
            close(sock);
        });
    } else {
        SYLAR_LOG_INFO(g_logger2) << "else " << errno << " " << strerror(errno);
    }

}
void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    IOManager iom(2);
    iom.schedule(&test_fiber);
    sleep(10);

}

Timer::ptr s_timer;
void test_timer() {
    IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        SYLAR_LOG_INFO(g_logger2) << "hello timer i=" << i;
        if(++i == 3) {
            //s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, false);
}

int main(int argc, char** argv) {
    //test1();
    test_timer();
    return 0;
}
#endif
