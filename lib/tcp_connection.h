#pragma once

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

namespace networking {

class TcpApplication;
class TcpConnection;

// // worker 线程调用，处理业务包括Decoding Computing Encoding
// // 根据
// class TcpApplication {
// public:
//     friend class TcpConnection;

//     TcpApplication(const std::string& name): name_(name) {}

//     TcpApplication(const std::string& name, TcpConnection* connection): name_(name) {
//         SetTcpConnection(connection);
//     }

//     virtual ~TcpApplication() {}

//     void AppendBuffer(const Buffer& buffer) { buffer_.AppendBuffer(buffer); }

//     // 返回需要发送的数据
//     virtual std::shared_ptr<Buffer> ApplicationLayerProcess() = 0;

//     virtual int ConnectionCompletedCallBack() { return 0; }
//     virtual int ConnectionClosedCallBack() { return 0; }
//     virtual int MessageCallBack() { return 0; }

//     // 应该放在EventLoop线程中处理
//     virtual int WriteCompletedCallBack() { return 0; }

// protected:

//     void SetTcpConnection(TcpConnection* connection) {
//         connection_ = connection;
//     }

//     TcpConnection* connection_ = nullptr;
//     Buffer buffer_;
//     std::string name_;
// };

class TcpConnection: public Channel {
public:
    friend class Acceptor;
    // friend class TcpApplication;

    TcpConnection(int connected_fd, const std::string& name = "connection", WorkThread* worker = nullptr) {
        Set(connected_fd, CHANNEL_EVENT_READ, name);
        input_buffer_ = std::make_shared<Buffer>();
        output_buffer_ = std::make_shared<Buffer>();
        worker_ = worker;
    }

    virtual ~TcpConnection() { Close(); }

    // static std::shared_ptr<Channel> MakeTcpConnectionChannel(int connected_fd, std::shared_ptr<TcpApplication> tcp_application);

    // void Init(std::shared_ptr<TcpApplication> application) {
    //     application->SetTcpConnection(this);
    //     application_ = application;
    //     application_->ConnectionCompletedCallBack();
    // }

    int Close() {
        // 执行Close前的回调函数
        HandleConnectionClosed();
        return close(fd_);
    }

    int EventReadCallback() override;
    int EventWriteCallback() override;

    int HandleConnectionClosed();

    // 这里的接口需要考虑到在其他线程（非EventLoop线程）调用的情况，否则 decoding-computing-encoding 只能在EventLoop线程中和IO串行执行
    // int SendData(const char *data, int size);
    void SendString(std::shared_ptr<std::string> data);
    void SendBuffer(std::shared_ptr<Buffer> buffer);
    void Shutdown();
    Buffer* GetInputBuffer() { return input_buffer_.get(); }

    bool InLoopThread() { return event_loop_->InOwnerThread(); }

    virtual int WriteCompletedCallBack() { return 0; }
    virtual int ConnectionCompletedCallBack() { return 0; }
    virtual int ConnectionClosedCallBack() { return 0; }
    // 处理input_buffer_的回调函数（数据读取到了input_buffer_之后执行）
    virtual int MessageCallBack(std::shared_ptr<Buffer> message_buffer) { return 0; }

protected:

    void FocusWriteEvent();

    void CancelFocusWriteEvent();

    void SendDataInLoop(const char *data, int size);

    void SendBufferInLoop(std::shared_ptr<Buffer> buffer);

    void SendStringInLoop(std::shared_ptr<std::string> str);

    void ShutdownInLoop();

    std::string name_;
    std::shared_ptr<Buffer> input_buffer_;   //接收缓冲区
    std::shared_ptr<Buffer> output_buffer_;  //发送缓冲区

    // std::shared_ptr<TcpApplication> application_;  // Tcp上层应用，比如Http
    bool closed_callback_executed_ = false;
    WorkThread* worker_ = nullptr;
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

void RunTaskInLoopThread(const Task& task) {
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
        RunTaskInLoopThread(task);
    }
}
std::map<int, TcpApplication> map_;
std::list<Task> task_list_;
}
*/

/*
EventLoop被唤醒后，处理connection 的事件类型：

套接字可读（从套接字读取数据，写入到input_buffer_）
    读取到0字节，设置套接字状态为closed，不会再向套接字写入数据（直接将套接字移出关注列表）

套接字可写（读取output_buffer_的数据，写入套接字）
    如果状态为closed，不需要再发送数据，直接丢弃out_buffer_
    如果状态为closing（不再向buffer追加数据），但是要把已经在buffer_中的数据继续处理完毕，处理完之后，真正执行ShutDown
    Normal，正常发送数据，处理完之后，需要更新channel，关闭关注写

应用层发送数据（写入数据到output_buffer_）
    如果状态不为normal，直接丢弃数据，并打印日志
    

应用层要求关闭写
    如果状态为normal，则设置closing，否则维持不变
    ShutDown成功
    
*/
