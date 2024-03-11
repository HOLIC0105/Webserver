#include "threadpool.hh"
#include "locker.hh"
#include "http_connect.hh"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>

#define MAX_FD 65535
#define MAX_EVENT_NUM 65536

void AddSig(int sig, void (*handle)(int)) {
  struct sigaction sa;
  sa.sa_handler = handle;
  sa.sa_flags = 0;
  sigfillset(&sa.sa_mask);
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

  return 0;
}