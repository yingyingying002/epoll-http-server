#include <iostream>
#include <unistd.h>
#include "Server.h"

int main(int argc, char *argv[]) {
  unsigned short port = DEFAULT_PORT;
  if (argc > 1) {
    if (argc > 2) {
      std::cout<<"error command, we need at most 2 arguments"<<std::endl;
      exit(1);
    }
    port = atoi(argv[1]);
  }

  bool ret;
  Server m_server(ret, 0);
  if ( ret == false) {
    std::cout<<"server init failed"<<std::endl;
    exit(1);
  }
  /* 服务器启动 */
  m_server.Process();

  return 0;
}