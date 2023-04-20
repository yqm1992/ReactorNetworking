#pragma once

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
// #include "tcp_server.h"

namespace networking {

class TcpConnection: public Channel {
public:
    TcpConnection(int connected_fd, EventLoop *event_loop) {
        Set(connected_fd, CHANNEL_EVENT_READ, static_cast<void *>(event_loop), "connection");
        input_buffer_ = std::make_shared<Buffer>();
        output_buffer_ = std::make_shared<Buffer>();

        // char *buf = malloc(16);
        // sprintf(buf, "connection-%d\0", connected_fd);
        // tcpConnection->name = buf;
    }

    ~TcpConnection() {}

    // bool Init();

    static std::shared_ptr<Channel> MakeChannel(int connected_fd, EventLoop *event_loop) {
        std::shared_ptr<Channel> channel;
        TcpConnection* connection = new TcpConnection(connected_fd, event_loop);
        channel.reset(static_cast<Channel*>(connection));
        return channel;
    }

    virtual int EventReadCallback() override;
    virtual int EventWriteCallback() override;

    int HandleConnectionClosed();

    int SendData(void *data, int size);
    int SendBuffer(Buffer *buffer);
    void Shutdown();

    virtual int ConnectionClosedCallBack();
    virtual int MessageCallBack();
    virtual int WriteCompletedCallBack() { return 0; };

private:
    EventLoop* GetEventLoop() { return static_cast<EventLoop*>(data_); }

    std::string name_;
    std::shared_ptr<Buffer> input_buffer_;   //接收缓冲区
    std::shared_ptr<Buffer> output_buffer_;  //发送缓冲区

    // void * data; //for callback use: http_server
    // void * request; // for callback use
    // void * response; // for callback use
};

}