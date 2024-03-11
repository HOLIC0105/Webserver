#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include <cstdio>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

class http_connect{
  public:
    static int epollfd_;
    static int user_count_;

    http_connect(){};
    ~http_connect(){};
    void Process(); //solve client request
    void Init(const int &sockfd, const sockaddr_in & addr);
    void Close_Connect();
    bool Read(); //nonblock read all
    bool Write(); // nonblock write all
  private:
    int socketfd_;
    sockaddr_in socketfd_addr_;
};

extern void AddFd(int epollfd, int fd, bool one_shot);

extern void RemoveFd(int epollfd, int fd);

extern void ModifyFd(int epollfd, int fd, int ev);
#endif