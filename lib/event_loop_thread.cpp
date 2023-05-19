#include "event_loop_thread.h"

namespace networking {

EventLoopThread::EventLoopThread(const std::string& event_loop_name) {
    event_loop_ = std::make_shared<EventLoop>(event_loop_name);
    thread_name_ = event_loop_->GetName() + "-Thread";
}

EventLoopThread::EventLoopThread(std::shared_ptr<EventLoop> event_loop) {
    event_loop_ = event_loop;
    thread_name_ = event_loop_->GetName() + "-Thread";
}

void EventLoopThread::Start() {
    work_thread_ = new std::thread(&EventLoopThread::Run, this);
    sync_cond_.Wait();
}

void EventLoopThread::Run() {    
    // 初始化event loop，之后通知主线程
    event_loop_->Init();
    yolanda_msgx("event loop thread init and signal, %s", thread_name_.c_str());
    sync_cond_.Notify();
    //子线程event loop run
    event_loop_->Run();
}

}
