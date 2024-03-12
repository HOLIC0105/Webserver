#include "http_connect.hh"

// 网站的根目录
const char* doc_root = "/home/nowcoder/webserver/resources";

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

void http_connect::Unmap() {
  if(fileaddress_) {

    munmap(fileaddress_, filestat_.st_size);
    fileaddress_ = NULL;

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

  int temp = 0;

  if(bytes_to_send_ == 0) {
    ModifyFd(epollfd_, socketfd_, EPOLLOUT);
    Init();
    return true;
  } // ???????

  while(true) {

  }

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

  if(text[0] == '\0') { //空行表示HTTP头部解析完毕

    if(contentlength_ != 0) {

      checkstate_ = CHECK_STATE_CONTENT;

      return NO_REQUEST;

    }

    return GET_REQUEST;

  } else if(strncasecmp(text, "Host:", 5) == 0) {

    text += 5;
    text += strspn(text, " \t");

    host_ = text;

  } else if(strncasecmp(text, "Connection:", 11) == 0) {

    text += 11;
    text += strspn(text, " \t");
    linger_ = strcasecmp(text, "keep-alive") == 0 ? true : false;

  } else if(strncasecmp(text, "Content-Length:", 15) == 0) {

    text += 15;
    text += strspn(text, " \t");

    contentlength_ = atoi(text);

  } else { 
    //unknow hreader
  }
  
  return NO_REQUEST;

}
//判断是否完整读入
http_connect:: HttpCode http_connect:: ParseContent(char *text){

  if(readidx_ >= (contentlength_ + checkidx_)) {

    text[contentlength_] = '\0';
    return GET_REQUEST;

  }  

  return NO_REQUEST;

}

http_connect:: HttpCode http_connect:: DoRequest(){
  
  fileplace_ = doc_root;

  fileplace_ += url_;

  if(stat(fileplace_.c_str(), &filestat_) == -1) return NO_RESOURCE;   //请求文件是否存在

  if((filestat_.st_mode & S_IROTH) == 0) return FORBIDDEN_REQUEST;     //请求文件是否有读权限

  if(S_ISDIR(filestat_.st_mode)) return BAD_REQUEST;                   //请求文件是否是目录

  int fd = open(fileplace_.c_str(), O_RDONLY);

  fileaddress_ = reinterpret_cast<char *> (mmap(NULL, filestat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

  close(fd);
  return FILE_REQUEST;
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

bool http_connect::AddResponse(const char * format, ...) {
  //写入数据太多,断开链接
  if(writeidx_ >= WRITE_BUFFER_SIZE_) return false; 

  va_list arg_list;
  va_start(arg_list, format);

  /*
    若生成字符串的长度大于size，则将字符串的前size个字符复制到str，
    同时将原串的长度返回（不包含终止符），这里原串可能大于size，
  */
  int len = vsnprintf(writebuf_ + writeidx_, WRITE_BUFFER_SIZE_ - writeidx_ - 1, format, arg_list); 

  if(len >= WRITE_BUFFER_SIZE_ - writeidx_ - 1) {
    return false;
  }

  writeidx_ += len;
  va_end(arg_list);

  return true;

}

bool http_connect:: AddStatusLine(const int & status, const char* title){
  return AddResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool http_connect:: AddContentLength(const int & content_length){
   return AddResponse("Content-Length: %d\r\n", content_length);
}

bool http_connect:: AddContentType(){
  return AddResponse("Content-Type:%s\r\n", "text/html");
}

bool http_connect:: AddLinger(){
    return AddResponse("Connection: %s\r\n", (linger_ == true ) ? "keep-alive" : "close");
}

bool http_connect:: AddBlankLine(){
  return AddResponse( "%s", "\r\n" );
}

bool http_connect:: AddHeaders(const int & content_length ){
    AddContentLength(content_length);
    AddContentType();
    AddLinger();
    AddBlankLine();
}

bool http_connect:: AddContent(const char* content){
  return AddResponse("%s", content);
}

bool http_connect:: ProcessWrite(const HttpCode & ret){

  //  NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

  switch(ret) {
    case NO_REQUEST:
    { 
      break;
    }
    case GET_REQUEST:
    {

      break;
    }
    case BAD_REQUEST:
    {

      break;
    }
    case NO_RESOURCE:
    {
      
      break;
    }
    case FORBIDDEN_REQUEST:
    {

      break;
    }
    case FILE_REQUEST:
    {

      break;
    }
    case INTERNAL_ERROR:
    {

      break;
    }
    case CLOSED_CONNECTION:
    {

      break;  
    }
  }
};

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
