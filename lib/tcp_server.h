#pragma once

#include "channel.h"
#include "event_loop_thread.h"

namespace networking {

class TcpServer {
public:
    TcpServer(int thread_num): 
        thread_num_(std::max(1, thread_num)) {}

    void Start(std::shared_ptr<Channel> acceptor_channel);

    EventLoop* SelectSubEventLoop();

protected:

    std::shared_ptr<EventLoop> GetMainEventLoop() { return main_loop_thread_->GetEventLoop(); }

    // 创建thread_pool的主线程
    std::shared_ptr<networking::EventLoopThread> main_loop_thread_;
    // 是否已经启动
    int started_;
    // 线程数目
    int thread_num_;
    // event_loop_thread数组
    std::vector<std::shared_ptr<networking::EventLoopThread>> sub_loop_threads_;

    // 当前选用的event_loop对应的index，用来决定选择哪个event_loop_thread服务
    int position_ = 0;
};

}