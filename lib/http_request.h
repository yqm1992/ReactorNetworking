#pragma once

#include <string>
#include <list>

namespace networking {

struct RequestHeader {
    RequestHeader(const std::string& key, const std::string& value): key(key), value(value) {}
    std::string key;
    std::string value;
};

enum HttpRequestState: int {
    REQUEST_STATUS,    // 等待解析状态行
    REQUEST_HEADERS,   // 等待解析headers
    REQUEST_BODY,      // 等待解析请求body
    REQUEST_DONE       // 解析完成
};

class HttpRequest {
public:
    friend class HttpLayer;
    friend class HttpConnection;

    //初始化一个request对象
    HttpRequest() {}

    //清除一个request对象
    void Clear();

    //重置一个request对象
    void Reset();

    //给request增加header
    void AddHeader(const std::string& key, const std::string& value);

    //根据key值获取header熟悉
    bool GetHeader(const std::string& key, std::string* value);

    //获得request解析的当前状态
    HttpRequestState CurrentState() { return current_state_; }

    //根据request请求判断是否需要关闭服务器-->客户端单向连接
    int CloseConnection();

    void Display();

private:
    std::string version_;
    std::string method_;
    std::string url_;
    enum HttpRequestState current_state_ = REQUEST_STATUS;
    std::list<RequestHeader> request_headers_;
};

}