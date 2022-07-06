#ifndef MAINREACTOR_H
#define MAINREACTOR_H

// #include "util_timer.h"
#include "SubReactor.h"

#define SUBCREATOR_NUM 4

/*
    负责通过epoll监控连接事件：
    收到连接请求后通过Acceptor对象中的accept获取连接，将新的连接随机分配给某一个SubReactor对象
*/
class MainReactor
{
private:
    int select;
    class Acceptor // 内置连接类
    {
    private:
        sem *m_sem;     
        int m_ep_fd;
        int lsock;
        SubReactor * subreactor[SUBCREATOR_NUM];
    public:
        Acceptor() {
        }
        Acceptor(int _lsock): lsock(_lsock) {}
        ~Acceptor() {}

        void init() {
            m_sem = new sem(1);
            for (int i = 0; i < SUBCREATOR_NUM; ++i) {
                subreactor[i] = new SubReactor();
            }
            printf("SubReactor()\n");
            
            SubReactor::clientinit();
            printf("clientinit() %d\n", SUBCREATOR_NUM);

            for (int i = 0; i < SUBCREATOR_NUM; ++i) {
                printf("初始化subcreator[%d] = \n", i);
                subreactor[i]->init();
            }
        }

        void process(int connfd, sockaddr_in caddr, int num) {

            // 将新的客户端数据初始化并放入数组中
            // 随机选择一个SubCreator对象
            // 当一个SubReactor监听的连接数到达最大限制时，顺延到下一个
            subreactor[num]->addfd(connfd, caddr, true);
            
        }
    };
    
    int m_ep_l;
    int lsock;
    epoll_event ep_events;  
    Acceptor * ac;
    ThreadPool * acpool;
public:
    MainReactor(int _lsock) : lsock(_lsock), acpool(new ThreadPool(8)), select(-1) {
        m_ep_l = epoll_create(1);
        if (m_ep_l == -1) {
            perror("epoll_create");
            exit(-1);
        }
        epoll_event ev;
        ev.data.fd = lsock;
        ev.events = EPOLLIN | EPOLLET;     // 对方连接断开触发挂起，之前通过read返回值判断是否断开，RDHUP可以在内核判断
        epoll_ctl(m_ep_l, EPOLL_CTL_ADD, lsock, &ev);

        // 设置文件描述符非阻塞
        setnonblocking(lsock);
        ac = new Acceptor(lsock);
        // acpool = NULL; // 负责业务逻辑操作
        // try {
        //     acpool = new ThreadPool(4);
        // } catch (...) {
        //     exit(-1);
        // }
    }
    ~MainReactor() {}

    // 只负责检测是否有客户端连接
    int acceptConnect() {
        printf("init()\n");
        ac->init();
        while (1) {
            int num = epoll_wait(m_ep_l, &ep_events, MAX_EVENT_NUM, -1);
            if ((num < 0) && (errno != EINTR)) {
                perror("epoll_wait");
                break;
            }
            int connfd = -1;
            struct sockaddr_in caddr;
            socklen_t len_caddr = sizeof(caddr);
            // 由于listenSocket设置了ET模式，因此需要一次性将所有连接accept
            while ((connfd = accept(lsock, (struct sockaddr *)&caddr, &len_caddr)) > 0) {
                if (connfd >= MAX_FD) {
                    LOG << "连接已满，无法连接\n";
                    close(connfd);
                    continue;
                }
                LOG << "New connection from " << inet_ntoa(caddr.sin_addr) << ":" << ntohs(caddr.sin_port);
                select = connfd % SUBCREATOR_NUM;
                acpool->submit(std::bind(&Acceptor::process, ac, connfd, caddr, select));
                // ac->process(connfd, caddr, select);  
            } 
            if (connfd == -1 && errno != EAGAIN) {
                perror("accept");
                return -1;
            }
        }     
    }
};

#endif