# CPPWebServer

[TOC]

## 介绍

本项目为Linux环境下的轻量级Web服务器，项目代码为C++ 11 风格。

实现了实现对一定数量客户端服务的支持与响应，并允许客户端访问静态网页资源。

## 项目主要工作

- 采用**主从 Reactor** + **线程池**模式作为框架，使用 **Epoll** **实现 IO 多路复用**。
- 依据**生产者消费者**模型，构建**线程池**，使得服务器能够处理并发请求。采用**单例模式**构建**数据库连接池**，并实现了对数据库连接的 **RAII 管理**。
- 采用**有限状态机**的思想解析处理 HTTP 报文。
- 依据多生产者单消费者模型，通过**阻塞队列**实现了服务器的**异步日志系统**。
- 基于**小顶堆**维护一个定时器，记录客户端连接的时间信息，并及时关闭不活跃连接。
- 仿照实现了**自动增长**的缓冲区。
- 使用互斥锁和条件变量实现PV信号量。

## TODO LIST

- [ ] 单缓冲区日志 - > 读写双缓冲区日志
- [ ] 内存池
- [ ] 池连接的负载均衡
- [ ] 对于响应文件内存映射的页面缓存 / LFU OR LRU？
- [ ] 服务器性能测试

## 环境

- Linux
- C++ 14
- MySQL

## 文件夹

- Note——项目实现过程中所做的笔记

- build
- src——代码文件
- test——针对各个模块的功能测试
- resources——网页静态资源（暂无）
- log——运行时日志（运行时创建）

## 项目构建

- ### 配置数据库

```mysql
// 创建数据库
create database webserver;

// 创建user表
USE webserver;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

- ### 项目构建

进入项目根目录

```bash
make
```

待完成后，运行以下命令即可打开Web服务器

```bash
./bin/server
```

## 推荐实现顺序

该部分仅针对本人在项目过程中考虑过的适合自己学习的功能实现顺序。

1. lock
2. buffer
3. DBConnPool
4. ThreadPool
5. http
6. timer
7. log
8. server

## 致谢

项目过程中参考借鉴了以下内容，受益匪浅。

- Linux高性能服务器编程，游双

- [TinyWebServer](https://github.com/qinguoyi/TinyWebServer )

- [WebServer](https://github.com/markparticle/WebServer )
