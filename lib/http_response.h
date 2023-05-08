#pragma once

#include <string>
#include <list>
#include "buffer.h"

namespace networking {

struct ResponseHeader {
    std::string key;
    std::string value;
};

enum HttpStatusCode: int {
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
};

class HttpResponse {
public:
    friend class HttpLayer;

    HttpResponse() {}

    void EncodeBuffer(Buffer* buffer);

    void Display();

private:
    HttpStatusCode status_code_ = Unknown;
    std::string status_message_;
    std::string content_type_;
    std::string body_;
    std::list<ResponseHeader> response_headers_;
    int response_headers_number_ = 0;
    int keep_connected_ = 0;
};
}


// TcpServer需要设置好回调函数（这里应该通过函数指针的形式，而不是虚函数来实现，每一种）
// 新建TcpConnection的时候，需要设置好回调函数