# 基于Linux的C++多线程HTTP服务器

#### 介绍
C++编写的web服务器；设计实现了主从Reactor并发模型，非阻塞IO+线程池；解析了get请求；并实现了异步日志，记录服务器运行状态。

#### 软件架构
![输入图片说明](reactor%E4%B8%BB%E4%BB%8E%E6%A8%A1%E5%BC%8F.png)

#### 使用说明

1.  make
2.  cd build
3.  ./main [port]

#### 技术点
1.  使用主从Reactor模式+EPOLL(ET)边缘触发的IO多路复用技术；
2.  MainReactor只负责accept请求，通过随机的Round Robin算法分发给SbuReactor；
3.  使用多线程处理并发请求，创建了线程池避免线程频繁创建、销毁的开销；
4.  使用双缓冲技术实现了简单的异步日志系统；
5.  实现了基于小根堆的定时器，处理超时连接；
6.  支持GET请求解析；
7.  使用WebBench对服务器进行测压，支持上万并发；

#### 压力测试
> * 在同一宿主机中分别开启两个虚拟机
> * 宿主机：内存16GB 处理器：Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz   2.90 GHz
> * 虚拟机：内存4GB  处理器：8核

> * 测试时长：10s
> * 最高并发数20000时，失败请求0
> * 25000时由10个左右失败请求
> * 一万并发时QPS>20000
> * 由于在同一台宿主机，因此测试时宿主机压力很大，感觉对测试结果有影响
