
#include "tcp_application.h"
#include "tcp_connection.h"
#include "iostream"

namespace networking {

int TcpApplication::ConnectionClosedCallBack(TcpConnection* connection) {
    yolanda_msgx("ConnectionClosedCallBack");
    return 0;
}

int TcpApplication::MessageCallBack(TcpConnection* connection) {
    yolanda_msgx("MessageCallBack");
    std::cout << connection->input_buffer_->data_ << std::endl;
    return 0;
}

int TcpApplication::WriteCompletedCallBack(TcpConnection* connection) {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}

}