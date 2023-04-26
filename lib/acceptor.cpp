#include <assert.h>

#include "acceptor.h"
#include "tcp_connection.h"
#include "tcp_server.h"

namespace networking {

// std::shared_ptr<Channel> Acceptor::MakeAcceptorChannel(int listen_port) {
//     auto acceptor = new Acceptor(listen_port);
//     acceptor->Init();
//     std::shared_ptr<Channel> channel;
//     channel.reset(static_cast<Channel*>(acceptor));
//     return channel;
// }

std::shared_ptr<Channel> Acceptor::MakeTcpConnectionChannel(int connected_fd, EventLoop *event_loop) {
    auto tcp_server = GetTcpServer();
    // 选出一个EventLoop
    auto io_loop = tcp_server->SelectSubEventLoop();
    // 新建tcp_connction
    TcpConnection* tcp_connection = new TcpConnection(connected_fd, event_loop);
    // 先初始化应用层对象tcp_application
    auto tcp_application = MakeTcpApplicationLayer(tcp_connection);
    // 再用tcp_application初始化tcp_connection
    tcp_connection->Init(tcp_application);

    std::shared_ptr<Channel> tcp_connection_channel;
    tcp_connection_channel.reset(static_cast<Channel*>(tcp_connection));

    return tcp_connection_channel;
}

bool Acceptor::MakeNonblocking(int fd) {
    return fcntl(fd, F_SETFL, O_NONBLOCK) == 0;
}

int Acceptor::GetListenFD(int listen_port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (!MakeNonblocking(listen_fd)) {
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(listen_port);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ret1 < 0) {
        // error(1, errno, "bind failed ");
        close(listen_fd);
        return -1;
    }

    int ret2 = listen(listen_fd, LISTENQ);
    if (ret2 < 0) {
        // error(1, errno, "listen failed ");
        close(listen_fd);
        return -1;
    }
    //    signal(SIGPIPE, SIG_IGN);
    return listen_fd;
}

bool Acceptor::Init() {
    int listen_fd = GetListenFD(listen_port_);
    Set(listen_fd, CHANNEL_EVENT_READ, nullptr, "acceptor");
    return listen_fd >= 0;
}

// Acceptor 要能够获取所有SubEventLoop
int Acceptor::HandleConnectionEstablised() {
    struct sockaddr client_addr;
    socklen_t client_len;

    int conn_fd = accept(fd_, &client_addr, &client_len);
    if (conn_fd < 0) {
        // error(1, errno, "bind failed ");
        return -1;
    }
    if (!MakeNonblocking(conn_fd)) {
        close(conn_fd);
        return -1;
    }
    yolanda_msgx("new connection fd = %d", conn_fd);
    // close(fd_);
    
    auto tcp_server = GetTcpServer();
    auto io_loop = tcp_server->SelectSubEventLoop();
    io_loop->AddChannel(MakeTcpConnectionChannel(conn_fd, io_loop));
    return 0;
}

}
