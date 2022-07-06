// #include "Epoll.h"
// #include <errno.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <queue>
// #include <deque>
// #include <assert.h>
// #include <arpa/inet.h>
// #include <iostream>
// using namespace std;


// const int EVENTSNUM = 100000;                                 // 支持事件总数
// // const int EPOLLWAIT_TIME = 10000;                           // 等待时间

// // typedef shared_ptr<Channel> SP_Channel;

// Epoll::Epoll():
//     m_epollfd(epoll_create1(EPOLL_CLOEXEC)),
//     events(EVENTSNUM)
// {
//     assert(m_epollfd > 0);
// }


// Epoll::~Epoll()
// { }


// // 注册新描述符
// void Epoll::epoll_add(int fd, int timeout)
// {
//     epoll_event event;
//     event.data.fd = fd;
//     event.events = EPOLLIN | EPOLLET;
//     epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event );
//     setnonblocking( fd );
//     // if (timeout > 0)
//     // {
//     //     add_timer(request, timeout);
//     //     fd2http_[fd] = request->getHolder();
//     // }
// }


// // 修改描述符状态
// void Epoll::epoll_mod(int fd, int timeout)
// {
//     epoll_event event;
//     event.data.fd = fd;
//     event.events = events[fd].events | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
//     epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
//     // if (timeout > 0)
//     //     add_timer(request, timeout);
// }


// // 从epoll中删除描述符
// void Epoll::epoll_del(int fd)
// {   
//     int ret = epoll_ctl(fd, EPOLL_CTL_DEL, fd, 0);
//     // printf("删除 ep = %d 中的 fd = %d\n", ep_fd, fd);
//     if (ret == -1) {
//         perror("epoll_ctl");
//         return;
//     } else {
//         fd2http_[fd].reset();
//         close(fd);
//     }
        
// }


// // 返回活跃事件数
// int Epoll::poll()
// {
//     while (true)
//     {
//         // 等待事件触发， 返回需要处理的事件数目（就绪事件的数目）
//         int event_count = epoll_wait(m_epollfd, &*events.begin(), events.size(), -1);
//         if (event_count < 0)
//             perror("epoll wait error");
//         if (event_count > 0)
//             return event_count;
//     }
// }

// // void Epoll::handleExpired()
// // {
// //     timerManager_.handleExpiredEvent();
// // }

// // // 分发处理函数
// // std::vector<SP_Channel> Epoll::getEventsRequest(int events_num)
// // {
// //     std::vector<SP_Channel> req_data;
// //     for(int i = 0; i < events_num; ++i)
// //     {
// //         // 获取就绪事件产生的描述符
// //         int fd = events_[i].data.fd;

// //         SP_Channel cur_req = fd2chan_[fd];
            
// //         if (cur_req)
// //         {
// //             cur_req->setRevents(events_[i].events);
// //             cur_req->setEvents(0);
// //             // 加入线程池之前将Timer和request分离
// //             //cur_req->seperateTimer();
// //             req_data.push_back(cur_req);              // 得到就绪事件的 Channel
// //         }
// //         else
// //         {
// //             LOG << "SP cur_req is invalid";
// //         }
// //     }
// //     return req_data;
// // }

// // void Epoll::add_timer(SP_Channel request_data, int timeout)
// // {
// //     shared_ptr<HttpData> t = request_data->getHolder();
// //     if (t)
// //         timerManager_.addTimer(t, timeout);
// //     else
// //         LOG << "timer add fail";
// // }