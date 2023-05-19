#include "http_layer.h"
#include "assert.h"

#include <cstring>

#include <iostream>

namespace networking {

const char *CRLF = "\r\n";

char* HttpConnection::FindCRLF(char* s, int size) {
    char *crlf = (char *)memmem(s, size, CRLF, 2);
    return crlf;
}

int HttpConnection::ConnectionCompletedCallBack() {
    yolanda_msgx("connection completed (HTTP)");
    return 0;
}

int HttpConnection::ConnectionClosedCallBack() {
    yolanda_msgx("ConnectionClosedCallBack (HTTP)");
    return 0;
}

//数据读到buffer之后的callback
int HttpConnection::OnHttpRequest(const HttpRequest& http_request, HttpResponse* http_response) {
    auto& url = http_request.url_;
    const char* question = FindPattern(url.c_str(), url.size(), "?", 1);
    std::string path;
    if (question != nullptr) {
        path = url.substr(0, question-url.c_str()); 
    } else {
        path = url;
    }
    if (path == "/") {
        http_response->status_code_ = HttpStatusCode::OK;
        http_response->status_message_ = "ok";
        http_response->content_type_ = "text/html";
        http_response->body_ = "<html><head><title>This is network programming</title></head><body><h1>Hello, network programming</h1></body></html>";
    } else if (path == "/network") {

        http_response->status_code_ = HttpStatusCode::OK;
        http_response->status_message_ = "OK";
        http_response->content_type_ = "text/plain";
        http_response->body_ = "hello, network programming";
    } else {
        http_response->status_code_ = HttpStatusCode::NotFound;
        http_response->status_message_ = "Not Found";
        http_response->keep_connected_ = 1;
    }
    return 0;
}

int HttpConnection::MessageCallBack(std::shared_ptr<Buffer> message_buffer) {
    yolanda_msgx("get message from %s", GetDescription().c_str());
    http_buffer_.AppendBuffer(*message_buffer.get());
    ApplicationLayerProcess();
}

// buffer是框架构建好的，并且已经收到部分数据的情况下
// 注意这里可能没有收到全部数据，所以要处理数据不够的情形
void HttpConnection::ApplicationLayerProcess() {
    yolanda_msgx("get message from %s", GetDescription().c_str());
    // std::cout << connection_->GetInputBuffer()->ReadStartPos() << std::endl;

    if (ParseHttpRequest() == 0) {
        const char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        SendData(error_response, sizeof(error_response));
        Shutdown();
    }

    //处理完了所有的request数据，接下来进行编码和发送
    if (http_request_.CurrentState() == REQUEST_DONE) {
        http_request_.Display();
        OnHttpRequest(http_request_, &http_response_);
        Buffer buffer;
        http_response_.EncodeBuffer(&buffer);
        http_response_.Display();
        SendBuffer(buffer);
        if (http_request_.CloseConnection()) {
            Shutdown();
        }
        http_request_.Reset();
    }
}

int HttpConnection::WriteCompletedCallBack() {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}

const char* HttpConnection::FindPattern(const char *start, int size, const char* target, int target_size) {
    return static_cast<const char *>(memmem(static_cast<const void *>(start), size, static_cast<const void *>(target), target_size) );
}

int HttpConnection::ProcessStatusLine(const char *start, const char *end) {
    int size = end - start;
    //method
    const char *space = FindPattern(start, end - start, " ", 1);
    assert(space != nullptr);
    int method_size = space - start;
    http_request_.method_ = std::string(start, method_size);

    //url
    start = space + 1;
    space = FindPattern(start, end - start, " ", 1);
    assert(space != nullptr);
    int url_size = space - start;
    http_request_.url_ = std::string(start, url_size);

    //version
    start = space + 1;
    http_request_.version_ = std::string(start, end-start);
    return size;
}

int HttpConnection::ParseHttpRequest() {    
    int ok = 1;
    auto& current_state = http_request_.current_state_;
    char* start = http_buffer_.ReadStartPos();
    char* end = http_buffer_.ReadStartPos() + http_buffer_.ReadableSize();
    
    while (current_state != REQUEST_DONE) {
        if (current_state == REQUEST_STATUS) {
            // const char *crlf = http_buffer->FindCRLF();
            const char *crlf = FindCRLF(start, end - start);
            if (crlf) {
                int request_line_size = ProcessStatusLine(start, crlf);
                if (request_line_size) {
                    start += (request_line_size+2);
                    current_state = REQUEST_HEADERS;
                }
            }
        } else if (current_state == REQUEST_HEADERS) {
            // const char *crlf = http_buffer_.FindCRLF();
            const char *crlf = FindCRLF(start, end - start);
            if (crlf) {
                /**
                 *    <start>-------<colon>:-------<crlf>
                 */
                int request_line_size = crlf - start;
                const char *colon = FindPattern(start, request_line_size, ": ", 2);
                if (colon != nullptr) {
                    std::string key(start, colon - start);
                    std::string value(colon + 2, crlf - colon - 2);
                    http_request_.AddHeader(key, value);
                    start += (request_line_size+2);
                } else {
                    //读到这里说明:没找到，就说明这个是最后一行
                    start += 2; // CRLF size
                    current_state = REQUEST_DONE;
                }
            }
        }
    }
    // 解析成功，丢弃掉消费的部分
    http_buffer_.DiscardReadableData(start - http_buffer_.ReadStartPos());
    return ok;
}

}
