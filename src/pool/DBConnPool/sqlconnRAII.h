#ifndef SQLCONNRAII_H

#define SQLCONNRAII_H
#include "sqlconnpool.h"

// RAII  避免手动释放数据库连接
class sqlConnRAII
{
public:
    // 从connPool里面取连接到sql
    sqlConnRAII(MYSQL*& sql, SqlConnPool& connPool)
    {
        if (sql == nullptr)
            exit(1);
        sql = connPool.getConn();
        sql_ = &sql;
        connPool_ = &connPool;
    }
    ~sqlConnRAII()
    {
        connPool_->FreeConn(*sql_);
    }
private:
    // 使用指针防止拷贝，节省空间
    MYSQL** sql_;
    SqlConnPool* connPool_;
};



#endif // ~SQLCONNRAII_H