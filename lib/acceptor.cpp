#include <assert.h>

#include "acceptor.h"
#include "tcp_connection.h"
#include "tcp_server.h"

namespace networking {

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
    Set(listen_fd, CHANNEL_EVENT_READ, "acceptor");
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
    
    auto io_loop = GetTcpServer()->SelectSubEventLoop();
    auto tcp_connection_channel = TcpConnection::MakeTcpConnectionChannel(conn_fd, MakeTcpApplication());
    io_loop->AddChannel(tcp_connection_channel);
    return 0;
}

}
