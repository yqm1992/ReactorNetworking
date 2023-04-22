#include "http_server.h"

#include <iostream>

namespace networking {

HttpApplication  HttpServer::HTTP_APPLICATION;

int HttpApplication::ConnectionCompletedCallBack(TcpConnection* connection) {
    yolanda_msgx("ConnectionCompletedCallBack");
    return 0;
}

int HttpApplication::ConnectionClosedCallBack(TcpConnection* connection) {
    yolanda_msgx("ConnectionClosedCallBack");
    return 0;
}

int HttpApplication::MessageCallBack(TcpConnection* connection) {
    yolanda_msgx("MessageCallBack");
    std::cout << connection->GetInputBuffer()->ReadStartPos() << std::endl;
    connection->SendData("Hello world\n", 13);
    return 0;
}

int HttpApplication::WriteCompletedCallBack(TcpConnection* connection) {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}


}