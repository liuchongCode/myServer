# 基于Linux的C++多线程HTTP服务器

#### 介绍
C++编写的web服务器；使用了主从Reactor并发模型，非阻塞IO+线程池；解析了get请求；并实现了异步日志，记录服务器运行状态。

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
6.  支持GET请求解析与优雅的关闭连接；
7.  使用WebBench对服务器进行测压，支持上万并发；

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
