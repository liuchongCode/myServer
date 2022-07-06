#include "SubReactor.h"
#include <iostream>

using namespace std;

std::vector<std::shared_ptr<http_connect>> SubReactor::clients = std::vector<std::shared_ptr<http_connect>> (MAX_FD);

void SubReactor::clientinit() {
    // for (int i = 0; i < MAX_FD; ++i) {
    //     SubReactor::clients[i] = std::make_shared<http_connect>();
    // }
}

void SubReactor::init() {
    
    m_ep_fd = epoll_create(1);
    if (m_ep_fd == -1) {
        perror("epoll_create");
        exit(-1);
    }

    pool->submit(std::bind(&SubReactor::listen, this));
}

// 添加文件描述符到epoll实例
void SubReactor::addfd(int fd, const sockaddr_in &caddr, bool one_shot) {
    clients[fd] = std::make_shared<http_connect>();
    clients[fd]->init(m_ep_fd, fd, caddr);
    
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;     // 对方连接断开触发挂起，之前通过read返回值判断是否断开，RDHUP可以在内核判断

    if (one_shot) {
        // 防止同一个通信被不同的线程处理
        ev.events |= EPOLLONESHOT;
    }
    
    int ret = epoll_ctl(m_ep_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
        perror("epoll_ctl");
        close(fd);
        return;
    }
    usernum++;
    // cout << "向ep = " << m_ep_fd << "中添加 fd = " << fd << "\n";
    LOG << "向ep = " << m_ep_fd << "中添加 fd = " << fd << "\n";
    // TimeQueue->add(clients[fd], TIMEOUT);
    // 设置文件描述符非阻塞

    setSocketNoLinger(fd);
    ret = setnonblocking(fd);
    // TimeQueue->add(clients[fd], TIMEOUT);
}

// 为管道添加fd
void SubReactor::addfd(int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( m_ep_fd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

// 删除epoll实例中删除文件描述符
bool SubReactor::delfd(int fd) {
    int ret = epoll_ctl(m_ep_fd, EPOLL_CTL_DEL, fd, NULL);

    if (ret == -1) {
        perror("epoll_ctl");
        return false;
    }
    return true;
}

// 修改fd，主要是为了重置socket上的EPOLLONESHOT事件
// 以确保下一次可读时，EPOLLIN事件能被触发
void SubReactor::modfd(int fd, int ev) {
    assert(fd != -1);
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    // printf("修改ep = %d 中的fd = %d 的事件为: %s\n", m_ep_fd, fd, (ev==EPOLLIN ? "EPOLLIN" : "EPOLLOUT"));
    int ret = epoll_ctl(m_ep_fd, EPOLL_CTL_MOD, fd, &event);
    if (ret == -1) {
        perror("epoll_ctl");
        return;
    }
}

void SubReactor::listen() {
    bool timeout = false;
    bool stop_server = false;
    
    while (1) {
        int waittime = TimeQueue->getnexttick();
        // printf("waittime : %d\n", waittime);
        // 当新链接注册时，此处如果一直阻塞，将导致死锁
        int num = epoll_wait(m_ep_fd, events, MAX_EVENT_NUM, waittime);
        if ((num < 0) && (errno != EINTR)) {
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < num; ++i) {
            int sockfd = events[i].data.fd;
            if ((events[i].events & EPOLLHUP) && !(events[i].events & EPOLLIN))
                continue;
            else if (events[i].events & EPOLLERR) {
                LOG << "对方断开连接\n";
                // pool->submit(std::bind(&SubReactor::closeConn_, this, clients[sockfd]));
                if (clients[sockfd] != NULL) {
                    closeConn_(clients[sockfd]);
                }
                continue;
            } else if (events[i].events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) { 
                handleRead_(clients[sockfd]);
            } else if (events[i].events & EPOLLOUT) {
                handleWrite_(clients[sockfd]);
            }
        }
        // 最后处理定时事件，因为I/O事件有更高的优先级。当然，这样做将导致定时任务不能精准的按照预定的时间执行。
        TimeQueue->handleExpiredClient();
    }
}

void SubReactor::handleWrite_(std::shared_ptr<http_connect> client) {
    // cout << "handleWrite_\n";
    assert(client);
    // TimeQueue->add(client, TIMEOUT);
    pool->submit(std::bind(&SubReactor::onWrite_, this, client));
}

void SubReactor::handleRead_(std::shared_ptr<http_connect> client) {
    // cout << "handleRead_\n";
    assert(client);
    pool->submit(std::bind(&SubReactor::onRead_, this, client));
}

void SubReactor::onRead_(std::shared_ptr<http_connect> client) {
    // LOG << "onRead_\n";
    assert(client);
    int reterrno = 0;
    bool ret = client->read(reterrno);
    if (!ret) {
        // LOG << "读取失败\n";
        closeConn_(client);
        return;
    }
    // pool->submit(std::bind(&SubReactor::onProcess_, this, client));
    onProcess_(client);
}

void SubReactor::onWrite_(std::shared_ptr<http_connect> client) {
    // LOG << "onWrite_\n";
    assert(client);
    int reterrno = 0;
    int ret = client->write(reterrno);
    if (ret == 0) {
        /* TCP缓冲区满，等待下次写事件 */
        modfd(client->getsockfd(), EPOLLOUT);
        return;
    } else if (ret == 1) {
        /* 写完成且keepalive */
        TimeQueue->add(client, TIMEOUT);
        modfd(client->getsockfd(), EPOLLIN);
        return;
    }
    /* 写完成或者写出错 */
    closeConn_(client);
}

void SubReactor::onProcess_(std::shared_ptr<http_connect> client) {
    assert(client);
    int ret = client->process();
    if (ret == 0) {
        modfd(client->getsockfd(), EPOLLOUT);
    } else if (ret == 1) {
        modfd(client->getsockfd(), EPOLLIN);
    } else {
        LOG << "解析失败，关闭连接\n";
        closeConn_(client);
    }
}

void SubReactor::closeConn_(std::shared_ptr<http_connect> client) {
    assert(client);
    // if (client->getTimer()) {
    //     client->getTimer()->clearClient();
    //     client->getTimer()->setDeleted();
    // }
    
    // int fd = client->getsockfd();
    // delfd(fd);
    if (!TimeQueue->setDeleted(client->getsockfd())) {
        client->close_conn();
        // clients[client->getsockfd()].reset();
    }
        
}

