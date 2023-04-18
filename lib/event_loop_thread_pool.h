#pragma once

#include "event_loop_thread.h"

namespace networking {
class EventLoopThreadPool {
public:
    EventLoopThreadPool(int thread_num): thread_num_(std::max(1, thread_num)) {}

    void Start();

    std::shared_ptr<EventLoop> SelectSubEventLoop();

private:

    std::shared_ptr<EventLoop> GetMainEventLoop() { return main_loop_thread_->GetEventLoop(); }

    //创建thread_pool的主线程
    std::shared_ptr<networking::EventLoopThread> main_loop_thread_;
    //是否已经启动
    int started_;
    //线程数目
    int thread_num_;
    //数组指针，指向创建的event_loop_thread数组
    std::vector<std::shared_ptr<networking::EventLoopThread>> sub_loop_threads_;

    //表示在数组里的位置，用来决定选择哪个event_loop_thread服务
    int position_ = 0;

};

}