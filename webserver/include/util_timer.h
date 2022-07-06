// #ifndef TIMER_H
// #define TIMER_H

// #include <time.h>
// #include "http_connect.h"

// class util_timer; //前向声明，由于之后的结构体和类有互相使用的情况

// // 定时器类
// class util_timer
// {
// public:
//     util_timer * prev; // 指向前一个节点
//     util_timer * next; // 指向后一个节点
//     client * user_data;  
//     util_timer(): prev(NULL), next(NULL) {}
//     ~util_timer() {
//     }

//     time_t expire; // 任务超时时间，这里使用绝对时间
//     void (*call_back)(client *); // 任务回调函数，处理客户端数据，由定时器的执行者传递给回调函数
// };

// /*
//     定时器链表类:
//         1、将节点按照不活跃时间进行排序，排在队头的节点的http连接会最先被关闭
//         2、当有的连接有信息后，更新其时间，并更新其在链表中的位置
//         3、当连接超时后，要关闭连接并删除对应的节点
// */
// class sort_time_list {
// private:
//     util_timer *head, *tail;

// public:
//     sort_time_list(): head(NULL), tail(NULL) {}
//     ~sort_time_list() {
//         // 遍历并删除节点
//         util_timer * tmp = head->next;
//         while (tmp) {
//             delete head;
//             head = tmp;
//             tmp = tmp->next;
//         }
//     }

//     // 将目标定时器从head开始检查，添加到链表中
//     void add_timer(util_timer * timer) {
//         if (!timer) {
//             return;
//         }
        
//         // 链表为空
//         if (!head) {
//             head = tail = timer;
//             return;
//         }

//         // 保证链表升序
//         if (timer->expire < head->expire) {
//             timer->next = head;
//             head->prev = timer;
//             head = timer;
//             head->prev = NULL;
//             return;
//         }
//         add_timer(timer, head->next);
        
//     }

//     // 将目标定时器从指定节点(不是头节点也不是尾节点)开始检查，添加到链表中
//     void add_timer(util_timer * timer, util_timer * curr) {
//         if (!timer) {
//             return;
//         }

//         while (curr) {
//             if (timer->expire < curr->expire) {
//                 timer->next = curr;
//                 timer->prev = curr->prev;
//                 curr->prev = timer;
//                 timer->prev->next = timer;
//                 break;
//             }

//             curr = curr->next;
//         }
        
//         // 到了尾节点的下一个
//         if (!curr) {
//             tail->next = timer;
//             timer->prev = tail;
//             tail = timer;
//             tail->next = NULL;
//         }
//     }

//     // 当某个定时任务发生变化时要更新节点位置
//     void adjust_timer(util_timer * timer) {
//         if (!timer) {
//             return;
//         }

//         // 已经在尾部或者依然小于后一个节点，则不用调整
//         util_timer * tmp = timer->next;
//         if (!tmp || timer->expire < tmp->expire) {
//             return;
//         }

//         if (timer == head) {
//             head = head->next;
//             head->prev = NULL;
//             timer->next = NULL;
//             add_timer(timer);
//         } else {
//             timer->next->prev = timer->prev;
//             timer->prev->next = timer->next;
//             add_timer(timer, timer->next);
//         }

//     }

//     // 删除节点
//     void del_timer(util_timer * timer) {
//         if (!timer) {
//             return;
//         }

//         if (timer == head && timer == tail) {
//             delete timer;
//             head = NULL;
//             tail = NULL;
//             return;
//         }

//         if (timer == head) {
//             head = head->next;
//             head->prev = NULL;
//             delete timer;
//             return;
//         }

//         if (timer == tail) {
//             tail = tail->prev;
//             tail->next = NULL;
//             delete timer;
//             return;
//         }

//         timer->prev->next = timer->next;
//         timer->next->prev = timer->prev;
//         delete timer;
//     }

//     /* 
//         SIGALARM 信号每次被触发就在其信号处理函数中执行一次 tick() 函数，以处理链表上到期任务。
//     */
//     void tick() {
//         if (!head) {
//             return;
//         }

//         time_t curr = time(NULL); // 获取当前系统时间
//         util_timer * tmp = head;
//         while (tmp) {
//             if (curr < tmp->expire) {
//                 break;;
//             }
//             // printf("调用回调函数\n");
//             // 调用回调函数，执行定时任务(指关闭对应的http连接等操作)
//             tmp->call_back(tmp->user_data);
//             head = tmp->next;

//             if (head) {
//                 head->prev = NULL;
//             }
//             // printf("tmp是否为空 : %d\n", (int)(tmp == NULL));
//             delete tmp;
//             tmp = head;
//         }
//         // printf("tick over\n");
//     }
// };

// #endif