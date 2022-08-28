#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <cstring>
#include <errno.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>
#include <assert.h>
#include <pthread.h>
#include <mutex>
#include <memory>
#include <iostream>

#define TIMESLOT 5           // 每5秒产生一次alarm信号
#define MAX_FD 65535         // 最大文件描述符个数
#define MAX_EVENT_NUM 1000000  // 一次监听的最大的事件数量

// class util_timer;
// class sort_time_list; // 前向声明
// class SubReactor;
// class Timer;
struct client;

class http_connect
{
public:

     // HTTP请求方法，这里只支持GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE:当前正在分析请求行
        CHECK_STATE_HEADER:当前正在分析头部字段
        CHECK_STATE_CONTENT:当前正在解析请求体
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };

    /* 
        从状态机的三种可能状态，即行的读取状态，分别表示
        1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    */ 
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
    
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
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    
    int m_ep_fd; // 所有socket上的事件都被注册到同一个epoll对象中
    static const int READ_BUFFER_SIZE = 4096; // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 4096; // 写缓冲区大小

    http_connect() : m_sock_fd(-1) {
    }

    ~http_connect() {
    }

    int process();  // 处理任务
    void init(int ep_fd, int sockfd, const sockaddr_in & addr);  // 初始化新接收的连接
    // 关闭该连接
    void close_conn() {
        if (m_sock_fd != -1) {
            close(m_sock_fd);
            // LOG << "关闭fd = " << m_sock_fd << "\n";
            m_sock_fd = -1;
        }
    }
    bool read(int & saveErrno); // 一次性读取所有数据，非阻塞
    int write(int & saveErrno); // 一次性写完所有数据，非阻塞
    
    int getsockfd() {
        return this->m_sock_fd;
    }

    void setsockfd(int fd) {
        this->m_sock_fd = fd;
    }

    int writeBytes() {
        return bytes_to_write;
    }

    bool isKeepAlive() {
        return keep_alive;
    }

public:
    std::shared_ptr<client> timer;
private:  
    pthread_mutex_t m_mutex;  // 访问互斥信号量 

    int m_sock_fd; // 该http连接使用的socket
    sockaddr_in m_addr; // 通信的socket地址
    char m_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE]; // 写缓冲区
    int m_read_offset; // 读缓冲区偏移量，已读位置的下一个位置
    int m_write_offset; // 写缓冲区偏移量
    int bytes_to_write; // 要写的字节数
    int bytes_have_write; // 已经写的字节数

    METHOD m_method;   // 请求方法
    char * m_url;      // url
    char * m_version;  // http版本
    char * m_host;     // 主机名
    bool keep_alive;   // 是否保持连接

    int m_checked_index; // 当前正在分析的字符在缓冲区中的位置
    int m_start_line;    // 当前正在解析的行的起始位置
    int m_content_length; // 请求体长度
    char m_real_file[FILENAME_MAX]; // 文件名
    struct stat m_file_stat; // 文件状态
    struct iovec m_iv[2]; // 我们将采用writev来执行写操作
    int m_iv_count;
    char * m_file_addr; // 文件内存映射区地址

    CHECK_STATE m_check_state; // 主状态机当前所处状态

    void init(); // 初始化连接其余数据

    HTTP_CODE process_read(); // 解析HTTP请求
    bool process_write(HTTP_CODE ret); // 响应HTTP请求
    HTTP_CODE parse_request_line(char * text); // 解析HTTP请求行
    HTTP_CODE parse_request_head(char * text); // 解析HTTTP请求头
    HTTP_CODE parse_request_body(char * text); // 解析HTTP请求体
    LINE_STATUS parse_line(); // 解析一行
    inline char * get_line() { return m_read_buf + m_start_line; }
    HTTP_CODE do_request(); // 对解析完成的请求做出相应处理
    void unmap();
    bool add_response( const char* format, ... ); // 向写缓冲区写数据
    bool add_status_line(int status, const char * title); // 响应报文状态行
    bool add_headers(int content_len); // 响应报文头部
    bool add_content_length(int content_len); // 响应报文体长度
    bool add_blank_line(); // 响应报文空行
    bool add_content(const char *); //响应报文体
    bool add_content_type();
    bool add_keepalive();
};

// 信号操作
extern void sighandler(int sig);

// 添加信号捕捉
extern void addsig(int sig, void(handler)(int));

extern void timer_handler();

// extern void call_back(client * user_data);

extern int setnonblocking( int fd );

extern void setSocketNoLinger(int fd);

#endif
