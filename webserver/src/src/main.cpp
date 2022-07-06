#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <libgen.h>
#include <signal.h>
#include <assert.h>
#include "MainReactor.h"

// 心跳函数
// void timer_handler() {
//     // printf("执行心跳函数\n");
//     timer_list.tick();
//     alarm(TIMESLOT);
// }

// // 信号操作
// void sighandler(int sig) {
//     int save_errno = errno;
//     int msg = sig;
//     // send(pipefd[1], (char *)&msg, 1, 0);
//     errno = save_errno;
// }

// 添加信号捕捉
void addsig(int sig, void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

// void call_back(client * user_data) {
//     int sockfd = user_data->user->getsockfd();
//     if (sockfd != -1) {
//         epoll_ctl(user_data->user->m_ep_fd, EPOLL_CTL_DEL, sockfd, NULL);
//         assert(user_data);
//         printf("close %d | ", sockfd);
//         close(sockfd);
//         user_data->user->setsockfd(-1);
//         // printf("usernum : %d\n", http_connect::m_user_num--);
//     }
//     // printf("close fd %d\n", user_data->user.getsockfd());
// }
// static int pipefd[2];
// sort_time_list timer_list;
// 创建数组用于保存所有客户端信息
// client * users = new client[MAX_FD];

int main(int argc, char * argv[]) {

    // basename是当用户命令带路径的时候，只获取程序名
    if (argc <= 1) {
        printf("按照如下格式运行: %s port_number\n", basename(argv[0]));
        return -1;
    }

    printf(" ----- WebServer Start ---- \n");
    // 获取端口号
    int port = atoi(argv[1]);

    // 对SIGPIE信号做处理
    /*
        当客户端close一个连接时，若Server端接着发数据。根据TCP协议的规定，会收到一个RST响应，
        client再往这个服务器发送数据时，系统会发出一个SIGPIPE信号给进程，告诉进程这个连接已经断开了，不要再写了。
        又或者当一个进程向某个已经收到RST的socket执行写操作时，内核向该进程发送一个SIGPIPE信号。
        该信号的缺省操作是终止进程，因此进程必须捕获它以免不情愿的被终止。

        对一个已经收到FIN包的socket调用read方法, 如果接收缓冲已空, 则返回0, 这就是常说的表示连接关闭. 
        但第一次对其调用write方法时, 如果发送缓冲没问题, 会返回正确写入(发送). 
        但发送的报文会导致对端发送RST报文, 因为对端的socket已经调用了close, 完全关闭, 既不发送, 也不接收数据. 
        所以, 第二次调用write方法(假设在收到RST之后), 会生成SIGPIPE信号, 导致进程退出
    */
    addsig(SIGPIPE, SIG_IGN);

    // 创建监听socket
    int lsock = socket(PF_INET, SOCK_STREAM, 0);
    if (lsock == -1) {
        perror("socket");
        exit(-1);
    }

    // 设置端口复用
    int reuse = 1;
    int ret = setsockopt(lsock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    if (ret == -1) {
        perror("setsockopt");
        exit(-1);
    }

    // socket绑定地址
    struct sockaddr_in saddr;
    saddr.sin_family = PF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port =  htons(port);
    ret = bind(lsock, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }

    // 监听
    ret = listen(lsock, 2048);
    if (ret == -1) {
        perror("listen");
        exit(-1);
    }

    // 设置信号处理函数
    // addsig( SIGALRM, sighandler );
    // addsig( SIGTERM, sighandler );
    // bool stop_server = false;
    // alarm(TIMESLOT); // 定时5秒后产生SIGALARM信号
    MainReactor * mainreactor = new MainReactor(lsock);
    mainreactor->acceptConnect();
    
    close(lsock);
    // delete [] users;
    return 0;
}