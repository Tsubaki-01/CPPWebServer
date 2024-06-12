#include "httpconn.h"

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCnt;
bool HttpConn::isET;

HttpConn::HttpConn() :fd_(-1), addr_({ 0 }), isClose_(true), iovCnt_(-1) {};
HttpConn::~HttpConn()
{
    closeHttpConn();
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
void HttpConn::closeHttpConn()
{
    if (isClose_ == false)
    {
        isClose_ = true;
        userCnt--;
        close(fd_);
        // 写日志
    }
};

ssize_t HttpConn::read(int* errNo)
{
    ssize_t len = -1;
    do {
        len = readBuffer_.readFd(fd_, errNo);
        if (len <= 0)
            break;
    } while (isET); // ET模式需要一次性读完所有数据

    return len;
};
ssize_t HttpConn::write(int* errNo)
{
    ssize_t len = -1;
    if (iov_[0].iov_len + iov_[1].iov_len == 0) return 0;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0)
        {
            if (len < 0)
                *errNo = errno;
            break;
        }

        if (static_cast<size_t>(len) > iov_[0].iov_len) // iov_[1]中还有数据没写完，需要更新iov中的变量 // 类型转换防止iov_len过大被解析为ssize_t的负数
        {
            iov_[1].iov_base = static_cast<uint8_t*> (iov_[1].iov_base) + (len - iov_[0].iov_len); // iov_base是void*类型的指针，需要转换为字节
            iov_[1].iov_len = iov_[0].iov_len + iov_[1].iov_len - len;

            if (iov_[0].iov_len) // 更新iov_[0]的内容
            {
                iov_[0].iov_len = 0;
                writeBuffer_.retrieveAll();
                iov_[0].iov_base = const_cast<void*>(reinterpret_cast<const void*>(writeBuffer_.readPtr()));
            }
        }
        else // 只用到了iov_[0]
        {
            // iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuffer_.retrieve(len);
            iov_[0].iov_base = const_cast<void*>(reinterpret_cast<const void*>(writeBuffer_.readPtr()));
        }

        if (iov_[0].iov_len + iov_[1].iov_len == 0) break; // 写结束
    } while (isET || bytesToWrite() > 1024); // ET模式下或者待写数据超出缓冲区大小

    return len;
};

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

    iov_[0].iov_base = const_cast<void*>(reinterpret_cast<const void*>(writeBuffer_.readPtr()));
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