#include "http_connect.hh"

int http_connect :: user_count_ = 0;
int http_connect :: epollfd_ = -1;

void http_connect::Init(const int &sockfd, const sockaddr_in & addr){

  Init();

  socketfd_ = sockfd;
  socketfd_addr_ = addr;

  int reuse = 1;
  setsockopt(socketfd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

  AddFd(epollfd_, socketfd_, true);

  user_count_ ++;
}

void http_connect::Init() {
  checkstate_ = CHECK_STATE_REQUESTLINE;
  checkidx_ = 0;
  startline_ = 0;
  readidx_ = 0;
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

http_connect:: HttpCode http_connect:: ProcessRead() {

  LineStatus line_status = LINE_OK;
  HttpCode ret = NO_REQUEST;

  char *text = 0;

  while((checkstate_ == CHECK_STATE_CONTENT && line_status == LINE_OK) //解析到了请求体
      ||(line_status = ParseLine()) == LINE_OK) { //解析到了一行完整的数据

      text = GetLine();

      startline_ = checkidx_;

      printf("get one http line : %s\n", text);

      switch (checkstate_)
      {
        case CHECK_STATE_REQUESTLINE:
        {
          ret = ParseRequestLine(text);
          if(ret == BAD_REQUEST) {
            return BAD_REQUEST;
          }
          break;
        }
        case CHECK_STATE_HEADER:
        {
          ret = Parseheaders(text);
          if(ret == BAD_REQUEST) {
            return BAD_REQUEST;
          } else if(ret == GET_REQUEST) {
            return DoRequest();
          }
          break;
        }
        case CHECK_STATE_CONTENT:
        {
          ret = Parsecontent(text);
          if(ret == BAD_REQUEST) {
             return BAD_REQUEST;
          } else if(ret == GET_REQUEST) {
            return DoRequest();
          } else line_status = LINE_OPEN;
          break;
        }
        default:
        {
          return INTERNAL_ERROR;
        }
      }
  }
  return NO_REQUEST;
}
http_connect:: HttpCode http_connect:: ParseRequestLine(char *text) {

}
http_connect:: HttpCode http_connect:: Parseheaders(char *text){

}
http_connect:: HttpCode http_connect:: Parsecontent(char *text){

}
http_connect:: HttpCode http_connect:: DoRequest(){

}

http_connect:: LineStatus http_connect:: ParseLine(){
  char temp;
  for(; checkidx_ < readidx_; ++ checkidx_) {
    temp = readbuf_[checkidx_];
    if(temp == '\r') {
      if(checkidx_ + 1 == readidx_) {
        return LINE_OPEN;
      } else if(readbuf_[checkidx_ + 1] == '\n') {
        readbuf_[checkidx_ ++] = '\0';
        readbuf_[checkidx_ ++] = '\0';
        return LINE_OK;
      } else return LINE_BAD;
    } else if(temp == '\n') {
      if(checkidx_ > 1 && readbuf_[checkidx_ - 1] == '\r') {
        readbuf_[checkidx_ - 1] = '\0';
        readbuf_[checkidx_ ++] = '\0';
        return LINE_OK;
      }
      return LINE_BAD;
    }
  }
  return LINE_OPEN;
}

void http_connect::Process() {
  HttpCode read_ret = ProcessRead();
  if(read_ret == NO_REQUEST) {
    ModifyFd(epollfd_, socketfd_, EPOLLIN);
    return;
  }
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