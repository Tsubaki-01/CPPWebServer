#include "webserver.h"

WebServer::WebServer(int port, int trigMode, int timeoutMs, bool optLinger, // 服务器设置
    int sqlPort, const char* sqlUser, const char* sqlPwd, const char* dbName, // 数据库连接
    int connPoolNum, int threadNum, // 池化容量
    bool openLog, int logLevel, int logQueSize)  // 日志
    :port_(port), timeoutMs_(timeoutMs), openLinger_(optLinger), isClose_(false)
{
    timer_ = std::make_unique<timer>();
    threadPool_ = std::make_unique<ThreadPool>(threadNum);
    epoller_ = std::make_unique<Epoller>();

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);
    HttpConn::srcDir = srcDir_;
    HttpConn::userCnt = 0;

    initEventMode_(trigMode);
    if (initSocket_() == false)
        isClose_ = true;

    if (openLog)
    {
        Log::instance().init(logLevel, "./log", ".log", logQueSize);
        if (isClose_)
        {
            LOG_ERROR("========== Server init error!==========");// 写日志
        }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, optLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                (listenEvent_ & EPOLLET ? "ET" : "LT"),
                (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
            // 写日志
        }
    }

    SqlConnPool::instance().init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
};

WebServer::~WebServer()
{
    isClose_ = true;
    close(listenFd_);
    free(srcDir_);
    SqlConnPool::instance().closePool();
}


bool WebServer::initSocket_()
{
    int ret;

    struct sockaddr_in addr;
    if (port_ > 65536 || port_ < 1024)
    {
        LOG_ERROR("Port:%d error!", port_);// 写日志
        return false;
    }
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = port_;

    struct linger optlinger = { 0 };
    optlinger.l_onoff = openLinger_ ? 1 : 0;
    optlinger.l_linger = timeoutMs_ / 1000;

    // 打开socket
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0)
    {
        LOG_ERROR("Create socket error!", port_);// 写日志
        return false;
    }

    // 设置socket属性
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, (&optlinger), sizeof(optlinger));
    if (ret < 0)
    {
        LOG_ERROR("Init linger error!", port_);// 写日志
        close(listenFd_);
        return false;
    }

    // 设置socket属性
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&optval), sizeof(int));
    if (ret < 0)
    {
        LOG_ERROR("set socket setsockopt error !");// 写日志
        close(listenFd_);
        return false;
    }

    // 绑定socket和端口
    ret = bind(listenFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("Bind Port:%d error!", port_);// 写日志
        close(listenFd_);
        return false;
    }

    // 检查连接
    ret = listen(listenFd_, 6);
    if (ret < 0)
    {
        LOG_ERROR("Listen port:%d error!", port_);// 写日志
        close(listenFd_);
        return false;
    }

    // 将socket添加到epoll
    ret = epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret < 0)
    {
        LOG_ERROR("Add listen error!");// 写日志
        close(listenFd_);
        return false;
    }

    setFdNonBlock(listenFd_);

    LOG_INFO("Server port:%d", port_);// 写日志

    return true;

};


void WebServer::initEventMode_(int trigMode)
{
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0: // 00
        break;
    case 1: // 01
        connEvent_ |= EPOLLET;
        break;
    case 2: // 10
        listenEvent_ |= EPOLLET;
        break;
    case 3: // 11
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
};

int WebServer::setFdNonBlock(int fd)
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::addClient_(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeoutMs_ > 0)
    {
        timer_->add(fd, timeoutMs_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonBlock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].getFd());// 写日志
};

void WebServer::dealListen_()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    do
    {
        int fd = accept(listenFd_, reinterpret_cast<struct sockaddr*>(&addr), &len);
        if (fd <= 0)
            return;
        else if (HttpConn::userCnt >= Max_FD) {
            sendError_(fd, "Server Busy");
            LOG_WARN("Clients is full!");// 写日志
            return;
        }

        addClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}; // 处理监听事件

void WebServer::dealWrite_(HttpConn* client)
{
    assert(client);
    extendTime_(client);
    threadPool_->addTask(std::bind(&WebServer::onWrite_, this, client));
}; // 处理写事件

void WebServer::dealRead_(HttpConn* client)
{
    assert(client);
    extendTime_(client);
    threadPool_->addTask(std::bind(&WebServer::onRead, this, client));
}; // 处理读事件

void WebServer::sendError_(int fd, const char* info)
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0)
    {
        LOG_WARN("send error to client[%d] error!", fd);// 写日志
    }
    close(fd);
}; // 发送错误信息

void WebServer::extendTime_(HttpConn* client)
{
    assert(client);
    if (timeoutMs_ > 0)
        timer_->adjust(client->getFd(), timeoutMs_);
}; // 延长连接的超时时间
void WebServer::closeConn_(HttpConn* client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());// 写日志
    epoller_->delFd(client->getFd());
    client->closeHttpConn();
}; // 关闭连接

void WebServer::onRead(HttpConn* client)
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN)
    {
        closeConn_(client);
        return;
    }
    onProcess(client);
}; // 处理读操作
void WebServer::onWrite_(HttpConn* client)
{
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->bytesToWrite() == 0) // 传输完成
    {
        if (client->isKeepAlive())
        {
            onProcess(client);
            return;
        }
    }
    else if (ret < 0)
    {
        if (writeErrno == EAGAIN)
        {
            epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}; // 处理写操作
void WebServer::onProcess(HttpConn* client)
{
    if (client->process()) // 有可写内容
    {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
    }
    else {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}; // 处理请求

void WebServer::start()
{
    int timeMs = -1;
    if (!isClose_)
    {
        LOG_INFO("========== Server start ==========");// 写日志
    }

    while (!isClose_)
    {
        if (timeoutMs_ > 0)
            timeMs = timer_->getNextTick();
        int eventCnt = epoller_->wait(timeMs); // 等待事件

        for (int i = 0;i < eventCnt;i++)
        {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i); // 获取第i个事件的信息

            if (fd == listenFd_)
                dealListen_(); // 处理新的连接请求
            else if (events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
            {
                assert(users_.count(fd) > 0 && "user insists");
                closeConn_(&users_[fd]);
            }
            else if (events & (EPOLLIN))
            {
                assert(users_.count(fd) > 0 && "user insists");
                dealRead_(&users_[fd]);
            }
            else if (events & (EPOLLOUT))
            {
                assert(users_.count(fd) > 0 && "user insists");
                dealWrite_(&users_[fd]);
            }
            else
            {
                LOG_ERROR("Unexpected event");
            }
        }
    }
};