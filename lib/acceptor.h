#pragma once

#include "common.h"
#include "channel.h"
#include "tcp_server.h"

namespace networking {

class Acceptor: public Channel {
public:

    Acceptor() {}   

    bool Init(TcpServer* tcp_server, int port);
    
    // virtual int EventWriteCallback() override;

    virtual int EventReadCallback() override; 

    static std::shared_ptr<Channel> MakeChannel(TcpServer* tcp_server, int listen_port) {
        auto acceptor = new Acceptor();
        acceptor->Init(tcp_server, listen_port);
        std::shared_ptr<Channel> channel;
        channel.reset(static_cast<Channel*>(acceptor));
        return channel;
    }

    static bool MakeNonblocking(int fd);

    static int GetListenFD(int listen_port);

private:
    TcpServer* GetTcpServer() { return static_cast<TcpServer*>(data_); }

    int listen_port_;
} ;

}