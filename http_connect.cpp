#include "http_connect.hh"

int http_connect :: user_count_ = 0;
int http_connect :: epollfd_ = -1;

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
    printf("CLOSE CONNEC\n");
    RemoveFd(epollfd_, socketfd_);
    socketfd_ = -1;
    user_count_ --;
  }
}

bool http_connect::Read() {
  if(readidx_ >= READ_BUFFER_SIZE_) return false;
  int bytes_read = 0;
  while(true) {
    bytes_read = recv(socketfd_, readbuf_ + readidx_, READ_BUFFER_SIZE_ - readidx_, 0);
    if(bytes_read == -1) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        break; //没有数据
      }
      return false;
    } else if(bytes_read == 0) {
      //关闭连接
      return false;
    }
    readidx_ += bytes_read;
  }
  printf("client date : \n%s\n", readbuf_);
  return true;
}  //循环读取client数据,直到无数据

bool http_connect::Write() {
  printf("WRITE ALL\n");
  return true;
  /*
    ...
    ...
  */
}

void http_connect::Process() {
  ProcessRead();
} // prase http request , generate a response


void SetNonBlocking(const int & fd) {
  int old_flag = fcntl(fd, F_GETFL);
  int new_flag = old_flag | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_flag);
}

void AddFd(const int & epollfd, const int & fd, const bool & one_shot) {
  epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLRDHUP;

  if(one_shot) {
    event.events |= EPOLLONESHOT;
  } //only one thread can solve, need use epoll_ctl to clean this

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

  SetNonBlocking(fd);
} //向指定epoll中加入新的fd

void RemoveFd(const int & epollfd, const int & fd) {
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);
} //将fd从指定epoll删除

void ModifyFd(const int & epollfd, const int & fd, const int & ev) {
  epoll_event event;
  event.data.fd = fd;
  event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
} // 更改fd在epoll中的状态，Reset EPOLLONESHOT event