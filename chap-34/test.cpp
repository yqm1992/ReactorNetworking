#include <vector>
#include <memory>
#include "lib/tcp_server.h"
#include "lib/tcp_application.h"

using namespace networking;

int main() {
    int thread_num = 2;
    int listen_port = 43211;
    std::shared_ptr<TcpApplication> application = std::make_shared<TcpApplication>();
    networking::TcpServer tcp_server(thread_num, listen_port, application);
    tcp_server.Start();
    return 0;
}