#pragma once

//#include "common.h"
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
    
    Channel() {}

    virtual ~Channel() {}

    int GetFD() const { return fd_; }

    bool WriteEventIsEnabled();

    void EnableWriteEvent(); 

    void DisableWriteEvent();

    virtual int EventReadCallback(); 

    virtual int EventWriteCallback();

protected:
    void Set(int fd, int events) { fd_ = fd; events_ = events; }

	int fd_ = -1;
	int events_ = 0;   // 表示event类型
};

}


