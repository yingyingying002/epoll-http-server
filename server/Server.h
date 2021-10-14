/* 防止重复定义 */
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define DEFAULT_PORT 8000
#define EPOLL_MAX_EVENT 500
#define BUFFER_MAX_LEN 256
const int REUSE_OPEN = 1;

class Server {
 public:
  Server(bool &ret, int work_mode = 0, int port = DEFAULT_PORT);
  ~Server();
  bool InitServer();
  void Work(int accept_fd, sockaddr_in &client_addr);
  void Process();
 private:
  void MultiProcessWork(int accept_fd, sockaddr_in client_addr);
  void MultiThreadWork(int accept_fd, sockaddr_in client_addr);
  void NormalWork(int accept_fd, sockaddr_in client_addr);
  sockaddr_in server_addr_;
  int socket_fd_;
  int work_mode_;
  unsigned short port_;
};

#endif