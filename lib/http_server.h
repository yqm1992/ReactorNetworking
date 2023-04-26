#pragma once

#include <memory>
#include "tcp_connection.h"
#include "acceptor.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

#include <iostream>

namespace networking {

class HttpLayer: public TcpApplication {
public:
    HttpLayer(TcpConnection* connection): TcpApplication(connection, "http_layer") {}

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

class HttpAcceptor: public Acceptor {
public:
    HttpAcceptor(int listen_port): Acceptor(listen_port) {}

    static std::shared_ptr<Channel> MakeHttpAcceptorChannel(int listen_port) {
        auto acceptor = new HttpAcceptor(listen_port);
        acceptor->Init();
        std::shared_ptr<Channel> channel;
        channel.reset(static_cast<Channel*>(acceptor));
        return channel;
    }
    
private:
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication(TcpConnection * connection) override { 
        auto http_layer = new HttpLayer(connection);
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(http_layer));
        return tcp_application; 
    }
};

class HttpServer {
public:
    HttpServer(int thread_num) {
        tcp_server_ = std::make_shared<TcpServer>(thread_num);
    }

    ~HttpServer() {}

    void Start(std::shared_ptr<Channel> acceptor_channel) { tcp_server_->Start(acceptor_channel); }
    
private:

    std::shared_ptr<TcpServer> tcp_server_;
};

}