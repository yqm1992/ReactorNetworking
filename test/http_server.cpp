#include "lib/http_layer.h"

using namespace networking;

int main() {
    int thread_num = 2;
    int listen_port = 43211;

    HttpServer http_server(thread_num, listen_port);
    http_server.Start();

    std::string str;
    while (true) {
        std::cin >> str;
        if (str == "exit") {
            break;
        }
    }

    return 0;
}
