#ifndef SQLCONNPOOL_H

#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <mutex>
#include <iostream>
#include <memory>
#include <queue>
#include <cassert>

#include "../../lock/locker.h"


class SqlConnPool
{
public:
    SqlConnPool(const SqlConnPool&) = delete;
    SqlConnPool& operator=(const SqlConnPool&) = delete;

    // 单例模式初始化
    static SqlConnPool& instance();

    MYSQL* getConn();
    void FreeConn(MYSQL* sqlConn);
    int getFreeConnCount();

    void init(const char* host, int port,
        const char* user, const char* pwd,
        const char* dbName, int connSize = defaultConnSize);
    void closePool();

    // 测试函数
    void test_showMessage() {
        std::cout << "Singleton instance" << std::endl;
    }
private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_; // 最大连接数
    int usedCount_; // 已使用连接数
    int freeCount_; // 空闲连接数

    std::queue<MYSQL*> sqlConnQue_; // 连接队列
    std::mutex mtx_; // 互斥锁 负责设计到连接队列的操作
    Semaphore sem_; // PV信号量

    static const int defaultConnSize = 10;
};

#endif // ~SQLCONNPOOL_H