#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "log.h"
// #include "event_dispatcher.h"
#include "epoll_dispatcher.h"
#include "channel.h"
#include "wakeup_channel.h"
// #include "utils.h"

namespace networking {

EventLoop::EventLoop(const std::string& name): quit_(0), is_handle_pending_(0), name_(name) {}

bool EventLoop::Init() {
    yolanda_msgx("set epoll as dispatcher, %s", name_.c_str());
    event_dispatcher_.reset(static_cast<EventDispatcher*>( new EpollDispatcher() ));

    if (!event_dispatcher_->Init(this)) {
        return false;
    }

    //add the socketfd to event
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair_) < 0) {
        LOG_ERR("socketpair set fialed");
        return false;
    }
    
    WakeupChannel* wakeup_channel = new WakeupChannel();
    wakeup_channel->Init(this, socket_pair_[1]);
    wakeup_channel_.reset(static_cast<Channel*>(wakeup_channel));
    AddChannel(wakeup_channel_);
}

// in the i/o thread
int EventLoop::HandlePendingChannel() {
    //get the lock
    std::unique_lock<std::mutex> lock(mutex_);
    is_handle_pending_ = 1;

    for (auto& channel_element: pending_channels_) {
        auto channel = channel_element.channel;
        auto fd = channel->GetFD();
        switch (channel_element.type)
        {
            case ADMIN_CHANNEL_ADD:
                HandlePendingAdd(channel);
                break;
            case ADMIN_CHANNEL_REMOVE:
                HandlePendingRemove(fd);
                break;
            case ADMIN_CHANNEL_UPDATE:
                HandlePendingUpdate(fd);
                break;
            default:
                break;
        }
    }
    
    pending_channels_.clear();
    is_handle_pending_ = 0;

    // ~lock, release the lock

    return 0;
}

int EventLoop::AdminChannel(std::shared_ptr<Channel> channel, int type) {
    //get the lock
    std::unique_lock<std::mutex> lock(mutex_);
    assert(is_handle_pending_ == 0);
    //add channel into the pending list
    ChannelElement channel_element(type, channel);
    pending_channels_.push_back(channel_element);
    Wakeup();
    return 0;
}

std::shared_ptr<Channel> EventLoop::GetChannel(int fd) {
    if (channel_map_.find(fd) == channel_map_.end()) {
        return nullptr;
    }
    auto found_channel = channel_map_.find(fd)->second;
    assert(fd == found_channel->GetFD());
    return found_channel;
}

// in the i/o thread
int EventLoop::HandlePendingAdd(std::shared_ptr<Channel> channel) {
    yolanda_msgx("add channel fd == %d, %s", channel->GetFD(), name_.c_str());
    int fd = channel->GetFD();

    if (fd < 0) {
        return 0;
    }

    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel != nullptr) {
        return 0;
    }

    //add channel
    event_dispatcher_->Add(*channel);
    channel_map_.emplace(fd, channel);
    return 1;
}

// in the i/o thread
int EventLoop::HandlePendingRemove(int fd) {
    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel == nullptr) {
        return -1;
    }

    //update dispatcher(multi-thread)here
    int retval = (event_dispatcher_->Del(*found_channel) == -1) ? -1 : 1;
    if (retval == 1) {
        channel_map_.erase(fd);
    }
    return retval;
}

// in the i/o thread
int EventLoop::HandlePendingUpdate(int fd) {
    yolanda_msgx("update channel fd == %d, %s", fd, name_.c_str());

    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel == nullptr) {
        return -1;
    }

    //update channel
    event_dispatcher_->Update(*found_channel);
}

int EventLoop::ChannelEventActivate(int fd, int channel_revents) {
    yolanda_msgx("activate channel fd == %d, revents=%d, %s", fd, channel_revents, name_.c_str());

    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel == nullptr) {
        return -1;
    }

    if (channel_revents & (CHANNEL_EVENT_READ)) {
        found_channel->EventReadCallback();
    }
    if (channel_revents & (CHANNEL_EVENT_WRITE)) {
        found_channel->EventWriteCallback();
    }

    return 0;

}

void EventLoop::Wakeup() {
    char one = 'a';
    ssize_t n = write(socket_pair_[0], &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("Wakeup event loop thread failed");
    }
}

// int EventLoop::HandleWakeup() {
//     char one;
//     ssize_t n = read(socket_pair_[1], &one, sizeof one);
//     if (n != sizeof one) {
//         LOG_ERR("HandleWakeup failed");
//     }
//     yolanda_msgx("Wakeup, %s", name_.c_str());
//     return 0;
// }

// 1.参数验证
// 2.调用dispatcher来进行事件分发,分发完回调事件处理函数
int EventLoop::Run() {

    yolanda_msgx("event loop run, %s", name_.c_str());
    struct timeval timeval;
    timeval.tv_sec = 1;

    while (!quit_) {
        //block here to wait I/O event, and get active channels
        event_dispatcher_->Dispatch(&timeval);

        //handle the pending channel
        HandlePendingChannel();
    }

    yolanda_msgx("event loop end, %s", name_.c_str());
    return 0;
}

}
