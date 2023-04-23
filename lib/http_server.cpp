#include "http_server.h"

#include <iostream>

namespace networking {

HttpApplicationFactory HttpServer::HTTP_APPLICATION_FACTORY;

int HttpApplication::ConnectionCompletedCallBack() {
    yolanda_msgx("ConnectionCompletedCallBack");
    return 0;
}

int HttpApplication::ConnectionClosedCallBack() {
    yolanda_msgx("ConnectionClosedCallBack");
    return 0;
}

int HttpApplication::MessageCallBack() {
    yolanda_msgx("MessageCallBack");
    std::cout << connection_->GetInputBuffer()->ReadStartPos() << std::endl;
    connection_->SendData("Hello world\n", 13);
    return 0;
}

int HttpApplication::WriteCompletedCallBack() {
    yolanda_msgx("WriteCompletedCallBack");
    return 0;
}


}