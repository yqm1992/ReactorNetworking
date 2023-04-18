#pragma once

#include "channel.h"
#include "event_loop.h"


namespace networking {

class WakeupChannel: public Channel {
public:
    void Init(EventLoop* event_loop, int fd);
    
    virtual int EventReadCallback() override;
    
private:
    EventLoop* event_loop_;
};

}