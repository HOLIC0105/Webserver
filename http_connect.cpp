#include "http_connect.hh"

void AddFd(int epollfd, int fd, bool one_shot) {
  epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLRDHUP;

  if(one_shot) {
    event.events |= EPOLLONESHOT;
  } //only one thread can solve, need use epoll_ctl to clean this

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
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