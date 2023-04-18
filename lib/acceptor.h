#pragma once

#include "common.h"
#include "channel.h"
#include "event_loop_thread_pool.h"

namespace networking {

class Acceptor: public Channel {
public:

    Acceptor(EventLoopThreadPool* thread_pool, int port): thread_pool_(thread_pool), listen_port_(port) {}   

    bool Init();
    
    // virtual int EventWriteCallback() override;

    virtual int EventReadCallback() override; 

    static void MakeNonblocking(int fd);

private:
    EventLoopThreadPool* thread_pool_;
    int listen_port_;
} ;

}