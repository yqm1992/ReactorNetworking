#pragma once

#include "common.h"
#include "channel.h"
#include "tcp_connection.h"
#include "tcp_server.h"

namespace networking {

class Acceptor: public Channel {
public:

    friend class TcpServer;

    Acceptor(int listen_port): listen_port_(listen_port), tcp_server_(nullptr) {}

    virtual ~Acceptor() { Close(); }

    bool Init();

    void SetTcpServer(TcpServer* tcp_server) { tcp_server_ = tcp_server; }
    
    // virtual int EventWriteCallback() override;

    virtual int EventReadCallback() override { return HandleConnectionEstablised(); } 

    // static std::shared_ptr<Channel> MakeAcceptorChannel(int listen_port);

    static bool MakeNonblocking(int fd);

    static int GetListenFD(int listen_port);

    virtual int Close() override { return close(fd_); }

protected:

    // TcpConnection中包含的应用类对象，在创建connection的时候调用
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication() { 
        return std::make_shared<TcpApplication>("DefaultTcpApplication"); 
    }

    TcpServer* GetTcpServer() { return tcp_server_; }

    int HandleConnectionEstablised();

    int listen_port_;
    TcpServer* tcp_server_;
} ;

}