#pragma once

#include <memory>
#include "tcp_connection.h"
#include "tcp_server.h"

#include <iostream>

namespace networking {

class HttpApplication: public TcpApplication {
public:
    HttpApplication(): TcpApplication("http_application") {}

    ~HttpApplication() { std::cout << "~HttpApplication()" << std::endl; }

    static std::shared_ptr<TcpApplication> MakeTcpApplication() {
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(new HttpApplication()));
        return tcp_application;
    }


    int ConnectionCompletedCallBack(TcpConnection* connection) override;
    int ConnectionClosedCallBack(TcpConnection* connection) override;
    int MessageCallBack(TcpConnection* connection) override;
    int WriteCompletedCallBack(TcpConnection* connection) override;
    
};

class HttpServer {
public:
    HttpServer(int thread_num, int listen_port) {
        // 传入 std::shared_ptr<TcpApplication> 会被提前析构掉
        tcp_server_ = std::make_shared<TcpServer>(thread_num, listen_port, &HTTP_APPLICATION);
    }

    ~HttpServer() {}

    void Start() { tcp_server_->Start(); }
    
private:
    void ParseHttpRequest();

    std::shared_ptr<TcpServer> tcp_server_;
    static HttpApplication HTTP_APPLICATION;
};

}