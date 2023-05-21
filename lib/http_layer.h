#pragma once

#include <memory>
#include "tcp_connection.h"
#include "acceptor.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

#include <iostream>

namespace networking {

// class HttpLayer: public TcpApplication {
// public:
//     HttpLayer(): TcpApplication("http_layer") {}

//     ~HttpLayer() {std::cout << "~HttpLayer()" << std::endl;}

//     int ConnectionCompletedCallBack() override;
//     int ConnectionClosedCallBack() override;
//     int MessageCallBack() override;
//     int WriteCompletedCallBack() override;

//     static char* FindCRLF(char* s, int size);

// private:
//     static const char* FindPattern(const char *start, int size, const char* target, int target_size);
//     static int OnHttpRequest(const HttpRequest& http_request, HttpResponse* http_response);

//     int ProcessStatusLine(const char *start, const char *end);
//     int ParseHttpRequest();

//     HttpRequest http_request_;
//     HttpResponse http_response_;
// };

class HttpConnection: public TcpConnection {
public:
    HttpConnection(int fd, WorkThread* worker = nullptr): TcpConnection(fd, "http_connection", worker) {}

    ~HttpConnection() { std::cout << "~HttpConnection()" << std::endl; }

    int ConnectionCompletedCallBack() override;
    int ConnectionClosedCallBack() override;
    int MessageCallBack(std::shared_ptr<Buffer> message_buffer) override;
    int WriteCompletedCallBack() override;

    static char* FindCRLF(char* s, int size);

private:
    static const char* FindPattern(const char *start, int size, const char* target, int target_size);
    static int OnHttpRequest(const HttpRequest& http_request, HttpResponse* http_response);

    void ApplicationLayerProcess();
    int ProcessStatusLine(const char *start, const char *end);
    int ParseHttpRequest();

    Buffer http_buffer_;
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

    std::shared_ptr<Channel> MakeTcpConnectionChannel(int fd, WorkThread* worker) override {
        std::shared_ptr<Channel> channel;
        TcpConnection* tcp_connection = static_cast<TcpConnection*>(new HttpConnection(fd, worker));
        channel.reset(static_cast<Channel*>(tcp_connection));
        return channel;
    }
};

class HttpServer {
public:
    HttpServer(int thread_num, int worker_num, int listen_port): loop_num_(thread_num), worker_num_(worker_num), listen_port_(listen_port) {}

    bool Start() {
        auto acceptor_channel = HttpAcceptor::MakeHttpAcceptorChannel(listen_port_);
        if (acceptor_channel == nullptr) {
            return false;
        }
        tcp_server_ = std::make_shared<TcpServer>(loop_num_, worker_num_);
        tcp_server_->Start(acceptor_channel);
        return true;
    }

private:
    int loop_num_;
    int worker_num_;
    int listen_port_;
    std::shared_ptr<TcpServer> tcp_server_;
};

}