#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include "fdctrl.hh"

#include <string>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

class http_connect{
  public:
    static constexpr int READ_BUFFER_SIZE_ = 2048;     //读缓冲区的大小
    static constexpr int WRITE_BUFFER_SIZE_ = 2048;    //写缓冲区的大小

    // HTTP请求方法，这里只支持GET
    enum Method {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        解析客户端请求时的状态
        CHECK_STATE_REQUESTLINE:正在分析请求行
        CHECK_STATE_HEADER:正在分析头部字段
        CHECK_STATE_CONTENT:正在解析请求体
    */
    enum ChecktState { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };

   
    static int epollfd_;  //epoll文件标志符
   
    static int user_count_;  //已链接个数

    http_connect(){};
    ~http_connect(){};

    //处理HTTP请求的入口函数
    void Process(); 

    //初始化新链接
    void Init(const int &sockfd, const sockaddr_in & addr); 
    //断开链接
    void CloseConnect();
    // 对内存映射区执行munmap操作
    void Unmap();

    //非阻塞读取client数据,直到无数据
    bool Read(); 
    bool Write(); // nonblock write all

    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户数据
        GET_REQUEST         :   表示获得了一个完成的客户请求
        BAD_REQUEST         :   表示客户请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求,获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    */
    enum HttpCode { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LineStatus { LINE_OK = 0, LINE_BAD, LINE_OPEN };

    LineStatus ParseLine();

  private:

    void Init();

    char * GetLine() {return readbuf_ + startline_;}

      
    //解析一行
    LineStatus ParseLine();  
    //解析请求行：获得请求方法，目标URL, HTTP版本号
    HttpCode ParseRequestLine(char *text);
    //解析请求头
    HttpCode ParseHeaders(char *text);
    //解析请求体
    HttpCode ParseContent(char *text); 
    /*
      当得到一个完整正确的HTTP请求时，分析目标文件的属性,
      如果目标文件存在、对所有用户可读，且不是目录，
      则使用mmap将其映射到内存地址m_file_address处.
    */
    HttpCode DoRequest(); 
    //解析HTTP请求; 
    HttpCode ProcessRead(); 
    // 根据服务器处理HTTP请求的结果，决定返回给客户端的内容
    bool ProcessWrite(HttpCode ret);

    int socketfd_;
    sockaddr_in socketfd_addr_;

    char readbuf_[READ_BUFFER_SIZE_];           //读缓冲区
    int readidx_;                               //读缓冲区中已经读入的client数据的endl()
    int checkidx_;                              //正在分析的字符在读缓冲区的位置   
    int startline_;                           //当前正在解析的行的起始位置

    char writebuf_[WRITE_BUFFER_SIZE_];         //写缓冲区
    int writeidx_;                              //写缓冲区中未发送的字节数
    struct stat filestat_;                      //目标文件的状态
    char *fileaddress_;                         //目标文件在内存中的起始位置

    int bytes_to_send_;               //将要发送的字节数
    int bytes_have_send_;             //已经发送的字节数


    ChecktState checkstate_;          //当前所属状态
    

    char * url_;                      //文件名
    Method method_;                   //请求方法
    char * version_;                  //协议版本 only HTTP1.1
    char * host_;                     //主机名
    bool linger_;                     //是否保持链接
    int contentlength_;               //请求消息的总长度
    std::string fileplace_;           //请求文件的路径

};
#endif