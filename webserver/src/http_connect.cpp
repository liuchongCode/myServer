#include "SubReactor.h"

// 定义HTTP响应的一些状态信息
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";

// 网站的根目录
const char* doc_root = "../resources";

// 设置文件描述符非阻塞
int setnonblocking(int fd){
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    int ret = fcntl(fd, F_SETFL, flag);
    // if (ret == -1) {
        // printf("fd = %d", fd);
        // perror("fcntl");
    // }
    return ret;
}

void setSocketNoLinger(int fd) 
{
    struct linger linger_;
    linger_.l_onoff = 1;                    // 在closesocket()调用， 但是还有数据没发送完毕的时候容许逗留
    linger_.l_linger = 30;                  // 逗留时间为30s
    setsockopt(fd, SOL_SOCKET, SO_LINGER,(const char *) &linger_, sizeof(linger_));
}

/*
    EPOLLIN
        关联文件可用于读操作

    EPOLLOUT
        关联文件可用于写操作

    EPOLLRDHUP (since Linux 2.6.17)
        流套接字对等关闭连接，或关闭写入连接的一半
        (当使用边缘触发监控时，这个标志对于编写简单的代码来检测对等关闭特别有用)

    EPOLLPRI
        在文件描述符上有一个异常条件
    
    EPOLLERR
        关联文件描述符发生错误条件
        当管道的读端已关闭时，也会报告该事件
        Epoll_wait(2)将始终报告此事件;没有必要在事件中设置它

    EPOLLHUP
        关联文件描述符上发生了挂起
        Epoll wait(2)将始终等待此事件;没有必要在事件中设置它
        请注意，当从通道(如管道或流套接字)读取数据时，此事件仅表明对等端关闭了通道的末端
        只有在通道中所有未完成的数据都被消耗掉之后，从通道的后续读取才会返回e(文件结束)。

    EPOLLET
        为关联的文件描述符设置边缘触发行为
        epoll的默认行为是“水平触发”
        关于边缘和水平触发事件分布架构的详细信息，请参见epoll(7)。

    EPOLLONESHOT (since Linux 2.6.2)
        设置关联文件描述符的一次性行为 
        这意味着在使用epoll_wait(2)取出事件后，相关的文件描述符在内部被禁用，epoll接口不会报告其他事件  
        用户必须调用epoll_ctl()通过EPOLL_CTL_MOD来使用新的事件掩码重新装填文件描述符
*/

void http_connect::init(int ep_fd, int sockfd, const sockaddr_in & addr) {
    m_ep_fd = ep_fd;
    m_sock_fd = sockfd;
    m_addr = addr;
    // 设置端口复用
    int reuse = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret == -1) {
        perror("setsockopt");
        exit(-1);
    }
    init();
}

void http_connect::init() {
    m_check_state = CHECK_STATE_REQUESTLINE; // 初始化状态为解析请求首行
    m_checked_index = 0; // 当前正在解析的字符位置
    m_start_line = 0; // 当前正在解析的行的起始位置
    m_read_offset = 0; // 读缓冲区偏移量，已读位置的下一个位置索引
    m_write_offset = 0;  // 写缓冲区偏移量，已写位置的下一个位置索引
    bytes_to_write = 0; // 要发送的字节数
    bytes_have_write = 0; // 已经发送的字节数
    m_method = GET;
    m_url = NULL;
    m_version = NULL;
    m_host = NULL;
    keep_alive = false;
    m_content_length = 0;
    bzero(m_real_file, FILENAME_MAX);
    bzero(m_write_buf, READ_BUFFER_SIZE);
    m_file_addr = NULL;
    bzero(m_read_buf, READ_BUFFER_SIZE);
    m_iv_count = 0;
}

