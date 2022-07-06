// #include "PriorityQueue.h"
// #include <iostream>

// using namespace std;

// int main() {
//     PriorityQueue q;
//     for (size_t i = 0; i < 10; ++i) {
//         std::shared_ptr<http_connect> a = std::make_shared<http_connect>();
//         a->setsockfd(i);
//         q.add(a, i * 1000);
//     }
//     q.show();
//     printf("-------------------\n");

//     for (size_t i = 3; i < 6; ++i) {
//         std::shared_ptr<http_connect> a = std::make_shared<http_connect>();
//         a->setsockfd(i);
//         q.add(a, (i + 5) * 1000);
//     }
//     q.show();
//     sleep(3);

//     for (int i = 6; i < 8; ++i) {
//         q.setDeleted(i);
//     }
//     q.handleExpiredClient();
//     return 0;
// }