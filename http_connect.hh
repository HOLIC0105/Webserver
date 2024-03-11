#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include <sys/epoll.h>
#include <unistd.h>

class http_connect{
  public:
    http_connect();
    ~http_connect();
    void process(); //solve client request
};

extern void AddFd(int epollfd, int fd, bool one_shot);

extern void RemoveFd(int epollfd, int fd);

extern void ModifyFd(int epollfd, int fd, int ev);

#endif