#ifndef LOG_H

#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <string.h>
#include <cstdarg>
#include <cassert>
#include <chrono>
#include <sys/time.h>
#include <sys/stat.h>         //mkdir

#include "blockdeque.h"
#include "../buffer/buffer.h"

class Log
{
private:
    static const int LOG_PATH_LEN = 256; // 日志路径的最大长度
    static const int LOG_NAME_LEN = 256; // 日志名称的最大长度
    static const int MAX_LINES = 50000; // 每个日志文件的最大行数

    const char* path_; // 日志文件的路径
    const char* suffix_; // 日志文件的后缀

    // int maxLines_; // 每个日志文件的最大行数，用户配置

    int lineCount_; // 当前日志文件的行数
    int today_; // 当前日志的日期，用于按日期区分日志文件

    bool isOpen_; // 日志系统是否打开

    Buffer buffer_; // 日志缓冲区
    int level_; // 当前日志记录的级别
    bool isAsync_; // 是否异步写日志

    FILE* fp_; // 文件指针，指向当前打开的日志文件
    std::unique_ptr<BlockDeque<std::string>> deque_; // 阻塞队列，用于异步写日志
    std::unique_ptr<std::thread> writeThread_; // 异步写日志的线程
    std::mutex mtx_; // 互斥锁
private:
    Log();
    virtual ~Log();

    void appendLogLevelTitle(int level);

    void asyncWrite_(); // 异步写日志的实际操作

public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void init(int level,
        const char* path = "./log",
        const char* suffix = ".log",
        int maxQueueCapacity = 1024);

    static Log& instance(); // 获取单例
    static void flushLogThread(); // 异步刷新日志线程

    void write(int level, const char* format, ...); // 写入阻塞队列
    void flush(); // 强制刷新日志到文件一次。从阻塞队列取数据写入日志

    int getLevel();
    void setLevel(int level);
    bool isOpen() { return isOpen_; };
};

enum {
    DEBUG,
    INFO,
    WARN,
    ERROR
};


#define LOG_BASE(level, format, ...) \
    do {\
        Log& log = Log::instance();\
        if (log.isOpen() && log.getLevel() <= level) {\
            log.write(level, format, ##__VA_ARGS__); \
            log.flush();\
        }\
    } while (0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);



#endif // ~LOG_H