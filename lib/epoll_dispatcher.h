#pragma once

#include  <sys/epoll.h>
#include "dispatcher.h"
#include "event_loop.h"
#include "log.h"

#define MAXEVENTS 128

namespace networking {

class EpollDispatcher: public Dispatcher {
public:

    EpollDispatcher(const std::string& name): Dispatcher(name) {}

    ~EpollDispatcher() { Clear(); }

    static std::shared_ptr<Dispatcher> MakeDispatcher() {
        std::shared_ptr<Dispatcher> dispatcher;
        dispatcher.reset(static_cast<Dispatcher*>( new EpollDispatcher("epoll") ));
        return dispatcher;
    }
    
    virtual bool Init(void* data) override;

	virtual bool Add(const Channel& channel) override;	

	virtual bool Del(const Channel& channel) override;	

	virtual bool Update(const Channel& channel) override;	

	virtual bool Dispatch(struct timeval *) override;	

	virtual void Clear() override;	

protected:

    struct epoll_event GetEpollEvent(const Channel& channel);

    int GetChannelEvents(int epoll_events);

    EventLoop* event_loop_ = nullptr;
    int event_count_ = 0;
    int nfds_ = 0;
    int realloc_copy_ = 0;
    int efd_ = 0;
    std::vector<struct epoll_event> events_;
};

}