#pragma once

#include <memory>
#include "tcp_connection.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

#include <iostream>

namespace networking {

class HttpLayer: public TcpApplicationLayer {
public:
    HttpLayer(TcpConnection* connection): TcpApplicationLayer(connection, "http_layer") {}

    ~HttpLayer() {std::cout << "~HttpLayer()" << std::endl;}

    int ConnectionCompletedCallBack() override;
    int ConnectionClosedCallBack() override;
    int MessageCallBack() override;
    int WriteCompletedCallBack() override;
private:
    static const char* FindPattern(const char *start, int size, const char* target, int target_size);
    static int OnHttpRequest(const HttpRequest& http_request, HttpResponse* http_response);

    int ProcessStatusLine(const char *start, const char *end);
    int ParseHttpRequest();

    // std::shared_ptr<HttpRequest> http_request_;
    // std::shared_ptr<HttpResponse> http_response_;

    HttpRequest http_request_;
    HttpResponse http_response_;
};

class HttpLayerFactory: public TcpApplicationLayerFactory {
public:
    virtual std::shared_ptr<TcpApplicationLayer> MakeTcpApplicationLayer(TcpConnection* connection) {
        std::shared_ptr<TcpApplicationLayer> tcp_application;
        tcp_application.reset(static_cast<TcpApplicationLayer*>(new HttpLayer(connection)));
        return tcp_application;
    }
    ~HttpLayerFactory() { std::cout << "~HttpLayerFactory()" << std::endl; }
};

class HttpServer {
public:
    HttpServer(int thread_num, int listen_port) {
        // 传入 std::shared_ptr<TcpApplicationLayer> 会被提前析构掉
        tcp_server_ = std::make_shared<TcpServer>(thread_num, listen_port, &HTTP_LAYER_FACTORY);
    }

    ~HttpServer() {}

    void Start() { tcp_server_->Start(); }
    
private:

    std::shared_ptr<TcpServer> tcp_server_;
    static HttpLayerFactory HTTP_LAYER_FACTORY; // 构建HttpLayer的方法
};

}