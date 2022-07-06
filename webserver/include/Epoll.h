// #pragma once
// #include "http_connect.h"
// #include "util_timer.h"
// #include <vector>
// #include <unordered_map>
// #include <sys/epoll.h>
// #include <memory>

// class Epoll
// {
// public:
//     Epoll();
//     ~Epoll();
//     void epoll_add(int fd, int timeout = -1);
//     void epoll_mod(int fd, int timeout = -1);
//     void epoll_del(int fd);
//     int poll();
//     // std::vector<std::shared_ptr<Channel>> poll();
//     // std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
//     // void add_timer(std::shared_ptr<Channel> request_data, int timeout);
//     int getEpollFd()
//     {
//         return m_epollfd;
//     }
//     void handleExpired();
// private:
//     static const int MAXFDS = 100000;
//     int m_epollfd;                                          // epoll_create()
//     std::vector<epoll_event> events;
//     // std::shared_ptr<Channel> fd2chan_[MAXFDS];             // 根据 fd 快速找到对应的 channel
//     std::shared_ptr<http_connect> fd2http_[MAXFDS];
//     // TimerManager timerManager_;
// };