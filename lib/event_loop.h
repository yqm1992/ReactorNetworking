#pragma once

#include <pthread.h>
#include <assert.h>
#include <atomic>
#include "channel.h"
#include "dispatcher.h"
#include "sync_cond.h"
#include "common.h"
#include "worker.h"

namespace networking {

// m


class EventLoop;

class WakeupChannel: public Channel {
public:
    void Init(int fd);

	~WakeupChannel() { Close(); }
	
    int Close() { return close(fd_); }
    
    int EventReadCallback() override;
};

class EventLoop {
public:
    typedef std::function<void()> Task;

    EventLoop(const std::string& name);

    ~EventLoop() { Stop(); }

    bool Init();

    int Run();

    void RunTaskInLoopThread(Task task);

    void QueueInLoop(Task task);

    void AddChannel(std::shared_ptr<Channel> channel) {
        auto task = std::bind(&EventLoop::AddChannelInLoop, this, channel);
        RunTaskInLoopThread(task);
    }

    void RemoveChannel(int fd) {
        auto task = std::bind(&EventLoop::RemoveChannelInLoop, this, fd);
        RunTaskInLoopThread(task);
    }

    void UpdateChannelEvent(int fd) {
        auto task = std::bind(&EventLoop::UpdateChannelInLoop, this, fd);
        RunTaskInLoopThread(task);
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

    bool InOwnerThread() { return owner_thread_id_ == pthread_self(); }

private:

    std::shared_ptr<Channel> GetChannel(int fd);

    void DoPendingTasksInLoop();

    // int HandlePendingChannels(); // 处理pending队列中的ChannelElement

    // int HandleChannelElement(const ChannelElement& channel_element); // 处理单个ChannelElement

    void AddChannelInLoop(std::shared_ptr<Channel> channel);

    void RemoveChannelInLoop(int fd);

    void UpdateChannelInLoop(int fd);

    // int AdminChannel(std::shared_ptr<Channel> channel, int type);

    std::string name_;
    pthread_t owner_thread_id_;
    int socket_pair_[2];
    std::shared_ptr<Channel> wakeup_channel_;
    std::unique_ptr<Dispatcher> event_dispatcher_;
    std::map<int, std::shared_ptr<Channel>> channel_map_;
    std::mutex mutex_;
    int is_handle_pending_;
	// std::list<ChannelElement> pending_channels_;
    // std::list<ChannelElement> error_channels_; // 存放有问题的channel
    std::list<Task> task_list_;
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
// AddChannelInLoop
// dispatcher->Add

// ------------ EventLoop线程中调用AddChannel -----------------
// * event_loop_thread
// AddChannel
// AdminChannel(1)
// HandleChannelElement
// AddChannelInLoop
// dispatcher->Add

// AddChannel RemoveChannel UpdateChannel
//              AdminChannel
//              HandleChannelElement
// AddChannelInLoop RemoveChannelInLoop UpdateChannelInLoop
