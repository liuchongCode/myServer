// #ifndef TIMER_H
// #define TIMER_H

// #include "http_connect.h"
// #include <queue>
// #include <functional>

// class SubReactor;

// class Timer
// {
// private:
//     int index_; // 在堆中的索引
//     bool deleted_;
//     size_t expiredTime_;    // 关闭时间
//     std::shared_ptr<http_connect> SPHttpConnect;
// public:
//     Timer(std::shared_ptr<http_connect> client, int timeout);
//     Timer (Timer &t);
//     ~Timer();
//     void update(int timeout);
//     std::shared_ptr<http_connect> getSPHttpConnect() {
//         return SPHttpConnect;
//     }
//     bool isValid();
//     void clearClient();     // 将节点与httpconn脱离
//     void setDeleted() { deleted_ = true; }
//     bool isDeleted() const { return deleted_; }
//     size_t getExpTime() const { return expiredTime_; }
// };

// struct TimerCmp {
//     bool operator() (std::shared_ptr<Timer> &a, std::shared_ptr<Timer> &b) {
//         return a->getExpTime() > b->getExpTime();   // 小根堆
//     }
// };

// class TimerManager
// {
// private:
//     typedef std::shared_ptr<Timer> spTimerNode;
//     std::priority_queue<spTimerNode, std::deque<spTimerNode>, TimerCmp> timerQueue;
// public:
//     TimerManager();
//     ~TimerManager();
//     void addTimer(std::shared_ptr<http_connect> client, int timeout);
//     void handleExpiredClient();
// };

// #endif

