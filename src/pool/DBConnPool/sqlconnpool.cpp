#include "./sqlconnpool.h"

SqlConnPool::SqlConnPool()
{
    usedCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool::~SqlConnPool()
{
    closePool();
}

/* 单例模式初始化
由于析构函数是private，不使用智能指针 */
SqlConnPool& SqlConnPool::instance()
{
    static SqlConnPool instance;
    return instance;
}

// 获取数据库连接
MYSQL* SqlConnPool::getConn()
{
    // 暂无可用连接
    if (sqlConnQue_.empty())
    {
        // 写日志
        return nullptr;
    }
    MYSQL* sql = nullptr;
    // P 操作
    sem_.wait();
    // 取连接     ——优化：负载均衡
    {
        std::lock_guard<std::mutex> lock(mtx_);
        sql = sqlConnQue_.front();
        sqlConnQue_.pop();
        --freeCount_;
        ++usedCount_;
    }
    return sql;
}

// 释放连接
void SqlConnPool::FreeConn(MYSQL* sqlConn)
{
    assert(sqlConn);
    // 还连接
    {
        std::lock_guard<std::mutex> lock(mtx_);
        sqlConnQue_.push(sqlConn);
        ++freeCount_;
        --usedCount_;
    }
    // V 操作
    sem_.signal();
}

// 查询可用连接数量
int SqlConnPool::getFreeConnCount()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return sqlConnQue_.size();
}

// 初始化连接池
void SqlConnPool::init(const char* host, int port,
    const char* user, const char* pwd,
    const char* dbName, int connSize)
{
    assert(connSize > 0);

    for (int i = 0;i < connSize;i++)
    {
        MYSQL* sql = mysql_init(nullptr);
        if (!sql)
        {
            // 写日志
            exit(1);
        }

        sql = mysql_real_connect(sql, host,
            user, pwd, dbName, port, nullptr, 0);
        if (!sql)
        {
            // 写日志
            exit(1);
        }

        sqlConnQue_.push(sql);
        ++freeCount_;
    }

    MAX_CONN_ = connSize;
    sem_.exchange(connSize);
}

// 关闭连接池
void SqlConnPool::closePool()
{
    std::lock_guard<std::mutex> lock(mtx_);
    while (!sqlConnQue_.empty())
    {
        auto item = sqlConnQue_.front();
        sqlConnQue_.pop();
        mysql_close(item);
    }
    MAX_CONN_ = 0;
    usedCount_ = 0;
    freeCount_ = 0;
    mysql_library_end();
}
