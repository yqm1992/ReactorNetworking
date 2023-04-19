#include <vector>
#include <memory>
#include "lib/tcp_server.h"

int main() {
    int thread_num = 2;
    int listen_port = 43211;
    networking::TcpServer thread_pool(thread_num, listen_port);
    thread_pool.Start();
    return 0;
}