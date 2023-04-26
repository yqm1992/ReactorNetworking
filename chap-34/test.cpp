#include <vector>
#include <memory>
#include "lib/http_server.h"

using namespace networking;

int main() {
    int thread_num = 2;
    int listen_port = 43211;

    auto acceptor_channel = HttpAcceptor::MakeHttpAcceptorChannel(listen_port);
    networking::HttpServer http_server(thread_num);
    http_server.Start(acceptor_channel);
    return 0;
}

// 1、早上6:30点个卯，然后回去（不吃午饭），然后等到下午过来
// 2、早上8:50过来，到会议室坐到中午吃法的时候，然后