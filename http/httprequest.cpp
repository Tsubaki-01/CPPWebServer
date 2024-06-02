#include "httprequest.h"

/*
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
 */

 // private

// 常量：默认HTML文件集合和标签映射
const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML
{
"/index", "/register", "/login","/welcome", "/video", "/picture"
};
const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG
{
{"/register.html", 0}, {"/login.html", 1}
};


// 解析请求行
bool HttpRequest::parseRequestLine(const std::string& line)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if (std::regex_match(line, match, patten))
    {
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state_ = HEADERS;
        return true;
    }
    return false;
};
// 解析请求头
void HttpRequest::parseHeader(const std::string& line)
{
    std::regex patten("^([^:])*: ?(.*)$");
    std::smatch match;
    if (std::regex_match(line, match, patten))
    {
        header_[match[1]] = match[2];
    }
    else state_ = BODY;
};
// 解析请求体
void HttpRequest::parseBody(const std::string& line)
{
    body_ = line;
    parsePost();
    state_ = FINISH;
    // 写日志
};
// 解析请求路径
void HttpRequest::parsePath()
{
    if (path_ == "/")
        path_ = "/index.heml";
    else {
        if (DEFAULT_HTML.count(path_) > 0)
            path_ += ".html";
    }
};
// 解析POST请求数据
void HttpRequest::parsePost()
{
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded")
    {
        parseFromUrlencoded();
        if (DEFAULT_HTML_TAG.count(path_))
        {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            // 写日志
            if (tag == 0 || tag == 1)
            {
                bool isLogin = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], isLogin))
                {
                    path_ = "/welcome.html";
                }
                else path_ = "/error.html";
            }
        }
    }
};

// 从URL编码格式解析数据
void HttpRequest::parseFromUrlencoded()
{
    if (body_.size() == 0)
        return;

    std::string key, value;
    int num = 0;
    int size = body_.size();
    int right = 0, left = 0;

    for (;right < size;right++)
    {
        char c = body_[right];

        switch (c)
        {
        case '=':
            key = body_.substr(left, right - left);
            left = right + 1;
            break;
        case '+':
            body_[right] = ' ';
            break;
        case '%':
            int num = std::stoi(body_.substr(right + 1, 2), nullptr, 16);
            body_[right] = num;
            body_.erase(body_.begin() + right + 1);
            body_.erase(body_.begin() + right + 1);
            right -= 2;
            break;
        case '&':
            value = body_.substr(left, right - left);
            post_[key] = value;
            left = right + 1;
            // 写日志
            break;
        default:
            break;
        }

    }
    assert(right >= left);

    if (post_.count(key) <= 0 && left < right)
        post_[key] = body_.substr(left, right - left);


};

// 验证用户信息
bool HttpRequest::userVerify(const std::string& name, const std::string& pwd, bool isLogin)
{
    if (name.empty() || pwd.empty()) return false;
    // 写日志

    MYSQL* sql;
    sqlConnRAII tempRAII(sql, SqlConnPool::instance());
    assert(sql);

    bool flag = false;
    // unsigned int j = 0;
    char order[256] = { 0 };
    // MYSQL_FIELD* fields = nullptr;
    MYSQL_RES* res = nullptr;


    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    // 写日志

    if (!mysql_query(sql, order)) // 查询失败
    {
        // 写日志
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    // j = mysql_num_fields(res);
    // fields = mysql_fetch_field(res);

    while (MYSQL_ROW row = mysql_fetch_row(res))
    {
        // 写日志
        std::string password(row[1]);

        if (isLogin)
            if (pwd == password) // 登录成功
                flag = true;
            else {
                flag = false; // 密码错误
                // 写日志
            }
        else {
            flag = false; // 注册时用户名重复
            // 写日志
        }
    }
    mysql_free_result(res);

    if (!isLogin)
    {
        // 写日志
        memset(order, 0, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        // 写日志
        if (!mysql_query(sql, order)) // 插入失败
        {
            // 写日志
            flag = false;
        }
    }

    // 写日志
    return flag;

};

// public
void HttpRequest::init()
{
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
};
// 解析HTTP请求
bool HttpRequest::parser(Buffer& buffer)
{
    const char CRLF[] = "\r\n";
    if (buffer.readableBytes() <= 0)
        return false;
    while (buffer.readableBytes() && state_ != FINISH)
    {
        const char* END_OF_LINE = std::search(buffer.curPtr(), buffer.writePtr(), CRLF, CRLF + 2);
        std::string lineContent = std::string(buffer.curPtr(), END_OF_LINE);

        switch (state_)
        {
        case REQUEST_LINE:
            if (parseRequestLine(lineContent) == false)
                return false;
            else parsePath();
            break;
        case HEADERS:
            parseHeader(lineContent);
            if (buffer.readableBytes() <= 2) // 跳过Get的空行
                state_ = FINISH;
            break;
        case BODY:
            parseBody(lineContent);
            break;
        default:
            break;
        }
        if (END_OF_LINE == buffer.writePtr())
            break;
        buffer.retrieveUntil(END_OF_LINE + 2);
    }
    // 写日志

    return true;
};

// 获取请求路径
std::string HttpRequest::path() const
{
    return path_;
};
std::string& HttpRequest::path()
{
    return path_;
};

// 获取请求方法（如GET、POST）
std::string HttpRequest::method() const
{
    return method_;
};
// 获取HTTP版本（如HTTP/1.1）
std::string HttpRequest::version() const
{
    return version_;
};

// 获取POST请求参数（根据key获取value）
std::string HttpRequest::getPost(const std::string& key) const
{
    assert(key != "");

    if (post_.count(key) > 0)
        return post_.at(key); // const 函数里面不能使用[]访问，因为[]是非const函数
    return "";
};
std::string HttpRequest::getPost(const char* key) const
{
    assert(key != nullptr);
    if (post_.count(key) > 0)
        return post_.at(key); // 隐式转换
    return "";
};

// 判断是否为保持连接（keep-alive）
bool HttpRequest::isKeepAlive() const
{
    if (header_.count("Connection") == 1)
    {
        return header_.find("Connection")->second == "Keep-Alive" && version_ == "1.1";
    }
    return false;
};