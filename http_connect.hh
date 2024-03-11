#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include <cstdio>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

class http_connect{
  public:
    static constexpr int READ_BUFFER_SIZE_ = 2048;
    static constexpr int WRITE_BUFFER_SIZE_ = 2048;

    static int epollfd_;  //epoll fd
    static int user_count_; //已链接个数

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
    char readbuf_[READ_BUFFER_SIZE_];
    int readidx_; //读缓冲区中已经读入的client数据的最后一个字节的下标
    char writebuf_[WRITE_BUFFER_SIZE_];

};

extern void AddFd(const int & epollfd, const int & fd, const bool & one_shot);

extern void RemoveFd(const int & epollfd, const int & fd);

extern void ModifyFd(const int & epollfd, const int & fd, const int & ev);
#endif