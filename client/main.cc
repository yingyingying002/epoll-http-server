#include <iostream>
#include <unistd.h>
#include <pthread.h>


/* 创建httpClient对象，发送http请求，得到httpResponse */
static const unsigned short PORT = 8000;
static const unsigned int THREAD_NUMBER = 10;
void* SendByCurl(void *) {
  ::system("curl -i \"http://localhost:8000/index.html\"");
}

void* SendByHand(void *) {
  
}

int main(int argc, char *argv[]) {
  int wait_length = 200;
  if (argc == 2) {
    wait_length = atoi(argv[1]);
  }
  
  pthread_t tid[THREAD_NUMBER];

  std::cout<<"client started..."<<std::endl;
  for (int i = 0; i < THREAD_NUMBER; ++i) {
    if ( pthread_create(&tid[i], NULL, SendByHand, NULL) == -1 ) {
      std::cout<<"sub thread created failed"<<std::endl;
    }
    usleep(wait_length * 1000); /* 默认停顿200ms */
  }
    
  return 0;
}

