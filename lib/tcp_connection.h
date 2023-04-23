#pragma once

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

namespace networking {

class TcpApplication;

class TcpConnection: public Channel {
public:
    friend class TcpApplication;

    TcpConnection(int connected_fd, EventLoop *event_loop, TcpApplicationFactory* application_factory) {
        Set(connected_fd, CHANNEL_EVENT_READ, static_cast<void *>(event_loop), "connection");
        input_buffer_ = std::make_shared<Buffer>();
        output_buffer_ = std::make_shared<Buffer>();
        application_factory_ = application_factory;

        // char *buf = malloc(16);
        // sprintf(buf, "connection-%d\0", connected_fd);
        // tcpConnection->name = buf;
    }

    ~TcpConnection() {}

    void Init() {
        application_ = application_factory_->MakeTcpApplication(this);
        application_->ConnectionCompletedCallBack();
    }

    static std::shared_ptr<Channel> MakeChannel(int connected_fd, EventLoop *event_loop, TcpApplicationFactory* application_factory);

    virtual int EventReadCallback() override;
    virtual int EventWriteCallback() override;

    int HandleConnectionClosed();

    int SendData(const char *data, int size);
    int SendBuffer(Buffer *buffer);
    void Shutdown();
    Buffer* GetInputBuffer() { return input_buffer_.get(); }

private:
    EventLoop* GetEventLoop() { return static_cast<EventLoop*>(data_); }

    std::string name_;
    std::shared_ptr<Buffer> input_buffer_;   //接收缓冲区
    std::shared_ptr<Buffer> output_buffer_;  //发送缓冲区

    std::shared_ptr<TcpApplication> application_;  // 上层应用
    TcpApplicationFactory* application_factory_; // Tcp上层应用数据，比如Http

    // void * data; //for callback use: http_server
    // void * request; // for callback use
    // void * response; // for callback use
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
