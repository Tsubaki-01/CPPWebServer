#ifndef HTTPRESPONSE_H

#define HTTPRESPONSE_H

#include "../buffer/buffer.h"

#include <string>
#include <unordered_map>
#include <cassert>
#include <memory>
#include <fcntl.h> // open 
#include <unistd.h> // close
#include <sys/stat.h>
#include <sys/mman.h>


class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void handleResponse(Buffer& buffer); // 生成HTTP响应，填充缓冲区
    void unmapFile(); // 解除内存映射
    char* file();
    size_t fileLength() const;
    void handleErrorContent(Buffer& buffer, std::string errorMessage);
    int getCode() const { return code_; };

private:
    void addStateLineToBuffer_(Buffer& buff);
    void addHeaderToBuffer_(Buffer& buff);
    void addContentToBuffer_(Buffer& buff);

    void errorHtml_(); // 生成错误页面的HTML内容
    std::string getFileType_();

    int code_; // HTTP响应状态码
    bool isKeepAlive_;

    std::string srcDir_; // 服务器资源的根目录
    std::string path_; // 请求的资源路径

    char* mmFile_; // 指向内存映射文件的指针
    struct stat mmFileStat_ = { 0 }; // 内存映射文件的状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE; // 文件后缀名到MIME类型的映射
    static const std::unordered_map<int, std::string> CODE_TO_STATUS; // 状态码到状态描述的映射
    static const std::unordered_map<int, std::string> ERRORCODE_TO_PATH; // 错误状态码到错误页面路径的映射
};




#endif // ~HTTPRESPONSE_H
