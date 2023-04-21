#pragma once

#include <string>

namespace networking {

class TcpConnection;

class TcpApplication {
public:
    TcpApplication() {}

    virtual ~TcpApplication() {}

    virtual int ConnectionClosedCallBack(TcpConnection* connection);
    virtual int MessageCallBack(TcpConnection* connection);
    virtual int WriteCompletedCallBack(TcpConnection* connection);

private:
    std::string name_;
};

}