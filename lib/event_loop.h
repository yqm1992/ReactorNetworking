#pragma once

#include <pthread.h>
#include "channel.h"
#include "event_dispatcher.h"
#include "sync_cond.h"
#include "common.h"

namespace networking {

struct ChannelElement {
    ChannelElement(int type, std::shared_ptr<Channel> channel): type(type), channel(channel) {}
    int type; //1: add  2: delete 3:update
    std::shared_ptr<Channel> channel;
};

class EventLoop {
public:

    EventLoop(): EventLoop(nullptr) {}

    EventLoop(const std::string& thread_name);

    int Run();

    int AddChannel(int fd, std::shared_ptr<Channel> channel);

    int RemoveChannel(int fd);

    int UpdateChannel(int fd, std::shared_ptr<Channel> channel);

    // dispatcher派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
    // res: EVENT_READ | EVENT_READ等
    int ChannelEventActivate(int fd, int channel_revents);

    void Wakeup();

private:

    bool IsInSameThread();

    bool Check(int fd, std::shared_ptr<Channel> channel);

    std::shared_ptr<Channel> GetChannel(int fd);

    int HandleWakeup();

    int HandlePendingChannel();

    int HandlePendingAdd(int fd, std::shared_ptr<Channel> channel);

    int HandlePendingRemove(int fd);

    int HandlePendingUpdate(int fd, std::shared_ptr<Channel> channel);

    int AdminChannel(int fd, std::shared_ptr<Channel> channel, int type);

    int quit_;
    std::unique_ptr<EventDispatcher> event_dispatcher_;
    std::map<int, std::shared_ptr<Channel>> channel_map_;

    int is_handle_pending_;
	std::list<ChannelElement> pending_channels_;

    pthread_t owner_thread_id_;
    std::mutex mutex_;
    //std::condition_variable cond_;
    int socket_pair_[2];
    std::shared_ptr<Channel> wakeup_channel_;
    std::string thread_name_;
};

}

// AddChannel
// AdminChannel
// HandlePendingChannel or Wakeup
// HandlePendingAdd
// dispatcher->Add
