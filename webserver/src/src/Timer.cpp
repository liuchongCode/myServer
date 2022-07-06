// #include "Timer.h"
// #include <sys/time.h>
// #include <iostream>

// Timer::Timer(std::shared_ptr<http_connect> client, int timeout)
//     :   deleted_(false),
//         SPHttpConnect(client)
// {   
//     // std::cout << "同时有 " << SPHttpConnect.use_count() << " 个引用计数" << std::endl;
//     struct timeval now;
//     gettimeofday(&now, NULL);
//     expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;    // 当前时间加上定时时间
// }

// Timer::Timer(Timer &t) : SPHttpConnect(t.SPHttpConnect)
// {   }

// Timer::~Timer()
// {
// }

// void Timer::update(int timeout) {
//     struct timeval now;
//     gettimeofday(&now, NULL);
//     expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;
// }

// bool Timer::isValid() {
//     struct timeval now;
//     gettimeofday(&now, NULL);
//     size_t temp = (long)(now.tv_sec * 1000) + now.tv_usec / 1000;
//     if (temp < expiredTime_) {
//         return true;
//     } else {      
//         this->setDeleted();
//         return false;
//     }
// }

// void Timer::clearClient() {
//     SPHttpConnect.reset();
//     this->setDeleted();
// }

// TimerManager::TimerManager() {
// }

// TimerManager::~TimerManager() {
// }

// void TimerManager::addTimer(std::shared_ptr<http_connect> client, int tiemout) {
//     spTimerNode new_node(new Timer(client, tiemout));
//     timerQueue.push(new_node);
//     // 如果http_connect之前就已经有Timer节点在队列中，要脱离，再然后再添加
//     // 否则当时间到后，旧Timer会把fd关闭，而该fd还不应该关闭
//     if (client->getTimer() != NULL) {
//         client->getTimer()->clearClient();
//     }
//     client->linkTimer(new_node);
// }

// /* 
//     因为:    
//         (1) 优先队列不支持随机访问
//         (2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
//     所以对于被置为deleted的时间节点，会延迟到它超时 或 它前面的节点都被删除时，它才会被删除。

//     一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
//     这样做有两个好处：
//         (1) 第一个好处是不需要遍历优先队列，省时。
//         (2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，
//             如果监听的请求在超时后的下一次请求中又一次出现了，
//     就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
// */

// void TimerManager::handleExpiredClient() {
//     while (!timerQueue.empty()) {
//         spTimerNode curr = timerQueue.top();
//         if (curr->isDeleted()) {
//             // if (curr->getSPHttpConnect() != NULL)
//             //     curr->getSPHttpConnect()->close_conn();
//             timerQueue.pop();
//         } else if (!curr->isValid()) {
//             // if (curr->getSPHttpConnect() != NULL)
//             //     curr->getSPHttpConnect()->close_conn();
//             timerQueue.pop();
//         } else
//             break;
//     }
// }

