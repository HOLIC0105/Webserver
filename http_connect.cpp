#include "http_connect.hh"

void http_connect::Init(const int &sockfd, const sockaddr_in & addr){
  socketfd_ = sockfd;
  socketfd_addr_ = addr;

  int reuse = 1;
  setsockopt(socketfd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

  AddFd(epollfd_, socketfd_, true);

  user_count_ ++;
}

void http_connect::Close_Connect() {
  if(socketfd_ != -1) {
    RemoveFd(epollfd_, socketfd_);
    socketfd_ = -1;
    user_count_ --;
  }
}

bool http_connect::Read() {
  printf("READ ALL\n");
  return true;
  /*
    ...
    ...
  */
} 

bool http_connect::Write() {
  printf("WRITE ALL\n");
  return true;
  /*
    ...
    ...
  */
}

void http_connect::process() {

  printf("prase http request , generate a response\n");
   /*
    ...
    ...
  */

} // prase http request , generate a response


void SetNonBlocking(int fd) {
  int old_flag = fcntl(fd, F_GETFL);
  int new_flag = old_flag | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_flag);
}

void AddFd(int epollfd, int fd, bool one_shot) {
  epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLRDHUP;

  if(one_shot) {
    event.events |= EPOLLONESHOT;
  } //only one thread can solve, need use epoll_ctl to clean this

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

  SetNonBlocking(fd);
} //add fd to epoll

void RemoveFd(int epollfd, int fd) {
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);
} //remove fd from epoll

extern void ModifyFd(int epollfd, int fd, int ev) {
  epoll_event event;
  event.data.fd = fd;
  event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
} // Reset EPOLLONESHOT event