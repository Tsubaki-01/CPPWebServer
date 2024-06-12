#ifndef HTTPREQUEST_H

#define HTTPREQUEST_H

#include "../buffer/buffer.h"
#include "../pool/DBConnPool/sqlconnRAII.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <regex>
#include <cassert>
#include <mysql/mysql.h>
#include <memory>


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
    // 解析HTTP请求
    bool parser(Buffer& buffer);

    // 获取请求路径
    std::string path() const;
    std::string& path();

    // 获取请求方法（如GET、POST）
    std::string method() const;
    // 获取HTTP版本（如HTTP/1.1）
    std::string version() const;

    // 获取POST请求参数（根据key获取value）
    std::string getPost(const std::string& key) const;
    std::string getPost(const char* key) const;

    // 判断是否为保持连接（keep-alive）
    bool isKeepAlive() const;

private:
    // 解析请求行
    bool parseRequestLine(const std::string& line);
    // 解析请求头
    void parseHeader(const std::string& line);
    // 解析请求体
    void parseBody(const std::string& line);
    // 解析请求路径
    void parsePath();
    // 解析POST请求数据
    void parsePost();

    // 从URL编码格式解析数据
    void parseFromUrlencoded();

    // 验证用户信息
    static bool userVerify(const std::string& name, const std::string& pwd, bool isLogin);

    // 成员变量
    PARSE_STATE state_;                          // 当前解析状态
    std::string method_, path_, version_, body_; // 请求方法、路径、版本、体
    std::unordered_map<std::string, std::string> header_; // 请求头
    std::unordered_map<std::string, std::string> post_;   // POST请求参数

    // 常量：默认HTML文件集合和标签映射
    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;


};

























#endif // ~HTTPREQUEST_H