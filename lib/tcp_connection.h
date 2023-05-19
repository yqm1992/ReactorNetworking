#pragma once

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

namespace networking {

class TcpApplication;
class TcpConnection;

// worker 线程调用，处理业务包括Decoding Computing Encoding
// 根据
class TcpApplication {
public:
    friend class TcpConnection;

    TcpApplication(const std::string& name): name_(name) {}

    TcpApplication(const std::string& name, TcpConnection* connection): name_(name) {
        SetTcpConnection(connection);
    }

    virtual ~TcpApplication() {}

    void AppendBuffer(const Buffer& buffer) { buffer_.AppendBuffer(buffer); }

    // 返回需要发送的数据
    virtual std::shared_ptr<Buffer> Process();

    virtual int ConnectionCompletedCallBack() { return 0; }
    virtual int ConnectionClosedCallBack() { return 0; }
    virtual int MessageCallBack() { return 0; }

    // 应该放在EventLoop线程中处理
    virtual int WriteCompletedCallBack() { return 0; }

protected:

    void SetTcpConnection(TcpConnection* connection) {
        connection_ = connection;
    }

    TcpConnection* connection_ = nullptr;
    Buffer buffer_;
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

    // 这里的接口需要考虑到在其他线程（非EventLoop线程）调用的情况，否则 decoding-computing-encoding 只能在EventLoop线程中和IO串行执行
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

/*
read-> decoding -> computing -> encoding -> write

Reactor + ThreadPool

OnMessage后，可以把input_buffer_中的数据取出来（或者启用一个新的buffer），和connection本身一起封装成一个task，交给worker线程池来处理
woker线程处理完task后，调用connection接口发送数据（把消息挂到EventLoop的事件队列里，由EventLoop线程完成实际的数据发送）


TcpConnection input_message

worker线程，需要为每个连接，保留数据

OnMessage() {
    Task task(this, input_buffer_); // 封装数据到task中
    input_buffer_ = std::make_shared<Buffer>(); // 更新buffer
    WrokerThread* worker = GetWorker(GetFD()); // 找到对应的worker
    worker->push(task); // worker处理
    return;
}

WorkerThread {
public:
    struct Task {
        TaskType type; // established, normal, closed, 
        std::shared_ptr<TcpConnection> connection;
        std::shared_ptr<Buffer> data;
    };

void DoTask(const Task& task) {
    auto iter = map_.find(task.connection->GetFD());
    if (iter == map_.end()) {
        iter = map_.emplace(task.connection->GetFD(), NewTcpApplication);
    }
    TcpApplication* application = task.connection->GetApplication();
    application->Append(task.buffer); // 把task中的数据追加到application缓冲区
    std::shared_ptr<Buffer> out_data = application->Process(); // Decoding, Computing，Encoding，处理完毕后需要丢弃缓冲区, 
    if (out_data) {
        connection->Send(out_data); // 通知EventLoop线程发送数据
    }
    // EventLoopThread:  只能处理connection状态，不能处理Application数据
    // WorkerThread:     只能处理Connection::Application数据，不能改变connection的状态，想要修改的话，需要通知EventLoopThread去进行修改
}

void Work() {
    while (work_) {
        Task task = task_list_.front();
        task_list_.pop_front();
        DoTask(task);
    }
}
std::map<int, TcpApplication> map_;
std::list<Task> task_list_;
}
*/
