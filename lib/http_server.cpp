#include "http_server.h"
#include "assert.h"

#include <cstring>

#include <iostream>

namespace networking {

HttpLayerFactory HttpServer::HTTP_LAYER_FACTORY;

int HttpLayer::ConnectionCompletedCallBack() {
    yolanda_msgx("connection completed");
    http_request_ = std::make_shared<HttpRequest>();
    return 0;
}

int HttpLayer::ConnectionClosedCallBack() {
    yolanda_msgx("ConnectionClosedCallBack");
    return 0;
}

int HttpLayer::MessageCallBack() {
    yolanda_msgx("MessageCallBack");
    std::cout << connection_->GetInputBuffer()->ReadStart() << std::endl;
    connection_->SendData("Hello world\n", 13);
    connection_->Shutdown();
    return 0;
}

// // buffer是框架构建好的，并且已经收到部分数据的情况下
// // 注意这里可能没有收到全部数据，所以要处理数据不够的情形
// int HttpLayer::MessageCallBack() {
//     yolanda_msgx("get message from %s", connection_->GetDescription());

//     struct http_request *httpRequest = (struct http_request *) tcpConnection->request;
//     struct http_server *httpServer = (struct http_server *) tcpConnection->data;

//     if (parse_http_request(input, httpRequest) == 0) {
//         char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
//         tcp_connection_send_data(tcpConnection, error_response, sizeof(error_response));
//         tcp_connection_shutdown(tcpConnection);
//     }

//     //处理完了所有的request数据，接下来进行编码和发送
//     if (http_request_current_state(httpRequest) == REQUEST_DONE) {
//         struct http_response *httpResponse = http_response_new();

//         //httpServer暴露的requestCallback回调
//         if (httpServer->requestCallback != NULL) {
//             httpServer->requestCallback(httpRequest, httpResponse);
//         }
//         struct buffer *buffer = buffer_new();
//         http_response_encode_buffer(httpResponse, buffer);
//         tcp_connection_send_buffer(tcpConnection, buffer);

//         if (http_request_close_connection(httpRequest)) {
//             tcp_connection_shutdown(tcpConnection);
//         }
// 	http_request_reset(httpRequest);
//     }
// }

int HttpLayer::WriteCompletedCallBack() {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}

const char* HttpLayer::Find(const char *start, int size, const char* target, int target_size) {
    return static_cast<const char *>(memmem(static_cast<const void *>(start), size, static_cast<const void *>(target), target_size) );
}

int HttpLayer::ProcessStatusLine(const char *start, const char *end) {
    int size = end - start;
    //method
    const char *space = Find(start, end - start, " ", 1);
    assert(space != nullptr);
    int method_size = space - start;
    http_request_->method_ = std::string(start, method_size);

    //url
    start = space + 1;
    space = Find(start, end - start, " ", 1);
    assert(space != nullptr);
    int url_size = space - start;
    http_request_->url_ = std::string(start, method_size);

    //version
    start = space + 1;
    http_request_->version_ = std::string(start, end-start);
    return size;
}

int HttpLayer::ParseHttpRequest() {
    auto input_buffer = connection_->GetInputBuffer();
    
    int ok = 1;
    auto& current_state = http_request_->current_state_;
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
                const char *colon = Find(start, request_line_size, ": ", 2);
                if (colon != nullptr) {
                    std::string key(start, colon - start);
                    std::string value(colon + 2, crlf - colon - 2);
                    http_request_->AddHeader(key, value);
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