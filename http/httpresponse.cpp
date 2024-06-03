#include "httpresponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_TO_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_TO_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};


HttpResponse::HttpResponse()
    : code_(-1), isKeepAlive_(false), mmFile_(nullptr), srcDir_(""), path_("")
{
    memset(&mmFileStat_, 0, sizeof(mmFileStat_));
};
HttpResponse::~HttpResponse()
{
    unmapFile();
};

void HttpResponse::init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1)
{
    assert(!srcDir.empty());
    if (mmFile_) unmapFile();
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    srcDir_ = srcDir;
    path_ = path;
    mmFile_ = nullptr;
    memset(&mmFileStat_, 0, sizeof(mmFileStat_));
};
void HttpResponse::handleResponse(Buffer& buffer)
{
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0
        || S_ISDIR(mmFileStat_.st_mode)) // 检查文件是否存在及是否是一个文件而非目录
        code_ = 404;
    else if (mmFileStat_.st_mode & S_IROTH == 0) // 检查文件权限是否允许其他用户读取
        code_ = 403;
    else if (code_ == -1) // 之前没有设置过code_
        code_ = 200;

    errorHtml_();
    addStateLineToBuffer_(buffer);
    addHeaderToBuffer_(buffer);
    addContentToBuffer_(buffer);

}; // 生成HTTP响应，填充缓冲区
void HttpResponse::unmapFile()
{
    if (mmFile_)
    {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
};
char* HttpResponse::file()
{
    return mmFile_;
};
size_t HttpResponse::fileLength() const
{
    return mmFileStat_.st_size;
};
void HttpResponse::handleErrorContent(Buffer& buffer, std::string errorMessage)
{
    std::string body;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(code_) + " : " + CODE_TO_STATUS.find(code_)->second;
    body += "<hr><em>HttpServer</em></body></html>";

    buffer.append("/r/n");
    buffer.append(body);
};

void HttpResponse::addStateLineToBuffer_(Buffer& buffer)
{
    std::string status;
    if (CODE_TO_STATUS.count(code_) == 1)
    {
        status = CODE_TO_STATUS.find(code_)->second;
    }
    else {
        code_ = 400;
        status = CODE_TO_STATUS.find(400)->second;
    }
    buffer.append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
};
void HttpResponse::addHeaderToBuffer_(Buffer& buffer)
{
    std::string header;
    header += "Connection: ";
    if (isKeepAlive_)
    {
        header += "keep-alive\r\n";
        header += "keep-alive: max=6, timeout=120\r\n"; // 6个连接 120秒超时
    }
    else {
        header += "close\r\n";
    }
    header += "Content-Type: " + getFileType_() + "; charset=UTF-8" + "\r\n";
    header += "Content-Length: " + std::to_string(mmFileStat_.st_size) + "\r\n";
    header += "\r\n";

    buffer.append(header);
};
void HttpResponse::addContentToBuffer_(Buffer& buffer)
{
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcFd < 0)
    {
        handleErrorContent(buffer, "File Not Found1");
        return;
    }

    // 内存映射
    mmFile_ = static_cast<char*>(mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0));
    close(srcFd);

    if (mmFile_ == MAP_FAILED)
    {
        mmFile_ = nullptr;
        handleErrorContent(buffer, "File Not Found!");
        return;
    }

    buffer.append(mmFile_, mmFileStat_.st_size);

    /*
    std::ifstream inputFile(srcDir_ + path_);
    if (!inputFile) {
        handleErrorContent(buffer, "File Not Found1");
        return;
    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf(); // 将文件内容读入字符串流
    std::string fileContent = buffer.str(); // 获取字符串流中的内容
    buffer.append(fileContent);
     */
};

void HttpResponse::errorHtml_()
{
    if (CODE_TO_PATH.count(code_) > 0)
    {
        path_ = CODE_TO_PATH.find(code_)->second;
        stat((srcDir_ + path_).c_str(), &mmFileStat_);
    }
}; // 生成错误页面的HTML内容
std::string HttpResponse::getFileType_()
{
    std::string::size_type index = path_.find_last_of('.');
    if (index == std::string::npos)
        return "text/plain";

    std::string suffix = path_.substr(index);
    if (SUFFIX_TYPE.count(suffix) > 0)
        return SUFFIX_TYPE.find(suffix)->second;

    return "text/plain";
};