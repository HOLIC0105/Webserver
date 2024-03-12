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
  url_ = 0;
  method_ = GET;
  version_ = 0;
  bzero(readbuf_, READ_BUFFER_SIZE_);
  linger_ = 0;
}

void http_connect::CloseConnect() {

  if(socketfd_ != -1) {

    RemoveFd(epollfd_, socketfd_);
    socketfd_ = -1;
    user_count_ --;

  }

}

bool http_connect::Read() {

  //请求过多，主动断开链接
  if(readidx_ >= READ_BUFFER_SIZE_) 
    return false; 

  int bytes_read = 0;
  while(true) {

    bytes_read = recv(socketfd_, readbuf_ + readidx_, READ_BUFFER_SIZE_ - readidx_, 0);

    if(bytes_read == -1) {
      
      if(errno == EAGAIN || errno == EWOULDBLOCK) {//没有数据
        break;
      }

      return false;

    } else if(bytes_read == 0) {//client关闭连接
      
      return false;

    }

    readidx_ += bytes_read;

  }

  return true;

}

bool http_connect::Write() {
  printf("WRITE ALL\n");
  return true;
  /*
    ...
    ...
  */
}

http_connect:: HttpCode http_connect:: ParseRequestLine(char *text) {

  url_ = strpbrk(text, " \t");

  if(!url_) return BAD_REQUEST;

  *url_ ++ = '\0';

  char * method = text;
  if(strcasecmp(method, "GET") == 0) {

    method_ = GET;

  } else return BAD_REQUEST;

  version_ = strpbrk(url_, " \t");

  if(!version_) return BAD_REQUEST;

  *version_ ++ = '\0';
  
  if(strcasecmp(version_, "HTTP/1.1") != 0) return BAD_REQUEST;

  if(strncasecmp(url_, "http://", 7) == 0) {
    url_ += 7; // 忽略 http://
    url_ = strchr(url_, '/');
  }

  if(!url_ || url_[0] != '/') return BAD_REQUEST;

  checkstate_ = CHECK_STATE_HEADER; //检查状态转移到解析请求头

  return NO_REQUEST;

}
http_connect:: HttpCode http_connect:: ParseHeaders(char *text){

}
http_connect:: HttpCode http_connect:: ParseContent(char *text){

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

        ret = ParseHeaders(text);

        if(ret == BAD_REQUEST) {

          return BAD_REQUEST;

        } else if(ret == GET_REQUEST) {

          return DoRequest();

        }
        break;

      }
      case CHECK_STATE_CONTENT:
      {

        ret = ParseContent(text);

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

bool ProcessWrite(http_connect::HttpCode a){};

void http_connect::Process() {
  //解析HTTP请求
  HttpCode read_ret = ProcessRead();
  if(read_ret == NO_REQUEST) {

    ModifyFd(epollfd_, socketfd_, EPOLLIN);
    return;

  }
  //生成HTTP响应
  bool write_ret = ProcessWrite(read_ret);
  if(!write_ret) {

    CloseConnect();

  }

  ModifyFd(epollfd_, socketfd_, EPOLLOUT);

} 
