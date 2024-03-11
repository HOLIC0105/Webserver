#include "threadpool.h"
#include "locker.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <signal.h>

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

  return 0;
}