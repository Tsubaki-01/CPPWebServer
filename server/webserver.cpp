#include "webserver.h"

Webserver::Webserver(int port, int trigMode, int timeoutMs, bool optLinger, // 服务器设置
    int sqlPort, const char* sqlUser, const char* sqlPwd, const char* dbName, // 数据库连接
    int connPoolNum, int threadNum, // 池化容量
    bool openLog, int logLevel, int logQueSize)  // 日志
    :port_(port), openLinger_(optLinger), timeoutMs_(timeoutMs), isClose_(false)
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
            // 写日志
        }
        else {

            // 写日志
        }
    }

    SqlConnPool::instance().init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
};

Webserver::~Webserver()
{
    isClose_ = true;
    close(listenFd_);
    free(srcDir_);
    SqlConnPool::instance().closePool();
}

bool Webserver::initSocket_()
{
    int ret;

};
