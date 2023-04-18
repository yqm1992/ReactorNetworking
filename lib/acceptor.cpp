#include <assert.h>

#include "acceptor.h"
#include "event_loop_thread_pool.h"

namespace networking {

void Acceptor::MakeNonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

bool Acceptor::Init() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    MakeNonblocking(listen_fd);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(listen_port_);

    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int ret1 = bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ret1 < 0) {
        // error(1, errno, "bind failed ");
        return false;
    }

    int ret2 = listen(listen_fd, LISTENQ);
    if (ret2 < 0) {
        // error(1, errno, "listen failed ");
        return false;
    }
    //    signal(SIGPIPE, SIG_IGN);
    Set(listen_fd, CHANNEL_EVENT_READ);
    return true;
}


// Acceptor 继承自Channel
// Acceptor 要能够获取所有SubEventLoop
int Acceptor::EventReadCallback() {
    struct sockaddr client_addr;
    socklen_t client_len;

    int conn_fd = accept(fd_, &client_addr, &client_len);
    if (conn_fd < 0) {
        // error(1, errno, "bind failed ");
        return -1;
    }
    MakeNonblocking(conn_fd);
    yolanda_msgx("new connection fd == %d", conn_fd);
    // close(fd_);
    
    auto sub_loop = thread_pool_->SelectSubEventLoop();
    // auto channel = new Channel();
    // channel->Init(conn_fd, CHANNEL_EVENT_READ);
    // sub_loop->AddChannel();
}

}