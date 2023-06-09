#include  <sys/epoll.h>
#include "dispatcher.h"
#include "common.h"
#include "epoll_dispatcher.h"


namespace networking {

void EpollDispatcher::Clear() {
    close(efd_);
}

struct epoll_event EpollDispatcher::GetEpollEvent(const Channel& channel) {
    epoll_event event = InitialEpollEvent();

    event.data.fd = channel.GetFD();

    if (channel.events_ & CHANNEL_EVENT_READ) {
        event.events |= EPOLLIN;
    }

    if (channel.events_ & CHANNEL_EVENT_WRITE) {
        event.events |= EPOLLOUT;
    }
    // event.events |= EPOLLET;

    return event;
}

int EpollDispatcher::GetChannelEvents(int epoll_events) {
    int channel_events = 0;
    if (epoll_events & EPOLLIN) {
        channel_events |= CHANNEL_EVENT_READ;
    }

    if (epoll_events & EPOLLOUT) {
        channel_events |= CHANNEL_EVENT_WRITE;
    }

    // if (epoll_events & EPOLLWRNORM) {
    //     channel_events |= CHANNEL_EVENT_WRITE;
    // }

    // if (epoll_events & EPOLLWRBAND) {
    //     channel_events |= 0;
    // }
    return channel_events;
}

bool EpollDispatcher::Init(void* data) {
    efd_ = epoll_create1(0);
    if (efd_ == -1) {
        // error(1, errno, "epoll create failed");
        return false;
    }

    event_loop_ = static_cast<EventLoop*>(data);
    events_.resize(MAXEVENTS);
    return true;
}


bool EpollDispatcher::Add(const Channel& channel) {
    struct epoll_event event = GetEpollEvent(channel);
    if (epoll_ctl(efd_, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
        // error(1, errno, "epoll_ctl add fd failed");
        return false;
    }

    return true;
}

bool EpollDispatcher::Del(const Channel& channel) {
    struct epoll_event event = GetEpollEvent(channel);
    if (epoll_ctl(efd_, EPOLL_CTL_DEL, event.data.fd, &event) == -1) {
        // error(1, errno, "epoll_ctl delete fd failed");
        return false;
    }

    return true;
}

bool EpollDispatcher::Update(const Channel& channel) {
    struct epoll_event event = GetEpollEvent(channel);
    if (epoll_ctl(efd_, EPOLL_CTL_MOD, event.data.fd, &event) == -1) {
        // error(1, errno, "epoll_ctl modify fd failed");
        return false;
    }
    return true;
}

bool EpollDispatcher::Dispatch(struct timeval *timeval) {
    int len = epoll_wait(efd_, &events_[0], MAXEVENTS, -1);
    // yolanda_msgx("epoll_wait wakeup, %s", name_.c_str());
    for (int i = 0; i < len; i++) {
        const auto& cur_event = events_[i];
        if ((cur_event.events & EPOLLERR) || (cur_event.events & EPOLLHUP)) {
            fprintf(stderr, "epoll error on [fd=%d]\n", cur_event.data.fd);
            // RemoveChannel 应该包含三个动作
            // 1、channel->Close() 2、从epoll中删除fd 3、从channel_map中删除fd
            // 回调函数中，只允许调用connection->Shutdown来关闭写连接，close统一由RemoveChannel来完成
            event_loop_->RemoveChannel(cur_event.data.fd);
            continue;
        }

        int channel_events = GetChannelEvents(cur_event.events);
        if (channel_events == 0) {
            yolanda_msgx("%d receive epoll_events: %d", cur_event.data.fd, cur_event.events);
        }
        event_loop_->ChannelEventActivate(cur_event.data.fd, channel_events);
    }

    return true;
}

}