#pragma once

#include "common.h"
#include "channel.h"
#include "tcp_server.h"

namespace networking {

class Acceptor: public Channel {
public:

    Acceptor(TcpServer* thread_pool, int port): thread_pool_(thread_pool), listen_port_(port) {}   

    bool Init();
    
    // virtual int EventWriteCallback() override;

    virtual int EventReadCallback() override; 

    static void MakeNonblocking(int fd);

private:
    TcpServer* thread_pool_;
    int listen_port_;
} ;

}