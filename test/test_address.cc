#include "../include/address.h"
#include "../include/Log.h"
#include<map>
#if 0
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
template <class T> static T CreateMask(uint32_t bits) {
  //* 将1 左移 (sizeof(T)*8-bits)位再减一
  return (1 << (sizeof(T) * 8 - bits)) - 1;
}
#include <iostream>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

void test_getaddrinfo(const std::string &hostname) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;    // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
        SYLAR_LOG_INFO(g_logger) << "getaddrinfo error: " << gai_strerror(status);
        return;
    }

    SYLAR_LOG_INFO(g_logger) << "IP addresses for " << hostname << ":";

    for(p = res; p != NULL; p = p->ai_next) {
        void *addr;
        std::string ipver;

        // Get the pointer to the address itself
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // Convert the IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        SYLAR_LOG_INFO(g_logger) << "  " << ipver << ": " << ipstr;
    }

    freeaddrinfo(res); // Free the linked list
}
void test() {
    std::vector<Address::ptr> addrs;

    SYLAR_LOG_INFO(g_logger) << "begin";
    bool v = Address::lookUp(addrs, "localhost:80");
    bool v1 = Address::lookUp(addrs, "www.baidu.com", AF_INET);
    bool v2 = Address::lookUp(addrs, "www.sylar.top", AF_INET);
    SYLAR_LOG_INFO(g_logger) << "end";
    if(!v&&!v1&&!v2) {
        SYLAR_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = Address::LookupAny("localhost:4080");
    if(addr) {
        SYLAR_LOG_INFO(g_logger) << *addr;
    } else {
        SYLAR_LOG_ERROR(g_logger) << "error";
    }
}
void test_getInterfaceAddress(){
    std::multimap<std::string,std::pair<Address::ptr,uint32_t>> result;
    Address::GetInterfaceAddresses(result,AF_INET);
    for(auto &i:result){
        SYLAR_LOG_INFO(g_logger) << i.first << " " << i.second.first->toString() << " " << i.second.second  ;
    }
}
void test_iface() {
    std::multimap<std::string, std::pair<Address::ptr, uint32_t> > results;

    bool v = Address::GetInterfaceAddresses(results);
    if(!v) {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        SYLAR_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = sylar::IPAddress::Create("www.sylar.top");
    auto addr = IPAddress::Create("127.0.0.8");
    if(addr) {
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}
void test_createmask(){
    SYLAR_LOG_DEBUG(g_logger) << CreateMask<uint64_t>(8) ;
}
int main(int argc, char** argv) {
    //test_ipv4();
    //test_iface();
    //test_getaddrinfo("www.bilibili.com");
    //test_createmask();
    test_getInterfaceAddress();
    return 0;
}
#endif
