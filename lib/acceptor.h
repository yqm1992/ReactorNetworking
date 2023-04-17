#pragma once

#include "common.h"

namespace networking {

class Acceptor {
public:
    Acceptor(int port): listen_port_(port) {}   

    bool Init();

    int GetFD() {}

    static void MakeNonblocking(int fd);

private:
    int listen_port_;
    int listen_fd_;
} ;

}