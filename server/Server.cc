#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "Server.h"
#include "../depdence/httpd.h"

Server::Server(bool &ret, int work_mode, int port) 
        :work_mode_(work_mode), port_(port) {
  ret = InitServer();
}

Server::~Server() {

}

void handle_func(int sig_no) {
  std::cout<<"server exit"<<std::endl; /* 这个 应该是server退出吧，为什么是subProcess退出 */
  int status;  
  exit(1);
}

bool Server::InitServer() {
  /* 设置服务器 */
  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr)); /* memset比bzero兼容性更好,bzero只能linux使用，要么就自己定义 */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 其实还是0, 表示接受所有ip */

  /* socket设置 */
  socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0); // sock_stream默认使用tcp协议
  fcntl(socket_fd_, F_SETFL, O_NONBLOCK); //我没有使用read和write，感觉可以省略
  setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &REUSE_OPEN, sizeof(REUSE_OPEN)); /* 地址复用 但是我已经设置了ANY,设置复用有什么用处？ */

  if (::bind(socket_fd_, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind failed, please check");
    return false; /* 错误码可以定义成enum */
  }

  if (::listen(socket_fd_, 20) == -1) {
    perror("listen failed, please check");
    return false;
  }
  
  std::cout<<"listening to port: "<<port_<<std::endl;
  signal(SIGINT, handle_func);
  signal(SIGQUIT, handle_func);
  return true;
}

void Server::Work(int accept_fd, sockaddr_in &client_addr) {
  std::cout<<"cur work accept_fd: "<<accept_fd<<"  client addr "<<inet_ntoa(client_addr.sin_addr) \
              <<":"<<client_addr.sin_port<<std::endl;
  switch (work_mode_) {
    case 0:
      NormalWork(accept_fd, client_addr);
      break;
    case 1:
      MultiThreadWork(accept_fd, client_addr);
      break;
    case 2:
      MultiProcessWork(accept_fd, client_addr);
    default:
      break;
  }
}

void Server::Process() {

  int socket_fd = socket_fd_; 
  int epoll_fd = ::epoll_create(EPOLL_MAX_EVENT);
  epoll_event m_epoll_event;
  m_epoll_event.data.fd = socket_fd;
  m_epoll_event.events = EPOLLIN;

  if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &m_epoll_event) == -1) {
    std::cout<<"[Process] epoll_ctl, error"<<std::endl;
    exit(1);
  }

  epoll_event events[EPOLL_MAX_EVENT];
  sockaddr_in client_addr;
  socklen_t client_addr_len;
  client_addr_len = sizeof(client_addr);
  char buf[BUFFER_MAX_LEN];
  
  time_t time_start = time(0); 
  do {
    int accept_fd;
    int check_number = ::epoll_wait(epoll_fd, events, EPOLL_MAX_EVENT, 1); /* 不限时 epoll_wait仍然会阻塞 */
    if (check_number == -1) {
      std::cout<<"[Process] epoll_wait error"<<std::endl;
      continue;
    }

    for (int i = 0; i < check_number; ++i) {
      if (events[i].data.fd == socket_fd) { /* 初次收到消息 */
        accept_fd = ::accept(socket_fd, (sockaddr *)&client_addr, &client_addr_len);
     
        if (accept_fd == -1) {
          ::sprintf(buf, "[pid %d] accept error", ::getpid());
          std::cout<< buf <<std::endl;
          continue;           
        }

        if (fcntl(accept_fd, F_SETFL, fcntl(accept_fd, F_GETFD, 0) | O_NONBLOCK) == -1) {
          continue;
        }

        Work(accept_fd, client_addr);
        close(accept_fd);
        std::cout<<"receive message from "<< inet_ntoa(client_addr.sin_addr) \
                 <<":"<< client_addr.sin_port <<std::endl;

      } else if (events[i].events & EPOLLERR) {
        std::cout<<"epoll error"<<std::endl;
        close(accept_fd);
      }
    }
  }while(difftime(time(0), time_start) < 10);
}

void Server::NormalWork(int accept_fd, sockaddr_in client_addr) {
  accept_request(accept_fd, &client_addr);
}

void Server::MultiProcessWork(int accept_fd, sockaddr_in client_addr) {
  /**
   * 多进程处理 epoll可以同时接收多个，但是依然是单个进程处理，通过fork创建子进程
   * 只让子进程处理
   */
  pid_t pid = ::fork(); /* 函数定义在unistd.h */
  if (pid == 0) {
    accept_request(accept_fd, &client_addr);
    exit(0);
  } else if(pid < 0) {
    std::cout<<"sub Process created failed"<<std::endl;
    // exit(1);
  } else {
    std::cout<<"[pid "<< pid <<"] deal with message from "<< inet_ntoa(client_addr.sin_addr) \
              <<":"<<client_addr.sin_port<<std::endl;
  }
  
}

void *StartAcceptReqeust(void *args) {
  std::pair<int, sockaddr_in> *temp = (std::pair<int, sockaddr_in> *)(args);
  accept_request(temp->first, &temp->second);
  pthread_exit(0);
}

void Server::MultiThreadWork(int accept_fd, sockaddr_in client_addr) {
  pthread_t tid;
  std::pair<int, sockaddr_in> args(accept_fd, client_addr);
  int pid;
  if (pid = pthread_create(&tid, NULL, StartAcceptReqeust, &args) != 0) {
    std::cout<<"sub thread created failed"<<std::endl;
  } else {
    std::cout<<"sub thread deal with message from "<< inet_ntoa(client_addr.sin_addr) \
             <<":"<<client_addr.sin_port<<std::endl;
  }
  pthread_join(tid, 0);

}
