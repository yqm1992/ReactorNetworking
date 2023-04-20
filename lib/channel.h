#pragma once

//#include "common.h"
//#include "buffer.h"
#include <string>

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
    friend class Dispatcher;
    friend class EpollDispatcher;
    
    Channel() {}

    virtual ~Channel() {}

    int GetFD() const { return fd_; }

    bool WriteEventIsEnabled() { return events_ & CHANNEL_EVENT_WRITE; }

    bool ReadEventIsEnabled() { return events_ & CHANNEL_EVENT_READ; }

    void EnableWriteEvent() { events_ |= CHANNEL_EVENT_WRITE; }

    void DisableWriteEvent() { events_ &= ~CHANNEL_EVENT_WRITE; }

    std::string GetDescription() { 
        std::string ret = "[fd=" + std::to_string(fd_) + " (" + type_ + ") focus_events=" + GetEventsString(events_) + "]";
        return std::move(ret);
    }

    virtual int EventReadCallback() { return 0; } 

    virtual int EventWriteCallback() { return 0; }

    static std::string GetEventsString(int revents) {
        std::string events_str;
        if (revents & CHANNEL_EVENT_READ) {
            events_str += (events_str == "") ? "READ" : "|READ";
        }
        if (revents & CHANNEL_EVENT_WRITE) {
            events_str += (events_str == "") ? "WRITE" : "|WRITE";
        }
        return std::move(events_str);
    }

protected:
    void Set(int fd, int events, void* data, const std::string& type) { fd_ = fd; events_ = events; data_= data; type_ = type; }

	int fd_ = -1;
	int events_ = 0;   // 表示event类型
    void *data_ = nullptr; //callback data, 可能是event_loop，也可能是tcp_server或者tcp_connection
    std::string type_;
};

}


