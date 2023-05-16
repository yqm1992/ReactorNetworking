#include "tcp_client.h"


namespace networking {
    
TcpClient::TcpClient(const std::string& server_address, int port, std::shared_ptr<TcpApplication> tcp_application) {
    server_address_ = server_address;
    port_ = port;
    tcp_application_ = tcp_application;
    loop_thread_ = std::make_shared<networking::EventLoopThread>("Main-Loop");
    loop_thread_->Start();
}

bool TcpClient::Connect() {
    if (channel_ == nullptr) {
        channel_ = MakeTcpConnectionChannel(server_address_, port_, tcp_application_);
        loop_thread_->GetEventLoop()->AddChannel(channel_);
    }
    return channel_ != nullptr;
}

std::shared_ptr<Channel> TcpClient::MakeTcpConnectionChannel(const std::string& server_address, int port, std::shared_ptr<TcpApplication> tcp_application) {
    int socket_fd; 
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERV_PORT); 
    inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr); 

    socklen_t server_len = sizeof(server_addr); 
    if (connect(socket_fd, (struct sockaddr *) &server_addr, server_len) < 0) {
        return nullptr;
    }
    return TcpConnection::MakeTcpConnectionChannel(socket_fd, tcp_application);
}

}