#pragma once

//#include "common.h"
//#include "event_loop.h"
//#include "buffer.h"

namespace networking {

enum CHANNEL_EVENT: int {
    CHANNEL_EVENT_TIMEOUT = 0x01,
    // Wait for a socket or FD to become readable 
    CHANNEL_EVENT_READ = 0x02,
    // Wait for a socket or FD to become writeable 
    CHANNEL_EVENT_WRITE = 0x04,
    // Wait for a POSIX signal to be raised
    CHANNEL_EVENT_SIGNAL = 0x08
};

class Channel {
public:
    friend class EventDispatcher;
    friend class EpollDispatcher;
    
    Channel(int fd, int events, void *data); 
    int GetFD() const { return fd_; }
    bool WriteEventIsEnabled();
    bool EnableWriteEvent(); 
    bool DisableWriteEvent();
    virtual int EventReadCallback(); 
    virtual int EventWriteCallback();

protected:
	int fd_;
	int events_;   //表示event类型
    void *data_; //callback data, 可能是event_loop，也可能是tcp_server或者tcp_connection
};

}


