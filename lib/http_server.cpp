#include "http_server.h"
#include "assert.h"

#include <cstring>

#include <iostream>

namespace networking {

HttpLayerFactory HttpServer::HTTP_LAYER_FACTORY;

int HttpLayer::ConnectionCompletedCallBack() {
    yolanda_msgx("connection completed");
    // http_request_ = std::make_shared<HttpRequest>();
    return 0;
}

int HttpLayer::ConnectionClosedCallBack() {
    yolanda_msgx("ConnectionClosedCallBack");
    return 0;
}

//数据读到buffer之后的callback
int HttpLayer::OnHttpRequest(const HttpRequest& http_request, HttpResponse* http_response) {
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

// int HttpLayer::MessageCallBack() {
//     yolanda_msgx("MessageCallBack");
//     std::cout << connection_->GetInputBuffer()->ReadStart() << std::endl;
//     connection_->SendData("Hello world\n", 13);
//     connection_->Shutdown();
//     return 0;
// }

// buffer是框架构建好的，并且已经收到部分数据的情况下
// 注意这里可能没有收到全部数据，所以要处理数据不够的情形
int HttpLayer::MessageCallBack() {
    yolanda_msgx("get message from %s", connection_->GetDescription().c_str());
    std::cout << connection_->GetInputBuffer()->ReadStart() << std::endl;

    if (ParseHttpRequest() == 0) {
        const char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        connection_->SendData(error_response, sizeof(error_response));
        connection_->Shutdown();
    }

    //处理完了所有的request数据，接下来进行编码和发送
    if (http_request_.CurrentState() == REQUEST_DONE) {
        OnHttpRequest(http_request_, &http_response_);
        Buffer buffer;
        http_response_.EncodeBuffer(&buffer);
        connection_->SendBuffer(buffer);
        if (http_request_.CloseConnection()) {
            connection_->Shutdown();
        }
        http_request_.Reset();
    }
}

int HttpLayer::WriteCompletedCallBack() {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}

const char* HttpLayer::FindPattern(const char *start, int size, const char* target, int target_size) {
    return static_cast<const char *>(memmem(static_cast<const void *>(start), size, static_cast<const void *>(target), target_size) );
}

int HttpLayer::ProcessStatusLine(const char *start, const char *end) {
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

int HttpLayer::ParseHttpRequest() {
    auto input_buffer = connection_->GetInputBuffer();
    
    int ok = 1;
    auto& current_state = http_request_.current_state_;
    while (current_state != REQUEST_DONE) {
        if (current_state == REQUEST_STATUS) {
            const char *crlf = input_buffer->FindCRLF();
            if (crlf) {
                int request_line_size = ProcessStatusLine(input_buffer->ReadStart(), crlf);
                if (request_line_size) {
                    input_buffer->DiscardReadableData(request_line_size);  // request line size
                    input_buffer->DiscardReadableData(2);  //CRLF size
                    current_state = REQUEST_HEADERS;
                }
            }
        } else if (current_state == REQUEST_HEADERS) {
            const char *crlf = input_buffer->FindCRLF();
            if (crlf) {
                /**
                 *    <start>-------<colon>:-------<crlf>
                 */
                const char* start = input_buffer->ReadStart();
                int request_line_size = crlf - start;
                const char *colon = FindPattern(start, request_line_size, ": ", 2);
                if (colon != nullptr) {
                    std::string key(start, colon - start);
                    std::string value(colon + 2, crlf - colon - 2);
                    http_request_.AddHeader(key, value);
                    input_buffer->DiscardReadableData(request_line_size);  //request line size
                    input_buffer->DiscardReadableData(2);  //CRLF size
                } else {
                    //读到这里说明:没找到，就说明这个是最后一行
                    input_buffer->DiscardReadableData(2);  //CRLF size
                    current_state = REQUEST_DONE;
                }
            }
        }
    }
    return ok;
}

}
