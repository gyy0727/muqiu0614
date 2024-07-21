/*
 * @Author: Gyy0727 3155833132@qq.com
 * @Date: 2024-03-02 12:07:17
 * @LastEditors: Gyy0727 3155833132@qq.com
 * @LastEditTime: 2024-03-10 21:51:37
 * @FilePath: /sylar/src/test.c192.168.1.100192.168.1.1000
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    std::cout << "main" << std::endl;
  int sockfd;
  struct sockaddr_in server_addr;
  const char *server_ip = "183.2.172.42"; // 远程服务器的 IP 地址
  int server_port = 80;                // 远程服务器的端口号
  const char *message = "GET / HTTP/1.0\r\n\r\n";

  // 创建套接字
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    perror("inet_pton");
    close(sockfd);
    return 1;
  }

  // 连接服务器
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect");
    close(sockfd);
    return 1;
  }

  // 发送数据
  ssize_t bytes_sent = send(sockfd, message, strlen(message), 0);
  if (bytes_sent < 0) {
    perror("send");
  } else {
    std::cout << "Sent " << bytes_sent << " bytes to server: " << message
              << std::endl;
  }

  // 接收服务器响应（可选）
  char buffer[10240];
  ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received < 0) {
    perror("recv");
  } else {
    buffer[bytes_received] = '\0';
    std::cout << "Received from server: " << buffer << std::endl;
  }

  // 关闭套接字
  close(sockfd);
  return 0;
}
