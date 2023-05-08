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