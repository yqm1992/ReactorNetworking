#include "lib/echo_service.h"

using namespace networking;

int main() {
    int thread_num = 2;
    int listen_port = 43211;

    EchoServer echo_server(thread_num, listen_port);
    echo_server.Start();

    return 0;
}