// 一次性读取所有数据，非阻塞
bool http_connect::read(int & saveErrno) {
    if (m_read_offset >= READ_BUFFER_SIZE) {
        LOG << m_sock_fd << " : 读缓冲区已满\n";
        return false;
    }

    // 读取到的字节
    int bytes_read = 0;
    int num = 0;
    while (1) {
        bytes_read = recv(m_sock_fd, m_read_buf + m_read_offset, READ_BUFFER_SIZE - m_read_offset, MSG_DONTWAIT);
        LOG << "recving end\n";
        if (bytes_read == -1) {
            saveErrno = errno;
            if (errno == EINTR) // 由于接收到信号而中断，应该重新读
                continue;

            // 由于设置了非阻塞，当没有数据可读时，不会等待数据，而是直接返回，并设置错误码
            if (errno == EAGAIN || errno == EWOULDBLOCK) {  
                break;
            }
            perror("recv");
            return false;
        } else if (bytes_read == 0) {
            // 对方关闭连接
            return false;
        }
        // 成功读取则修改偏移量
        m_read_offset += bytes_read;
    }
    return true;
} 

// 解析一行, 判断依据\r\n
http_connect::LINE_STATUS http_connect::parse_line() {
    char tmp;
    for (; m_checked_index < m_read_offset; ++m_checked_index) {
        tmp = m_read_buf[m_checked_index];
        if (tmp == '\r') {
            if (m_checked_index + 1 == m_read_offset) { // 说明行读取出错
                return LINE_OPEN;
            } else if(m_read_buf[m_checked_index + 1] == '\n') {
                m_read_buf[m_checked_index++] = '\0'; // 找到\r\n,将两个位置都置为字符串结束符
                m_read_buf[m_checked_index++] = '\0';
                return LINE_OK;
            } else {
                return LINE_BAD;
            }
        } else if (tmp == '\n') {
            if ((m_checked_index > 1) && (m_read_buf[m_checked_index - 1] == '\r')) {
                // 前一次数据不完整时，刚好从\r后断开
                m_read_buf[m_checked_index - 1] = '\0';
                m_read_buf[m_checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_BAD;
} 

/*
    主状态机，解析请求
*/
// 解析HTTP请求
http_connect::HTTP_CODE http_connect::process_read(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char * text = 0;
    while (((m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) 
        || ((line_status = parse_line()) == LINE_OK)) {
            // 解析到了一行完整的数据，或者解析到了请求体，也是完整的数据

            // 获取一行数据
            text = get_line();

            // 重置行起始位置
            m_start_line = m_checked_index;

            switch(m_check_state) {
                case CHECK_STATE_REQUESTLINE: {
                    ret = parse_request_line(text);
                    if (ret == BAD_REQUEST) {
                        return BAD_REQUEST;
                    }
                    break;
                }
                case CHECK_STATE_HEADER: {
                    ret = parse_request_head(text);
                    if (ret == BAD_REQUEST) {
                        return BAD_REQUEST;
                    } else if (ret == GET_REQUEST) {
                        // 请求头解析完成且没有请求体（即http请求解析完成）
                        return do_request();
                    }
                    break;
                }
                case CHECK_STATE_CONTENT: {
                    ret = parse_request_body(text);
                    if (ret == GET_REQUEST) {
                        // 请求解析完毕
                        return do_request();
                    }

                    line_status = LINE_OPEN;
                    break;
                }    
            
                default: {
                    return INTERNAL_ERROR;
                }
            }
    }
    return NO_REQUEST;
} 

// 解析HTTP请求行,获取请求方法，URL，HTTP版本
http_connect::HTTP_CODE http_connect::parse_request_line(char * text){
    // GET /index.html HTTP/1.1
    m_url = strpbrk(text, " \t"); // 寻找" \t"中任意字符出现的位置
    if (! m_url) { 
        return BAD_REQUEST;
    }
    
    *m_url++ = '\0';

    // GET\0/index.html HTTP/1.1
    char * method = text;
    if (strcasecmp(method, "GET") == 0) { // 忽略大小写比较
        m_method = GET;
    } else {
        return BAD_REQUEST;
    }

    // /index.html\0HTTP/1.1
    m_version = strpbrk(m_url, " \t");
    if (!m_version) {
        return BAD_REQUEST;
    }
    *m_version++ = '\0';
    if ((strcasecmp(m_version, "HTTP/1.1") != 0) && (strcasecmp(m_version, "HTTP/1.0") != 0)) {
        return BAD_REQUEST;
    }

    // http://192.168......
    if (strncasecmp(m_url, "http://", 7) == 0) {
        m_url += 7;
        m_url = strchr(m_url, '/');   // 找到第一个/所在位置
    }

    if (!m_url || m_url[0] != '/') {
        return BAD_REQUEST;
    }

    m_check_state = CHECK_STATE_HEADER; // 请求行解析完后将主状态机状态改为解析请求头

    return NO_REQUEST;
    
} 

// 解析HTTTP请求头
http_connect::HTTP_CODE http_connect::parse_request_head(char * text){
    // 遇到空行表示头部解析完毕
    if (text[0] == '\0') {
        // 如果http请求有消息体，则还需要读取m_content_length字节的消息体
        // 状态机转移到CHECK_STATE_CONTENT状态
        if (m_content_length != 0) {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }

        // 否则说明已经得到了完整的http请求
        return GET_REQUEST;
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11; 
        text += strspn(text, " \t"); // 跳过空格和换行符
        if (strcasecmp(text, "keep-alive") == 0) {
            keep_alive = true;
        }
    } else if (strncasecmp(text, "Content-Length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t"); // 跳过空格和换行符
        m_content_length = atoi(text);
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t"); // 跳过空格和换行符
        m_host = text;
    } else {
        LOG << "oop! unknow header\n";
    }
    return NO_REQUEST;
} 

// 解析HTTP请求体
http_connect::HTTP_CODE http_connect::parse_request_body(char * text){
    if (m_read_offset >= (m_checked_index + m_content_length)) {
        text[m_content_length] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// 获取了完整的请求后，分析目标文件属性（根据url），对请求响应
// 如果目标文件存在、对所有用户可读，且不是目录，则使用mmap将其
// 映射到内存地址m_file_addr处，并告诉调用者获取文件成功
http_connect::HTTP_CODE http_connect::do_request(){
    // "/home/liuchong/coding/NPU/webSever/web/webserver/resources"
    strcpy(m_real_file, doc_root);
    int len = strlen(doc_root);
    strncpy(m_real_file + len, m_url, FILENAME_MAX - len - 1);
    LOG << m_real_file << "\n";
    // 获取文件相关状态信息， -1 失败 ， 0 成功
    if (stat(m_real_file, &m_file_stat) < 0) {
        LOG << "没有该文件\n"; 
        return NO_RESOURCE;
    }

    // 判断访问权限
    if (!(m_file_stat.st_mode & S_IROTH)) {
        LOG << "没有该文件访问权限\n";
        return FORBIDDEN_REQUEST;
    }

    // 判断是否是目录
    if (S_ISDIR(m_file_stat.st_mode)) {
        return BAD_REQUEST;
    }
    // 以只读方式打开文件
    int fd = open(m_real_file, O_RDONLY);
    // 创建内存映射
    m_file_addr = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

// 对内存映射区执行ummap操作
void http_connect::unmap() {
    if (m_file_addr) {
        munmap(m_file_addr, m_file_stat.st_size);
        m_file_addr = NULL;
    }
}

// 一次性写完所有数据，非阻塞
int http_connect::write(int & saveErrno) {
    int bytes = 0;
    if (bytes_to_write == 0) {
        // 将要发送的字节为0， 这次响应结束
        init();
        return 1;
    }

    while (1) {
        // 分散写， 将不连续的多块内存的数据一起写出去
        bytes = writev(m_sock_fd, m_iv, m_iv_count);
        if (bytes <= -1) {
            saveErrno = errno;
            // 如果TCP写缓冲没有空间,则等待下一轮EPOLLOUT时间
            // 虽然在此期间服务器无法立即接收同一客户的下一个请求，但可以保证连接的完整性
            if (errno == EAGAIN) {
                // LOG << m_sock_fd << " : TCP写缓冲区已满,等待下一轮EPOLLOUT事件\n";
                // gettimeofday(&t2, NULL);
                // LOG << "time use : " << (long)(t2.tv_sec - t1.tv_sec) * 1000 + (int)(t2.tv_usec - t1.tv_usec)/1000 << "\n";
                return 0;
            }
            LOG << "写出错, ep = " << m_ep_fd << "fd = " << m_sock_fd << '\n';
            perror("writev");
            unmap();
            return -1;
        }
        bytes_to_write -= bytes;
        bytes_have_write += bytes;

        if (bytes_have_write >= m_iv[0].iov_len) {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = (void *)(m_file_addr + (bytes_have_write - m_write_offset));
            m_iv[1].iov_len = bytes_to_write;
        } else {
            m_iv[0].iov_base = m_write_buf + bytes_have_write;
            m_iv[0].iov_len -=bytes;
        }
        if (bytes_to_write <= 0) {
            // 没有数据要发送了
            unmap();
            if (keep_alive) {
                init();
                return 1;
            }
            return -1;
        }
    }
} 

// 向写缓冲区写数据
bool http_connect::add_response(const char * format, ...) {
    if (m_write_offset >= WRITE_BUFFER_SIZE) {
        return false;
    }

    va_list arglist;
    va_start(arglist, format);
    int len = vsnprintf(m_write_buf + m_write_offset, WRITE_BUFFER_SIZE - m_write_offset - 1, format, arglist);
    if (len >= (WRITE_BUFFER_SIZE - m_write_offset - 1)) {
        return false;
    }
    m_write_offset += len;
    va_end(arglist);
    return true;

} 


bool http_connect::add_status_line(int status, const char * title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
} // 响应报文状态行

bool http_connect::add_headers(int content_len) {
    return add_content_length(content_len) &&
        add_content_type() &&
        add_keepalive() &&
        add_blank_line();
} // 响应报文头部

bool http_connect::add_content_length(int content_len) {
     return add_response( "Content-Length: %d\r\n", content_len );
} // 响应报文体长度

bool http_connect::add_keepalive()
{
    return add_response("Connection: %s\r\n", ( keep_alive == true ) ? "keep-alive" : "close");
}

bool http_connect::add_blank_line() {
    return add_response("%s", "\r\n");
} // 响应报文空行

bool http_connect::add_content_type() {
    return add_response("Content-Type:%s\r\n", "text/html");
}

bool http_connect::add_content(const char* content) {
    return add_response( "%s", content );
} //响应报文体

// 响应HTTP请求
bool http_connect::process_write(HTTP_CODE ret){
    switch (ret)
    {
        case FILE_REQUEST:
            add_status_line(200, ok_200_title);
            add_headers(m_file_stat.st_size);
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_offset;
            m_iv[1].iov_base = m_file_addr;
            m_iv[1].iov_len = m_file_stat.st_size;
            m_iv_count = 2;
            bytes_to_write = m_write_offset + m_file_stat.st_size;
            return true;
        case INTERNAL_ERROR:
            add_status_line( 500, error_500_title );
            add_headers( strlen( error_500_form ) );
            if ( ! add_content( error_500_form ) ) {
                return false;
            }
            break;
        case BAD_REQUEST:
            add_status_line( 400, error_400_title );
            add_headers( strlen( error_400_form ) );
            if (m_sock_fd == 10)
                printf("400\n");
            if ( ! add_content( error_400_form ) ) {
                return false;
            }
            break;
        case NO_RESOURCE:
            add_status_line( 404, error_404_title );
            if (m_sock_fd == 10)
                printf("404\n");
            add_headers( strlen( error_404_form ) );
            if ( ! add_content( error_404_form ) ) {
                return false;
            }
            break;
        case FORBIDDEN_REQUEST:
            add_status_line( 403, error_403_title );
            add_headers(strlen( error_403_form));
            if ( ! add_content( error_403_form ) ) {
                return false;
            }
            break;
        default:
            return false;
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_offset;
    m_iv_count = 1;
    bytes_to_write = m_write_offset;
    return true;
} 


// 由线程池中的工作线程调用，处理HTTP请求的入口函数
int http_connect::process()
{   
    if (m_read_offset <= 0) {
        LOG << m_sock_fd << " : 读缓冲区为空\n";
        return 1;
    }
    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST) {
        // 请求不完整
        return 1;
    }

    // 生成响应
    bool write_ret = process_write(read_ret);
    if (!write_ret) {
        LOG << m_sock_fd << " : 生成响应失败\n";
        return -1;
    }
    return 0;
}
