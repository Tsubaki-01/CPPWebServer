#include <iostream>
#include "sqlconnpool.h"

int main() {
    const char* host = "localhost";
    const char* user = "root";
    const char* pwd = "password";
    const char* dbName = "test";
    int port = 3306;

    // 初始化连接池
    SqlConnPool::instance().init(host, port, user, pwd, dbName);

    // 获取连接并执行查询
    MYSQL* conn = SqlConnPool::instance().getConn();
    if (conn) {
        std::cout << "Successfully connected to MySQL!" << std::endl;

        // 执行查询操作
        // 这里可以执行一些数据库操作

        // 释放连接
        SqlConnPool::instance().FreeConn(conn);
    }
    else {
        std::cerr << "Failed to connect to MySQL!" << std::endl;
    }

    // 关闭连接池
    SqlConnPool::instance().closePool();

    return 0;
}
