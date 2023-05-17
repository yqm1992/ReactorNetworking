#pragma once

#include "tcp_connection.h"
#include "event_loop_thread.h"

namespace networking {

class TcpClient {
public:

    TcpClient(const std::string& server_address, int port, std::shared_ptr<TcpApplication> tcp_application);

    bool Connect();

    std::shared_ptr<TcpApplication> GetTcpApplication() { return tcp_application_; }

    static std::shared_ptr<Channel> MakeTcpConnectionChannel(const std::string& server_address, int port, std::shared_ptr<TcpApplication> tcp_application);

protected:
    std::shared_ptr<EventLoopThread> loop_thread_;
    std::shared_ptr<TcpApplication> tcp_application_;
    std::string server_address_;
    int port_;
};

}