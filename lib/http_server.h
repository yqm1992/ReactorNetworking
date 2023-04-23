#pragma once

#include <memory>
#include "tcp_connection.h"
#include "tcp_server.h"

#include <iostream>

namespace networking {

class HttpApplication: public TcpApplication {
public:
    HttpApplication(TcpConnection* connection): TcpApplication(connection, "http_application") {}

    ~HttpApplication() {std::cout << "~HttpApplication()" << std::endl;}

    int ConnectionCompletedCallBack() override;
    int ConnectionClosedCallBack() override;
    int MessageCallBack() override;
    int WriteCompletedCallBack() override;
    
};

class HttpApplicationFactory: public TcpApplicationFactory {
public:
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication(TcpConnection* connection) {
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(new HttpApplication(connection)));
        return tcp_application;
    }
    ~HttpApplicationFactory() { std::cout << "~HttpApplicationFactory()" << std::endl; }
};

class HttpServer {
public:
    HttpServer(int thread_num, int listen_port) {
        // 传入 std::shared_ptr<TcpApplication> 会被提前析构掉
        tcp_server_ = std::make_shared<TcpServer>(thread_num, listen_port, &HTTP_APPLICATION_FACTORY);
    }

    ~HttpServer() {}

    void Start() { tcp_server_->Start(); }
    
private:
    void ParseHttpRequest();

    std::shared_ptr<TcpServer> tcp_server_;
    static HttpApplicationFactory HTTP_APPLICATION_FACTORY; // 构建HttpApplication的方法
};

}