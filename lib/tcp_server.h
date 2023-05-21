#pragma once

#include "channel.h"
#include "event_loop_thread.h"

namespace networking {

class TcpServer {
public:
    TcpServer(int loop_num, int worker_num = 0): 
        loop_num_(std::max(1, loop_num)),
        worker_num_(std::max(0, worker_num)) {}

    void Start(std::shared_ptr<Channel> acceptor_channel);

    EventLoop* SelectSubEventLoop();

    WorkThread* SelectWorkThread();

protected:

    std::shared_ptr<EventLoop> GetMainEventLoop() { return main_loop_thread_->GetEventLoop(); }

    // 创建thread_pool的主线程
    std::shared_ptr<networking::EventLoopThread> main_loop_thread_;
    // 是否已经启动
    int started_;
    // EventLopp线程数目
    int loop_num_;
    // 工作线程数
    int worker_num_;
    // event_loop_thread数组
    std::vector<std::shared_ptr<networking::EventLoopThread>> sub_loop_threads_;

    std::vector<std::shared_ptr<WorkThread>> work_threads_;

    // 当前选用的event_loop对应的index，用来决定选择哪个event_loop_thread服务
    int loop_position_ = 0;
    // 当前选用的worker对应的index，用来决定选择哪个worker来执行decoding-computing-encoding任务
    int worker_position_ = 0;
};

}