#ifndef SUBREACTOR_H
#define SUBREACTOR_H

// #include "Timer.h"
#include <assert.h>
#include "writepool.h"
#include "threadpool.h"
#include "Epoll.h"
#include <memory>
#include "ThreadPool.h"
#include "Log/Logger.h"
#include "PriorityQueue.h"

#define TIMEOUT 5000

class SubReactor
{
private:
    // std::shared_ptr<Epoll> poller_;
    static std::vector<std::shared_ptr<http_connect>> clients;
    std::shared_ptr<PriorityQueue> TimeQueue;

    // std::shared_ptr<TimerManager> time_queue;
    // int pipefd[2];
    epoll_event events[MAX_EVENT_NUM];
    int usernum;
    int closenum;
    int recvnum;
    int sendnum;
    // client * users;
    // threadpool<http_connect> * readpool; // 负责业务逻辑操作
    // writepool<http_connect> * writep;
    std::unique_ptr<ThreadPool> pool;
public:
    int m_ep_fd;
    SubReactor() : pool(new ThreadPool(8)), TimeQueue(new PriorityQueue), usernum(0), closenum(0), recvnum(0), sendnum(0) {
        // 创建管道
        // int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
        // assert(ret != -1);
        // setnonblocking(pipefd[1]);
        // addfd(m_ep_fd, pipefd[0]);
    }
    ~SubReactor() {
        printf("SubReactor %d 被销毁\n", m_ep_fd);
    }

    // 初始化
    void init();

    // 初始化客户连接
    static void clientinit();

    // 添加文件描述符到epoll实例
    void addfd(int fd, const sockaddr_in &caddr, bool one_shot);

    // 为管道添加fd
    void addfd(int fd );

    // 删除epoll实例中删除文件描述符
    bool delfd(int fd);

    // 修改fd，主要是为了重置socket上的EPOLLONESHOT事件
    // 以确保下一次可读时，EPOLLIN事件能被触发
    void modfd(int fd, int ev);

    // subreactor监听自己的epfd
    void listen();

    // 监听到写事件时提交写操作
    void handleWrite_(std::shared_ptr<http_connect> client);

    // 监听到读事件时提交读操作
    void handleRead_(std::shared_ptr<http_connect> client);

    // 读操作
    void onRead_(std::shared_ptr<http_connect> client);

    // 写操作
    void onWrite_(std::shared_ptr<http_connect> client);

    // 解析http请求并写入缓冲区
    void onProcess_(std::shared_ptr<http_connect> client);

    //关闭一个HTTP连接
    void closeConn_(std::shared_ptr<http_connect> client);            
};

#endif
