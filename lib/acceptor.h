#pragma once

#include "common.h"
#include "channel.h"
#include "tcp_connection.h"
#include "tcp_server.h"

namespace networking {

class Acceptor: public Channel {
public:

    friend class TcpServer;

    Acceptor(int listen_port): listen_port_(listen_port) {}

    virtual ~Acceptor() {}

    bool Init();

    void SetTcpServer(TcpServer* tcp_server) { data_ = static_cast<void *>(tcp_server); }
    
    // virtual int EventWriteCallback() override;

    virtual int EventReadCallback() override { return HandleConnectionEstablised(); } 

    // static std::shared_ptr<Channel> MakeAcceptorChannel(int listen_port);

    static bool MakeNonblocking(int fd);

    static int GetListenFD(int listen_port);

protected:

    std::shared_ptr<Channel> MakeTcpConnectionChannel(int connected_fd, EventLoop *event_loop);

    // TcpConnection中包含的应用类对象，在创建connection的时候调用
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication(TcpConnection * connection) { 
        return std::make_shared<TcpApplication>(connection, "DefaultTcpApplication"); 
    }

    TcpServer* GetTcpServer() { return static_cast<TcpServer*>(data_); }

    int HandleConnectionEstablised();

    int listen_port_;
} ;

}