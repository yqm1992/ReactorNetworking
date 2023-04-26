#include "acceptor.h"
#include "tcp_server.h"

namespace networking {

// int TcpApplicationLayer::ConnectionCompletedCallBack(TcpConnection* connection) {
//     yolanda_msgx("ConnectionCompletedCallBack");
//     return 0;
// }

// int TcpApplicationLayer::ConnectionClosedCallBack(TcpConnection* connection) {
//     yolanda_msgx("ConnectionClosedCallBack");
//     return 0;
// }

// int TcpApplicationLayer::MessageCallBack(TcpConnection* connection) {
//     yolanda_msgx("MessageCallBack");
//     return 0;
// }

// int TcpApplicationLayer::WriteCompletedCallBack(TcpConnection* connection) {
//     yolanda_msgx("WriteCompletedCallBack");
//     return 0;
// }

void TcpServer::Start(std::shared_ptr<Channel> acceptor_channel) {
    main_loop_thread_ = std::make_shared<networking::EventLoopThread>("Main-Loop");
    main_loop_thread_->Start();
    for (int i = 0; i < thread_num_; ++i) {
        auto cur_thread = std::make_shared<networking::EventLoopThread>("Sub-Loop-" + std::to_string(i+1));
        cur_thread->Start();
        sub_loop_threads_.push_back(cur_thread);
    }
    static_cast<Acceptor*>(acceptor_channel.get())->SetTcpServer(this);
    main_loop_thread_->GetEventLoop()->AddChannel(acceptor_channel);
}

EventLoop* TcpServer::SelectSubEventLoop() {
    int index = (position_++) % thread_num_;
    if (position_ >= thread_num_) {
        position_ = 0;
    }
    return sub_loop_threads_[index]->GetEventLoop().get();
}

}
