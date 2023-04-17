#include <assert.h>

#include "acceptor.h"

namespace networking {

void Acceptor::MakeNonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

bool Acceptor::Init() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    MakeNonblocking(listen_fd_);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(listen_port_);

    int on = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listen_fd_, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        // error(1, errno, "bind failed ");
        return false;
    }

    int rt2 = listen(listen_fd_, LISTENQ);
    if (rt2 < 0) {
        // error(1, errno, "listen failed ");
        return false;
    }
    //    signal(SIGPIPE, SIG_IGN);
    return true;
}

}