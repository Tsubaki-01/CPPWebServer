#ifndef WEBSERVER_H

#define WEBSERVER_H

#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <cassert>
#include <cerrno>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "epoll.h"
#include "../log/log.h"
#include "../timer/timer.h"
#include "../pool/DBConnPool/sqlconnRAII.h"
#include "../pool/ThreadPool/threadpool.h"
#include "../http/httpconn.h"

class Webserver
{
private:
    static const int Max_FD = 65536;

    // 端口和超时配置
    int port_; // 服务器监听的端口号
    int timeoutMs_; // 超时时间，单位为毫秒
    bool openLinger_; // 是否开启优雅关闭连接（TCP的SO_LINGER选项）

    // 服务器状态
    bool isClose_; // 服务器是否关闭的标志
    int listenFd_; // 监听socket文件描述符

    // 事件处理
    uint32_t listenEvent_; // 监听事件类型（例如边缘触发或水平触发）
    uint32_t connEvent_; // 连接事件类型

    // 目录和用户连接
    char* srcDir_; // 服务器的根目录
    std::unordered_map<int, HttpConn> users_; // 用户连接，映射文件描述符到HttpConn对象

    std::unique_ptr<timer> timer_; // 定时器，管理连接的超时
    std::unique_ptr<ThreadPool> threadPool_; // 线程池，用于处理请求
    std::unique_ptr<Epoller> epoller_; // Epoll实例，用于事件通知

public:
    Webserver(int port, int trigMode, int timeoutMs, bool optLinger, // 服务器设置
        int sqlPort, const char* sqlUser, const char* sqlPwd, const char* dbName, // 数据库连接
        int connPoolNum, int threadNum, // 池化容量
        bool openLog, int logLevel, int logQueSize); // 日志

    ~Webserver();

    void start();

private:
    static int setFdNonBlock(int fd); // 设置文件描述符为非阻塞模式

    bool initSocket_();
    void initEventMode_(int trigMode);

    void addClient_(int fd, sockaddr_in addr);
    void dealListen_(); // 处理监听事件
    void dealWrite_(); // 处理写事件
    void dealRead_(); // 处理读事件

    void sendError_(int fd, const char* info); // 发送错误信息

    void extentTime_(HttpConn*); // 延长连接的超时时间
    void closeConn_(HttpConn* client); // 关闭连接

    void onRead(HttpConn* client); // 处理读操作
    void onWrite_(HttpConn* client); // 处理写操作
    void onProcess(HttpConn* client); // 处理请求

};

#endif  // ~WEBSEVER_H