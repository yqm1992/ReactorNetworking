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
    HttpLayer(): TcpApplication("http_layer") {}

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

    HttpRequest http_request_;
    HttpResponse http_response_;
};

class HttpAcceptor: public Acceptor {
public:
    HttpAcceptor(int listen_port): Acceptor(listen_port) {}

    static std::shared_ptr<Channel> MakeHttpAcceptorChannel(int listen_port) {
        auto acceptor = new HttpAcceptor(listen_port);
        if (!acceptor->Init()) {
            yolanda_msgx("failed to init acceptor");
            delete acceptor;
            return nullptr;
        }
        std::shared_ptr<Channel> channel;
        channel.reset(static_cast<Channel*>(acceptor));
        return channel;
    }
    
private:
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication() override { 
        auto http_layer = new HttpLayer();
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(http_layer));
        return tcp_application; 
    }
};

class HttpServer {
public:
    HttpServer(int thread_num, int listen_port): thread_num_(thread_num), listen_port_(listen_port) {}

    bool Start() {
        auto acceptor_channel = HttpAcceptor::MakeHttpAcceptorChannel(listen_port_);
        if (acceptor_channel == nullptr) {
            return false;
        }
        tcp_server_ = std::make_shared<TcpServer>(thread_num_);
        tcp_server_->Start(acceptor_channel);
        return true;
    }

private:
    int thread_num_;
    int listen_port_;
    std::shared_ptr<TcpServer> tcp_server_;
};

}