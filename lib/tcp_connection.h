#pragma once

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

namespace networking {

class TcpApplication;
class TcpConnection;

class TcpApplication {
public:
    TcpApplication(const std::string& name): name_(name) {}

    TcpApplication(const std::string& name, TcpConnection* connection): name_(name) {
        SetTcpConnection(connection);
    }

    void SetTcpConnection(TcpConnection* connection) {
        connection_ = connection;
    }

    virtual ~TcpApplication() {}

    virtual int ConnectionCompletedCallBack() { return 0; }
    virtual int ConnectionClosedCallBack() { return 0; }
    virtual int MessageCallBack() { return 0; }
    virtual int WriteCompletedCallBack() { return 0; }

protected:
    TcpConnection* connection_ = nullptr;
    std::string name_;
};

class TcpConnection: public Channel {
public:
    friend class Acceptor;
    friend class TcpApplication;

    TcpConnection(int connected_fd) {
        Set(connected_fd, CHANNEL_EVENT_READ, "connection");
        input_buffer_ = std::make_shared<Buffer>();
        output_buffer_ = std::make_shared<Buffer>();
    }

    ~TcpConnection() { Close(); }

    static std::shared_ptr<Channel> MakeTcpConnectionChannel(int connected_fd, std::shared_ptr<TcpApplication> tcp_application);

    void Init(std::shared_ptr<TcpApplication> application) {
        application->SetTcpConnection(this);
        application_ = application;
        application_->ConnectionCompletedCallBack();
    }

    virtual int Close() override {
        // 执行Close前的回调函数
        HandleConnectionClosed();
        return close(fd_);
    }

    virtual int EventReadCallback() override;
    virtual int EventWriteCallback() override;

    int HandleConnectionClosed();

    int SendData(const char *data, int size);
    int SendData(const std::string& data);
    int SendBuffer(const Buffer& buffer);
    void Shutdown();
    Buffer* GetInputBuffer() { return input_buffer_.get(); }

private:

    void FocusWriteEvent();

    void CancelFocusWriteEvent();

    std::string name_;
    std::shared_ptr<Buffer> input_buffer_;   //接收缓冲区
    std::shared_ptr<Buffer> output_buffer_;  //发送缓冲区

    std::shared_ptr<TcpApplication> application_;  // Tcp上层应用，比如Http
    bool closed_callback_executed_ = false;
};

}

// EventReadCallback
// SocketRead
// MessageCallBack

// TcpConnection::SendData
// write
// Buffer::Append
// TcpConnection::EnableWriteEvent
// EventLoop::UpdataChannel

// EventWriteCallback
// Buffer::SocketWrite
// TcpConnection::DisableWriteEvent
// EventLoop::UpdateChannelEvent
// WriteCompletedCallBack
