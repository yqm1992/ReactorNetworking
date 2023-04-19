#include <vector>
#include <memory>
#include "lib/tcp_server.h"

int main() {
    int thread_num = 4;
    int listen_port = 43211;
    networking::TcpServer tcp_server(thread_num, listen_port);
    tcp_server.Start();
    return 0;
}