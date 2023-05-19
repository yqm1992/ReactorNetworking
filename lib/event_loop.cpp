#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "log.h"
// #include "dispatcher.h"
#include "epoll_dispatcher.h"
#include "channel.h"
// #include "utils.h"

namespace networking {

// TODO: channel 关闭，删除过程中出现错误了怎么妥善处理比较好？

void WakeupChannel::Init(int fd) {
    Set(fd, CHANNEL_EVENT_READ, "wakeup_fd");
}

int WakeupChannel::EventReadCallback() {
    char one;
    ssize_t n = read(fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("HandleWakeup failed");
    }
    // yolanda_msgx("read from channel %s, %s", GetDescription().c_str(), event_loop_->GetName().c_str());
    return 0;
}

EventLoop::EventLoop(const std::string& name): work_(true), is_handle_pending_(0), name_(name) {}

bool EventLoop::Init() {
    owner_thread_id_ = pthread_self();
    yolanda_msgx("set epoll as dispatcher, %s", name_.c_str());
    event_dispatcher_.reset(static_cast<Dispatcher*>( new EpollDispatcher(name_) ));
    if (!event_dispatcher_->Init(this)) {
        return false;
    }

    //add the socketfd to event
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair_) < 0) {
        LOG_ERR("socketpair set fialed");
        return false;
    }
    
    WakeupChannel* wakeup_channel = new WakeupChannel();
    wakeup_channel->Init(socket_pair_[1]);
    wakeup_channel_.reset(static_cast<Channel*>(wakeup_channel));
    AddChannel(wakeup_channel_);
}

int EventLoop::HandleChannelElement(const ChannelElement& channel_element) {
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
    return 0;
}

// in the i/o thread
int EventLoop::HandlePendingChannels() {
    std::list<ChannelElement> cur_error_channel_list;
    //get the lock
    std::unique_lock<std::mutex> lock(mutex_);
    //  先处理pending_channels
    is_handle_pending_ = 1;
    for (auto& channel_element: pending_channels_) {
        if (HandleChannelElement(channel_element) != 0) {
            cur_error_channel_list.push_back(channel_element);
        }
    }
    pending_channels_.clear();
    // 再处理上次遗留的error_channels
    for (auto& channel_element: error_channels_) {
        if (HandleChannelElement(channel_element) != 0) {
            cur_error_channel_list.push_back(channel_element);
        }
    }
    std::swap(cur_error_channel_list, error_channels_);
    is_handle_pending_ = 0;
    // ~lock, release the lock

    return 0;
}

int EventLoop::AdminChannel(std::shared_ptr<Channel> channel, int type) {
    ChannelElement channel_element(type, channel);
    if (InOwnerThread()) {
        HandleChannelElement(channel_element); // EventLoop的owner线程可以直接处理
    } else {
        // 其他的线程，需要把事件挂载到pending队列中，然后唤醒owener线程去处理
        //get the lock
        std::unique_lock<std::mutex> lock(mutex_);
        assert(is_handle_pending_ == 0);
        //add channel into the pending list
        pending_channels_.push_back(channel_element);
        Wakeup();
    }
    return 0;
}

std::shared_ptr<Channel> EventLoop::GetChannel(int fd) {
    auto iter = channel_map_.find(fd);
    if (iter == channel_map_.end()) {
        return nullptr;
    }
    auto found_channel = iter->second;
    assert(fd == found_channel->GetFD());
    return found_channel;
}

// in the i/o thread
int EventLoop::HandlePendingAdd(std::shared_ptr<Channel> channel) {
    yolanda_msgx("add channel %s, %s", channel->GetDescription().c_str(), name_.c_str());
    int fd = channel->GetFD();

    if (fd < 0) {
        return 0;
    }

    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel != nullptr) {
        return 0;
    }
    assert(channel->GetEventLoop() == nullptr);
    channel->SetEventLoop(this);
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
    assert(found_channel->GetEventLoop() == this);

    yolanda_msgx("remove channel %s, %s", found_channel->GetDescription().c_str(), name_.c_str());

    //// TODO: close channel 析构函数
    //if (found_channel->Close() < 0) {
    //    return -1;
    //}
    // remove from dispatcher (multi-thread) here
    if (event_dispatcher_->Del(*found_channel) < 0) {
        return -1;
    }
    // remove from channel_map_
    channel_map_.erase(fd);
    return 1;
}

// in the i/o thread
int EventLoop::HandlePendingUpdate(int fd) {
    yolanda_msgx("update channel fd == %d, %s", fd, name_.c_str());

    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel == nullptr) {
        return -1;
    }
    assert(found_channel->GetEventLoop() == this);
    yolanda_msgx("update channel %s, %s", found_channel->GetDescription().c_str(), name_.c_str());

    //update channel
    event_dispatcher_->Update(*found_channel);
    return 1;
}

int EventLoop::ChannelEventActivate(int fd, int channel_revents) {
    // yolanda_msgx("activate channel fd == %d, revents=%d, %s", fd, channel_revents, name_.c_str());
    std::shared_ptr<Channel> found_channel = GetChannel(fd);
    if (found_channel == nullptr) {
        return -1;
    }
    if (wakeup_channel_->GetFD() != fd) {
        yolanda_msgx("activate channel %s, revents=%s, %s", found_channel->GetDescription().c_str(), Channel::GetEventsString(channel_revents).c_str(), name_.c_str());
    }

    if (channel_revents & (CHANNEL_EVENT_READ)) {
        found_channel->EventReadCallback();
    }
    if (channel_revents & (CHANNEL_EVENT_WRITE)) {
        found_channel->EventWriteCallback();
    }

    if (found_channel->NeedRecycle()) {
        RemoveChannel(found_channel->GetFD());
    }

    return 0;

}

void EventLoop::Wakeup() {
    char one = 'a';
    ssize_t n = write(socket_pair_[0], &one, sizeof one);
    if (n != sizeof(one)) {
        LOG_ERR("Wakeup event loop thread failed");
    }
}

// 1.参数验证
// 2.调用dispatcher来进行事件分发,分发完回调事件处理函数
int EventLoop::Run() {
    if (!InOwnerThread()) {
        LOG_ERR("event loop can only run in the owner thread");
        exit(1);
    }

    yolanda_msgx("event loop run, %s", name_.c_str());
    struct timeval timeval;
    timeval.tv_sec = 1;

    while (work_) {
        //block here to wait I/O event, and get active channels
        event_dispatcher_->Dispatch(&timeval);

        //handle the pending channel
        HandlePendingChannels();
    }

    yolanda_msgx("event loop end, %s", name_.c_str());
    return 0;
}

// Close连接： 框架 read(fd) == 0 || error，需要RemoveChannel（会调用channel->Close()）
//
// 上层主动Shutdown，DISABLE WriteEvent + UpdateChannel

}
