#include "lib/echo_service.h"

int main() {
    networking::EchoClient client("127.0.0.1", 43211);
    client.Connect();
    return 0;
}