#pragma once

#include <pthread.h>
#include "channel.h"
#include "event_dispatcher.h"
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

class EventLoop {
public:
    EventLoop(const std::string& name);

    bool Init();

    int Run();

    int AddChannel(std::shared_ptr<Channel> channel) {
        return AdminChannel(channel, ADMIN_CHANNEL_ADD);
    }

    int RemoveChannel(std::shared_ptr<Channel> channel) {
        return AdminChannel(channel, ADMIN_CHANNEL_REMOVE);
    }

    int UpdateChannel(std::shared_ptr<Channel> channel) {
        return AdminChannel(channel, ADMIN_CHANNEL_UPDATE);
    }


    void Wakeup();

    std::string GetName() { return name_; }

    // dispatcher派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
    // res: EVENT_READ | EVENT_READ等
    int ChannelEventActivate(int fd, int channel_revents);

private:

    bool Check(int fd, std::shared_ptr<Channel> channel);

    std::shared_ptr<Channel> GetChannel(int fd);

    // int HandleWakeup();

    int HandlePendingChannel();

    int HandlePendingAdd(std::shared_ptr<Channel> channel);

    int HandlePendingRemove(int fd);

    int HandlePendingUpdate(int fd);

    int AdminChannel(std::shared_ptr<Channel> channel, int type);

    std::string name_;
    int socket_pair_[2];
    std::shared_ptr<Channel> wakeup_channel_;
    std::unique_ptr<EventDispatcher> event_dispatcher_;
    std::map<int, std::shared_ptr<Channel>> channel_map_;
    std::mutex mutex_;
    int is_handle_pending_;
	std::list<ChannelElement> pending_channels_;
    int quit_;

};

}

// AddChannel
// AdminChannel(1)
// HandlePendingChannel or Wakeup
// HandlePendingAdd
// dispatcher->Add
