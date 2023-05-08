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
    TcpApplication(TcpConnection* connection, const std::string& name): connection_(connection), name_(name) {}

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

    TcpConnection(int connected_fd, EventLoop *event_loop) {
        Set(connected_fd, CHANNEL_EVENT_READ, static_cast<void *>(event_loop), "connection");
        input_buffer_ = std::make_shared<Buffer>();
        output_buffer_ = std::make_shared<Buffer>();
    }

    ~TcpConnection() {}

    void Init(std::shared_ptr<TcpApplication> application) {
        application_ = application;
        application_->ConnectionCompletedCallBack();
    }

    virtual int Close() override {
        return close(fd_);
    }

    virtual int EventReadCallback() override;
    virtual int EventWriteCallback() override;

    int HandleConnectionClosed();

    int SendData(const char *data, int size);
    int SendBuffer(const Buffer& buffer);
    void Shutdown();
    Buffer* GetInputBuffer() { return input_buffer_.get(); }

private:
    EventLoop* GetEventLoop() { return static_cast<EventLoop*>(data_); }

    std::string name_;
    std::shared_ptr<Buffer> input_buffer_;   //接收缓冲区
    std::shared_ptr<Buffer> output_buffer_;  //发送缓冲区

    std::shared_ptr<TcpApplication> application_;  // Tcp上层应用，比如Http
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
