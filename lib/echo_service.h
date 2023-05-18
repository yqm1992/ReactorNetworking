#pragma once

#include <iostream>

#include "tcp_client.h"
#include "acceptor.h"


namespace networking {
class EchoClientLayer: public networking::TcpApplication {
public:
    EchoClientLayer(): TcpApplication("echo_client_layer") {}

    ~EchoClientLayer() { std::cout << "~EchoClientLayer" << std::endl; }

    static std::string GetInput() {
        std::string str;
        std::cin >> str;
        return str;
    }

    static std::shared_ptr<TcpApplication> MakeTcpApplication() {
        auto echo_client_layer = new EchoClientLayer();
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(echo_client_layer));
        return tcp_application; 
    }

    void Wait() { sync_cond_.Wait(); }

    virtual int ConnectionCompletedCallBack() override { return 0;}

    virtual int ConnectionClosedCallBack() override {
        std::cout << "EchoClientLayer::ConnectionClosedCallBack" << std::endl;
        sync_cond_.Notify();
        return 0;
    }

    virtual int MessageCallBack() override {
        auto buffer = connection_->GetInputBuffer();
        std::cout << buffer->ReadStartPos() << std::endl;
        buffer->DiscardReadableData(buffer->ReadableSize());
        std::string str = GetInput();
        if (str == "exit") {
            connection_->MarkRecycle();
            return 0;
        }
        return connection_->SendData(str.c_str(), str.size());
    }
    virtual int WriteCompletedCallBack() override { return 0; }

private:
    SyncCond sync_cond_;
};

class EchoClient: public TcpClient {
public:
    EchoClient(const std::string& server_addr, int port): TcpClient(server_addr, port, EchoClientLayer::MakeTcpApplication()) {}

    void WaitExit() {
        auto tcp_application = GetTcpApplication();
        static_cast<EchoClientLayer*>(tcp_application.get())->Wait();
    }
};


class EchoServerLayer: public networking::TcpApplication {
public:
    EchoServerLayer(): TcpApplication("echo_server_layer") {}

    ~EchoServerLayer() {}

    static std::shared_ptr<TcpApplication> MakeTcpApplication() {
        auto echo_server_layer = new EchoServerLayer();
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(echo_server_layer));
        return tcp_application; 
    }

    virtual int ConnectionCompletedCallBack() override {
        connection_->SendData("welcome!");
        return 0;
    }

    virtual int ConnectionClosedCallBack() override { return 0; }

    virtual int MessageCallBack() override {
        auto buffer = connection_->GetInputBuffer();
        connection_->SendData("hello: ");
        connection_->SendData(buffer->ReadStartPos(), buffer->ReadableSize());
        buffer->DiscardReadableData(buffer->ReadableSize());
    }

    virtual int WriteCompletedCallBack() override { return 0; }
};

class EchoAcceptor: public Acceptor {
public:
    EchoAcceptor(int listen_port): Acceptor(listen_port) {}

    static std::shared_ptr<Channel> MakeHttpAcceptorChannel(int listen_port) {
        auto acceptor = new EchoAcceptor(listen_port);
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
        auto http_layer = new EchoServerLayer();
        std::shared_ptr<TcpApplication> tcp_application;
        tcp_application.reset(static_cast<TcpApplication*>(http_layer));
        return tcp_application; 
    }
};

class EchoServer {
public:
    EchoServer(int thread_num, int listen_port): thread_num_(thread_num), listen_port_(listen_port) {}

    bool Start() {
        auto acceptor_channel = EchoAcceptor::MakeHttpAcceptorChannel(listen_port_);
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
