#include "log.h"

Log::Log() :lineCount_(0), today_(0), isOpen_(true), isAsync_(false), fp_(nullptr),
deque_(nullptr), writeThread_(nullptr)
{ };


Log& Log::instance()
{
    static Log log;
    return log;
}; // 获取单例


void Log::init(int level,
    const char* path,
    const char* suffix,
    int maxQueueCapacity)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;

    // 同步与异步
    if (maxQueueCapacity > 0)
    {
        isAsync_ = true;
        if (deque_ == nullptr) // 懒加载
        {
            deque_ = std::make_unique<BlockDeque<std::string>>();

            writeThread_ = std::make_unique<std::thread>(flushLogThread);
        }
    }
    else {
        isAsync_ = false;
    }

    // 获取系统时间并编写日志文件名
    time_t timer = time(nullptr);
    struct tm t = *localtime(&timer);

    char fileName[LOG_NAME_LEN] = { 0 };
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
        path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    today_ = t.tm_mday;

    // 清空缓冲区并打开日志文件
    {
        std::lock_guard<std::mutex> lock(mtx_);
        buffer_.retrieveAll();
        if (fp_)
        {
            fflush(fp_); // 清空缓冲区
            fclose(fp_);
        }

        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }

};


void Log::appendLogLevelTitle(int level)
{
    switch (level) {
    case 0:
        buffer_.append("[debug]: ", 9);
        break;
    case 1:
        buffer_.append("[info] : ", 9);
        break;
    case 2:
        buffer_.append("[warn] : ", 9);
        break;
    case 3:
        buffer_.append("[error]: ", 9);
        break;
    default:
        buffer_.append("[info] : ", 9);
        break;
    }
};


void Log::asyncWrite_()
{
    std::string str;
    while (deque_->pop(str) && fp_)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        fputs(str.c_str(), fp_);
        fflush(fp_);
    }
}; // 异步写日志的实际操作
void Log::flushLogThread()
{
    Log::instance().asyncWrite_();
}; // 异步刷新日志线程


void Log::write(int level, const char* format, ...)
{
    struct timeval now = { 0, 0 };
    gettimeofday(&now, nullptr);
    time_t timer = time(nullptr);
    struct tm t = *localtime(&timer);

    va_list valist;

    // 需要创建新的日志
    if (today_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0)))
    {
        char newFile[LOG_NAME_LEN];
        char tail[36] = { 0 };
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (today_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 1, "%s/%s%s", path_, tail, suffix_);
            today_ = t.tm_mday;
            lineCount_ = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 1, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);
        }

        // 加锁并修改共享资源
        {
            std::unique_lock<std::mutex> locker(mtx_);
            flush();
            fclose(fp_);
            fp_ = fopen(newFile, "a");
            assert(fp_ != nullptr);
        }
    }

    // 写日志
    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_++;
        int len_time = snprintf(buffer_.writePtr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buffer_.hasWritten(len_time);

        // 等级信息
        appendLogLevelTitle(level);

        // 可变参数列表
        va_start(valist, format);
        int len_va = vsnprintf(buffer_.writePtr(), buffer_.writeableBytes(),
            format, valist);
        va_end(valist);
        buffer_.hasWritten(len_va);

        buffer_.append("\n\0", 2);

        // 写日志
        if (isAsync_ && deque_) // 异步
        {
            if (!deque_->full())
            {
                deque_->push_back(buffer_.retrieveAllToString());
            }
            else {
                // 强制处理队列中的一些消息，确保顺序
                while (!deque_->empty())
                {
                    flush();
                }
                deque_->push_back(buffer_.retrieveAllToString());
            }
        }
        else { // 同步
            fputs(buffer_.readPtr(), fp_);
        }
        buffer_.retrieveAll();
    }

}; // 写入阻塞队列
void Log::flush()
{
    if (fp_)
    {
        if (isAsync_)
            deque_->flush();
        fflush(fp_);
    }
}; // 强制刷新日志到文件一次。从阻塞队列取数据写入日志


int Log::getLogLevel()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return level_;
};
void Log::setLevel(int level)
{
    std::lock_guard<std::mutex> lock(mtx_);
    level_ = level;
};


Log::~Log()
{
    if (writeThread_ && writeThread_->joinable())
    {
        while (!deque_->empty())
        {
            flush();
        }
        writeThread_->join();
    }

    if (fp_)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        flush();
        fclose(fp_);
    }
};
