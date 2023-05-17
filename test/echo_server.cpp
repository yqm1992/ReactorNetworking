#include "lib/echo_service.h"

using namespace networking;

int main() {
    int thread_num = 2;
    int listen_port = 43211;

    EchoServer echo_server(thread_num, listen_port);
    echo_server.Start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
