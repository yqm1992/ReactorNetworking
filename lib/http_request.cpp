#include "http_request.h"
#include "common.h"

#define INIT_REQUEST_HEADER_SIZE 128

namespace networking {

const char *HTTP10 = "HTTP/1.0";
const char *HTTP11 = "HTTP/1.1";
const char *KEEP_ALIVE = "Keep-Alive";
const char *CLOSE = "close";

//清除一个request对象
void HttpRequest::Clear() {
    request_headers_.clear();
}

//重置一个request对象
void HttpRequest::Reset() {
    method_.clear();
    current_state_ = REQUEST_STATUS;
    version_.clear();
    url_.clear();
}

//给request增加header
void HttpRequest::AddHeader(const std::string& key, const std::string& value) {
    request_headers_.emplace_back(RequestHeader(key, value));
}

//根据key值获取header熟悉
bool HttpRequest::GetHeader(const std::string& key, std::string* value) {
    value->clear();
    for (auto& header: request_headers_) {
        if (header.key == key) {
            *value = header.value;
            return true;
        }
    }
    return false;
}

//根据request请求判断是否需要关闭服务器-->客户端单向连接
int HttpRequest::CloseConnection() {
    std::string connection;
    if (!GetHeader("Connection", &connection) && connection == CLOSE) {
        return 1;
    }
    if (version_ != "" &&
        version_ == HTTP10 &&
        connection == KEEP_ALIVE) {
            return 1;
        }
    return 0;
}


}