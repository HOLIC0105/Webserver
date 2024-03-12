#include "fdctrl.hh"

void SetNonBlocking(const int & fd) {

  int old_flag = fcntl(fd, F_GETFL);
  int new_flag = old_flag | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_flag);

}

//向指定epoll中加入新的fd
void AddFd(const int & epollfd, const int & fd, const bool & one_shot) {

  epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLRDHUP;

  if(one_shot) {

    event.events |= EPOLLONESHOT; //防止同一个链接被不同的线程处理

  } 

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

  SetNonBlocking(fd);

} 

//将fd从指定epoll删除
void RemoveFd(const int & epollfd, const int & fd) {

  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);

} 

// 修改文件描述符，重置EPOLLONESHOT事件，下一次可读时，EPOLLIN事件能被触发
void ModifyFd(const int & epollfd, const int & fd, const int & ev) {

  epoll_event event;
  event.data.fd = fd;
  event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
  
} 