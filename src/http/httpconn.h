#ifndef HTTPCONN_H

#define HTTPCONN_H

#include <stdlib.h>
#include <errno.h>
#include <cassert>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/uio.h>     // readv/writev

#include "../log/log.h"
#include "../pool/DBConnPool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httpresponse.h"
#include "httprequest.h"


class HttpConn
{
public:
    HttpConn();
    ~HttpConn();
    void init(int sockFd, const sockaddr_in& addr_);

    ssize_t read(int* errNo);
    ssize_t write(int* errNo);

    void closeHttpConn();

    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;

    bool process();

    int bytesToWrite()
    {
        return iov_[0].iov_len + iov_[1].iov_len;
    }
    bool isKeepAlive() const
    {
        return request_.isKeepAlive();
    }

    static bool isET; // 是否使用边缘触发（Edge Triggered）模式
    static const char* srcDir;
    static std::atomic<int> userCnt; // 当前活动的用户连接数

private:
    int fd_; // 保存连接的socket文件描述符
    struct sockaddr_in addr_; // 保存客户端的地址信息

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuffer_;
    Buffer writeBuffer_; // 写缓冲区用来存响应中除了body之外的较短内容

    HttpRequest request_;
    HttpResponse response_;
};


#endif // ~HTTPCONN_H