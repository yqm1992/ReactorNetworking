#pragma once

#include <pthread.h>
#include <assert.h>
#include <atomic>
#include "channel.h"
#include "dispatcher.h"
#include "sync_cond.h"
#include "common.h"

namespace networking {

enum ADMIN_CHANNEL : int {
    ADMIN_CHANNEL_ADD = 1,
    ADMIN_CHANNEL_REMOVE = 2,
    ADMIN_CHANNEL_UPDATE = 3
};

struct ChannelElement {
    ChannelElement(int type, std::shared_ptr<Channel> channel): type(type), channel(channel) {}
    int type; // 1: add  2: delete 3:update
    std::shared_ptr<Channel> channel;
};


class EventLoop;

class WakeupChannel: public Channel {
public:
    void Init(int fd);

    virtual int Close() override {
        return close(fd_);
    }
    
    virtual int EventReadCallback() override;
};

class EventLoop {
public:
    EventLoop(const std::string& name);

    ~EventLoop() { Stop(); }

    bool Init();

    int Run();

    int AddChannel(std::shared_ptr<Channel> channel) {
        if (channel == nullptr) {
            return 0;
        }
        return AdminChannel(channel, ADMIN_CHANNEL_ADD);
    }

    int RemoveChannel(int fd) {
        auto channel = GetChannel(fd);
        if (channel == nullptr) {
            return 0;
        }
        return AdminChannel(channel, ADMIN_CHANNEL_REMOVE);
    }

    int UpdateChannelEvent(int fd) {
        auto channel = GetChannel(fd);
        if (channel == nullptr) {
            return 0;
        }
        return AdminChannel(channel, ADMIN_CHANNEL_UPDATE);
    }

    void Stop() {
        work_ = false;
        Wakeup();
    }

    void Wakeup();

    std::string GetName() { return name_; }

    // dispatcher派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
    // res: EVENT_READ | EVENT_READ等
    int ChannelEventActivate(int fd, int channel_revents);

private:

    std::shared_ptr<Channel> GetChannel(int fd);

    bool InOwnerThread() { return owner_thread_id_ == pthread_self(); }

    int HandlePendingChannels(); // 处理pending队列中的ChannelElement

    int HandleChannelElement(const ChannelElement& channel_element); // 处理单个ChannelElement

    int HandlePendingAdd(std::shared_ptr<Channel> channel);

    int HandlePendingRemove(int fd);

    int HandlePendingUpdate(int fd);

    int AdminChannel(std::shared_ptr<Channel> channel, int type);

    std::string name_;
    pthread_t owner_thread_id_;
    int socket_pair_[2];
    std::shared_ptr<Channel> wakeup_channel_;
    std::unique_ptr<Dispatcher> event_dispatcher_;
    std::map<int, std::shared_ptr<Channel>> channel_map_;
    std::mutex mutex_;
    int is_handle_pending_;
	std::list<ChannelElement> pending_channels_;
    std::list<ChannelElement> error_channels_; // 存放有问题的channel
    std::atomic_bool work_;

};

}
// ------------ 其他线程中调用AddChannel -----------------
//      other_thread 调用AddChannel
// AddChannel
// AdminChannel(1)
// pending_channels_.push_back()
// Wakeup

//      event_loop_thread 处理
// HandlePendingChannels
// HandleChannelElement
// HandlePendingAdd
// dispatcher->Add

// ------------ EventLoop线程中调用AddChannel -----------------
// * event_loop_thread
// AddChannel
// AdminChannel(1)
// HandleChannelElement
// HandlePendingAdd
// dispatcher->Add

// AddChannel RemoveChannel UpdateChannel
//              AdminChannel
//              HandleChannelElement
// HandlePendingAdd HandlePendingRemove HandlePendingUpdate