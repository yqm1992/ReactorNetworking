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

void TcpServer::Start() {
    main_loop_thread_ = std::make_shared<networking::EventLoopThread>("Main-Loop");
    main_loop_thread_->Start();
    for (int i = 0; i < thread_num_; ++i) {
        auto cur_thread = std::make_shared<networking::EventLoopThread>("Sub-Loop-" + std::to_string(i+1));
        cur_thread->Start();
        sub_loop_threads_.push_back(cur_thread);
    }
    auto acceptor = new Acceptor();
    acceptor->Init(this, listen_port_);
    std::shared_ptr<Channel> channel;
    channel.reset(static_cast<Channel*>(acceptor));
    main_loop_thread_->GetEventLoop()->AddChannel(channel);
}

EventLoop* TcpServer::SelectSubEventLoop() {
    int index = (position_++) % thread_num_;
    if (position_ >= thread_num_) {
        position_ = 0;
    }
    return sub_loop_threads_[index]->GetEventLoop().get();
}

}
