#include "event_loop_thread_pool.h"

namespace networking {

void EventLoopThreadPool::Start() {
    main_loop_thread_ = std::make_shared<networking::EventLoopThread>();
    main_loop_thread_->Start();
    for (int i = 0; i < thread_num_; ++i) {
        auto cur_thread = std::make_shared<networking::EventLoopThread>(i);
        cur_thread->Start();
        sub_loop_threads_.push_back(cur_thread);
    }
}

std::shared_ptr<EventLoop> EventLoopThreadPool::SelectSubEventLoop() {
    int index = (position_++) % thread_num_;
    if (position_ >= thread_num_) {
        position_ = 0;
    }
    return sub_loop_threads_[index]->GetEventLoop();
}

}