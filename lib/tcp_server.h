#pragma once

#include "event_loop_thread.h"
#include "tcp_application.h"

namespace networking {
class TcpServer {
public:
    TcpServer(int thread_num, int listen_port, std::shared_ptr<TcpApplication> application): 
        thread_num_(std::max(1, thread_num)),
        listen_port_(listen_port),
        application_(application) {}

    void Start();

    std::shared_ptr<EventLoop> SelectSubEventLoop();

    std::shared_ptr<TcpApplication> GetTcpApplication() { return application_; }

private:

    std::shared_ptr<EventLoop> GetMainEventLoop() { return main_loop_thread_->GetEventLoop(); }

    //创建thread_pool的主线程
    std::shared_ptr<networking::EventLoopThread> main_loop_thread_;
    //是否已经启动
    int started_;
    //线程数目
    int thread_num_;
    int listen_port_;
    //数组指针，指向创建的event_loop_thread数组
    std::vector<std::shared_ptr<networking::EventLoopThread>> sub_loop_threads_;

    //表示在数组里的位置，用来决定选择哪个event_loop_thread服务
    int position_ = 0;
    std::shared_ptr<TcpApplication> application_;
};

}