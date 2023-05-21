#include "acceptor.h"
#include "tcp_server.h"

namespace networking {

// int TcpApplication::ConnectionCompletedCallBack(TcpConnection* connection) {
//     yolanda_msgx("ConnectionCompletedCallBack");
//     return 0;
// }

// int TcpApplication::ConnectionClosedCallBack(TcpConnection* connection) {
//     yolanda_msgx("ConnectionClosedCallBack");
//     return 0;
// }

// int TcpApplication::MessageCallBack(TcpConnection* connection) {
//     yolanda_msgx("MessageCallBack");
//     return 0;
// }

// int TcpApplication::WriteCompletedCallBack(TcpConnection* connection) {
//     yolanda_msgx("WriteCompletedCallBack");
//     return 0;
// }

void TcpServer::Start(std::shared_ptr<Channel> acceptor_channel) {
    main_loop_thread_ = std::make_shared<networking::EventLoopThread>("Main-Loop");
    main_loop_thread_->Start();
    for (int i = 0; i < loop_num_; ++i) {
        auto cur_thread = std::make_shared<networking::EventLoopThread>("Sub-Loop-" + std::to_string(i+1));
        cur_thread->Start();
        sub_loop_threads_.push_back(cur_thread);
    }
    for (int i = 0; i < worker_num_; ++i) {
        auto cur_thread = std::make_shared<WorkThread>();
        cur_thread->Start();
        work_threads_.push_back(cur_thread);
    }
    static_cast<Acceptor*>(acceptor_channel.get())->SetTcpServer(this);
    main_loop_thread_->GetEventLoop()->AddChannel(acceptor_channel);
}

EventLoop* TcpServer::SelectSubEventLoop() {
    int index = (loop_position_++) % loop_num_;
    if (loop_position_ >= loop_num_) {
        loop_position_ = 0;
    }
    return sub_loop_threads_[index]->GetEventLoop().get();
}

WorkThread* TcpServer::SelectWorkThread() {
    if (work_threads_.empty()) {
        return nullptr;
    }
    int index = (worker_position_++) % (work_threads_.size());
    if (worker_position_ >= work_threads_.size()) {
        worker_position_ = 0;
    }
    return work_threads_[index].get();
}

}
