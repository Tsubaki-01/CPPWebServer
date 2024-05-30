#ifndef HTTPREQUEST_H

#define HTTPREQUEST_H

#include "../buffer/buffer.h"


class HttpRequest
{
public:
    // 枚举解析状态
    enum PARSE_STATE {
        REQUEST_LINE,   // 解析请求行
        HEADERS,        // 解析请求头
        BODY,           // 解析请求体
        FINISH          // 完成解析
    };
    // 枚举HTTP请求结果
    enum HTTP_CODE {
        NO_REQUEST = 0,         // 没有请求
        GET_REQUEST,            // 获得请求
        BAD_REQUEST,            // 错误请求
        NO_RESOURSE,            // 没有资源
        FORBIDDENT_REQUEST,     // 禁止请求
        FILE_REQUEST,           // 文件请求
        INTERNAL_ERROR,         // 内部错误
        CLOSED_CONNECTION       // 关闭连接
    };

    HttpRequest() { init(); };
    ~HttpRequest() = default;

    void init();
    bool parser(Buffer& buffer);


private:

}

























#endif // ~HTTPREQUEST_H