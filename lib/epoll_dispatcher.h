#pragma once

#include  <sys/epoll.h>
#include "event_dispatcher.h"
#include "event_loop.h"
#include "log.h"

#define MAXEVENTS 128

namespace networking {

class EpollDispatcher: public EventDispatcher {
public:

    ~EpollDispatcher() { Clear(); }
    
    virtual bool Init(void* data) override;

	virtual bool Add(std::shared_ptr<Channel> channel) override;	

	virtual bool Del(std::shared_ptr<Channel> channel) override;	

	virtual bool Update(std::shared_ptr<Channel> channel) override;	

	virtual bool Dispatch(struct timeval *) override;	

	virtual void Clear() override;	

protected:

    struct epoll_event GetEpollEvent(std::shared_ptr<Channel> channel);

    EventLoop* event_loop_ = nullptr;
    int event_count_ = 0;
    int nfds_ = 0;
    int realloc_copy_ = 0;
    int efd_ = 0;
    std::vector<struct epoll_event> events_;
};

}