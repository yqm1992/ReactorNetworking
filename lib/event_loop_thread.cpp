#include "event_loop_thread.h"


namespace networking {

void EventLoopThread::Start() {
    work_thread_ = new std::thread(&EventLoopThread::Run, this);
    sync_cond_.Wait();
    yolanda_msgx("event loop thread started, %s", thread_name_.c_str());
}

void EventLoopThread::Run() {    
    // 初始化event loop，之后通知主线程
    event_loop_ = std::make_shared<EventLoop>(thread_name_);
    yolanda_msgx("event loop thread init and signal, %s", thread_name_.c_str());
    sync_cond_.Notify();
    //子线程event loop run
    event_loop_->Run();
}

}
