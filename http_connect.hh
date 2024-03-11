#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

class http_connect{
  public:
    static int epollfd_;
    static int user_count_;

    http_connect();
    ~http_connect();
    void process(); //solve client request
  private:
    int socketfd_;
    sockaddr_in socketfd_addr_;
};

extern void AddFd(int epollfd, int fd, bool one_shot);

extern void RemoveFd(int epollfd, int fd);

extern void ModifyFd(int epollfd, int fd, int ev);
#endif