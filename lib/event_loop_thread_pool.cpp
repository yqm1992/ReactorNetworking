#include "acceptor.h"
#include "event_loop_thread_pool.h"

namespace networking {

void EventLoopThreadPool::Start() {
    main_loop_thread_ = std::make_shared<networking::EventLoopThread>("Main-Loop");
    main_loop_thread_->Start();
    for (int i = 0; i < thread_num_; ++i) {
        auto cur_thread = std::make_shared<networking::EventLoopThread>("Sub-Loop" + std::to_string(i+1));
        cur_thread->Start();
        sub_loop_threads_.push_back(cur_thread);
    }
    auto acceptor = new Acceptor(this, 43211);
    acceptor->Init();
    std::shared_ptr<Channel> channel;
    channel.reset(static_cast<Channel*>(acceptor));
    main_loop_thread_->GetEventLoop()->AddChannel(channel);
}

std::shared_ptr<EventLoop> EventLoopThreadPool::SelectSubEventLoop() {
    int index = (position_++) % thread_num_;
    if (position_ >= thread_num_) {
        position_ = 0;
    }
    return sub_loop_threads_[index]->GetEventLoop();
}

}
