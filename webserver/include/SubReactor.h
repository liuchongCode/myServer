#ifndef SUBREACTOR_H
#define SUBREACTOR_H

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
    static std::vector<std::shared_ptr<http_connect>> clients;
    std::shared_ptr<PriorityQueue> TimeQueue;
    epoll_event events[MAX_EVENT_NUM];
    int usernum;
    int closenum;
    std::unique_ptr<ThreadPool> pool;
public:
    int m_ep_fd;
    SubReactor() : pool(std::make_unique<ThreadPool>(4)), TimeQueue(new PriorityQueue), usernum(0), closenum(0) {
    }
    ~SubReactor() {
        printf("SubReactor %d 被销毁\n", m_ep_fd);
    }

    // 初始化
    void init();

    // 初始化客户连接
    static void clientinit();

    // 添加文件描述符到epoll实例
    void addfd(const int& fd, const sockaddr_in &caddr, bool one_shot);

    // 为管道添加fd
    void addfd(const int& fd);

    // 删除epoll实例中删除文件描述符
    bool delfd(const int& fd);

    // 修改fd，主要是为了重置socket上的EPOLLONESHOT事件
    // 以确保下一次可读时，EPOLLIN事件能被触发
    void modfd(const int& fd, const int& ev);

    // subreactor监听自己的epfd
    void listen();

    // 监听到写事件时提交写操作
    void handleWrite_(std::shared_ptr<http_connect>& client);

    // 监听到读事件时提交读操作
    void handleRead_(std::shared_ptr<http_connect>& client);

    // 读操作
    void onRead_(std::shared_ptr<http_connect>& client);

    // 写操作
    void onWrite_(std::shared_ptr<http_connect>& client);

    // 解析http请求并写入缓冲区
    void onProcess_(const std::shared_ptr<http_connect>& client);

    //关闭一个HTTP连接
    void closeConn_(const std::shared_ptr<http_connect>& client);            
};

#endif
