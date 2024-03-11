#include "threadpool.hh"
#include "locker.hh"
#include "http_connect.hh"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>

#define MAX_FD 65535
#define MAX_EVENT_NUM 65536

void AddSig(int sig, void (*handle)(int)) {
  struct sigaction sa;
  sa.sa_handler = handle;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(sig, &sa, NULL);
}

int main(int argc, char* argv[]) {

  if(argc <= 1) {
    printf("need port num\n");
    exit(-1);
  }

  int port = atoi(argv[1]); 

  AddSig(SIGPIPE, SIG_IGN); 

  Threadpool<http_connect> * pool = NULL;
  try {
    pool = new Threadpool<http_connect>;
  } catch(...) {
    exit(-1);
  }

  http_connect * users = new http_connect[MAX_FD];

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd == -1) {
    perror("socket");
    exit(-1);
  }

  int reuseopt = 1;

  setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &reuseopt, sizeof(reuseopt));

  sockaddr_in listenfd_addr;
  listenfd_addr.sin_family = AF_INET;
  listenfd_addr.sin_addr.s_addr = INADDR_ANY;
  listenfd_addr.sin_port = htons(port);

  int ret = bind(listenfd, reinterpret_cast<sockaddr*>(&listenfd_addr), sizeof(listenfd_addr));
  if(ret == -1) {
    perror("bind");
    exit(-1);
  }

  listen(listenfd, 128);

  epoll_event events[MAX_EVENT_NUM];
  int epollfd = epoll_create1(0);
  http_connect::epollfd_ = epollfd;
  
  AddFd(epollfd, listenfd, false);
  
  while(true) {
    int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
    if(num == -1) {
      if(errno == EINTR) continue;
      perror("epoll_wait");
      exit(-1);
    }

    for(int i = 0; i < num; i ++) {

      int sockfd = events[i].data.fd;
      
      if(sockfd == listenfd) {
        sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&clientaddr), &clientaddr_len);
        if(clientfd == -1) {
          perror("accept");
          exit(-1);
        }
        char clientip[16];
        inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientip, sizeof(clientip));
        int clientport = ntohs(clientaddr.sin_port);
        printf("connect new\n");
        printf("client ip : %s , client port : %d\n", clientip, clientport);
        if(http_connect::user_count_ >= MAX_FD) {
          /*
            ...
            ...
            ...


          */
          close(clientfd);
          continue;
        }

        users[clientfd].Init(clientfd, clientaddr);

      } else if(events[i].events & (EPOLLRDHUP)){ 
        //Exception Disconnect
        users[sockfd].Close_Connect();
      } else if(events[i].events & EPOLLIN) {
        if(users[sockfd].Read()) {
          if(pool->Append(users + sockfd)) {
            //成功加入任务队列
          } else {
            //加入任务队列失败
          }
        } else {
          users[sockfd].Close_Connect(); // ???
        }
        ModifyFd(epollfd, sockfd, EPOLLIN);// Reset EPOLLONESHOT event
      } else if(events[i].events & EPOLLOUT) {
        if(!users[sockfd].Write()) {
          users[sockfd].Close_Connect(); // ???
        }
         ModifyFd(epollfd, sockfd, EPOLLOUT);// Reset EPOLLONESHOT event
      }
    }
  }

  close(epollfd);
  close(listenfd);

  delete [] users;
  delete pool;

  return 0;
}