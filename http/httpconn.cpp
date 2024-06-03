#include "httpconn.h"

HttpConn::HttpConn() :fd_(-1), addr_({ 0 }), isClose_(true), iovCnt_(-1) {};

HttpConn::~HttpConn()
{
    close();
};
void HttpConn::init(int sockFd, const sockaddr_in& addr)
{
    assert(sockFd > 0);
    userCnt++;
    addr_ = addr;
    fd_ = sockFd;
    readBuffer_.retrieveAll();
    writeBuffer_.retrieveAll();
    isClose_ = false;
    // 写日志
};

ssize_t HttpConn::read(int* errNo);
ssize_t HttpConn::write(int* errNo);

void HttpConn::close();

int HttpConn::getFd() const
{
    return fd_;
};
int HttpConn::getPort() const
{
    return addr_.sin_port;
};
const char* HttpConn::getIP() const
{
    static char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_.sin_addr, ip, INET_ADDRSTRLEN); // 字节序地址转字符串ipv4地址
    return ip;
};
sockaddr_in HttpConn::getAddr() const
{
    return addr_;
};

bool HttpConn::process()
{
    // 这边没有保证线程安全,但是缓冲区都是属于各个连接的
    request_.init();
    if (readBuffer_.readableBytes() <= 0) // 没有http请求
        return false;
    else if (request_.parser(readBuffer_)) // 解析http请求成功
    {
        // 写日志
        response_.init(srcDir, request_.path(), request_.isKeepAlive());
    }
    else response_.init(srcDir, request_.path(), false, 400); // http请求格式有误

    response_.handleResponse(writeBuffer_);

    iov_[0].iov_base = writeBuffer_.readPtr();
    iov_[0].iov_len = writeBuffer_.readableBytes();
    iovCnt_ = 1;

    if (response_.file() && response_.fileLength()) // 有内存映射并且文件中有内容
    {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLength();
        iovCnt_ = 2;
    }
    //写日志

    return true;
};