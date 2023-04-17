#pragma once

#include    <pthread.h>

#include <string>
#include <thread>
#include "event_loop.h"
#include "sync_cond.h"


namespace networking {

class EventLoopThread {
public:
    EventLoopThread(): thread_name_("Main-Loop-Thread") {}

    EventLoopThread(int id) {
		thread_name_ = "Sub-Loop-Thread-" + std::to_string(id+1);
    }

    ~EventLoopThread() {
        if (work_thread_) {
            work_thread_->join();
            delete work_thread_;
            work_thread_ = nullptr;
        }
    }

    std::shared_ptr<EventLoop> GetEventLoop() { return event_loop_; }

    // 由主线程调用，初始化一个子线程，并且让子线程开始运行event_loop
    void Start();

private:

    void Run();


    std::shared_ptr<EventLoop> event_loop_;
    std::thread* work_thread_ = nullptr;
    pthread_t thread_tid_ = 0;        // thread ID
    SyncCond sync_cond_;
    std::string thread_name_;
    long thread_count_ = 0;    // connections handled
};

}
