#include <iostream>
#include "tcp_connection.h"

namespace networking {
// TODO: 想一下TcpConnection的生命周期是怎样的

// std::shared_ptr<Channel> TcpConnection::MakeTcpConnectionChannel(int connected_fd, std::shared_ptr<TcpApplication> tcp_application) {
//     // 新建tcp_connction
//     TcpConnection* tcp_connection = new TcpConnection(connected_fd);
//     // 用tcp_application初始化tcp_connection
//     tcp_connection->Init(tcp_application);

//     std::shared_ptr<Channel> tcp_connection_channel;
//     tcp_connection_channel.reset(static_cast<Channel*>(tcp_connection));

//     return tcp_connection_channel;
// }

void TcpConnection::FocusWriteEvent() {
    EnableWriteEvent();
    event_loop_->UpdateChannelEvent(fd_);
}

void TcpConnection::CancelFocusWriteEvent() {
    DisableWriteEvent();
    event_loop_->UpdateChannelEvent(fd_);
}

// 从socket读取数据，写入buffer
int TcpConnection::EventReadCallback() {
    if (input_buffer_->SocketRead(fd_) > 0) {
        //应用程序真正读取Buffer里的数据
        std::shared_ptr<Buffer> message_buffer = input_buffer_;
        input_buffer_ = std::make_shared<Buffer>();
        // 发送到worker线程去处理
        if (worker_) {
            worker_->Push(std::bind(&TcpConnection::MessageCallBack, this, message_buffer));
        } else {
            MessageCallBack(message_buffer);
        }
    } else {
		// 后续的read都会返回0
        // 通知EventLoop删除该channel，在删除该连接之前会执行HandleConnectionClosed()
        MarkRecycle();
    }
    return 0;
}

// EventReadCallback中调用，处理connection关闭的情况
int TcpConnection::HandleConnectionClosed() {
    if (closed_callback_executed_) {
        return 0;
    }
    ConnectionClosedCallBack();
    closed_callback_executed_ = true;
    return 0;
    // Shutdown();
}

// 从output_buffer 读取数据，然后写入到socket
int TcpConnection::EventWriteCallback() {
    // TODO: check
    // assertInSameThread(eventLoop);

    ssize_t writed_socket_size = output_buffer_->SocketWrite(fd_);
    if (writed_socket_size > 0) {
        //如果数据完全发送出去，就不需要继续了
        if (output_buffer_->ReadableSize() == 0) {
            CancelFocusWriteEvent();
        }
        //回调WriteCompletedCallBack
        WriteCompletedCallBack();
    } else {
        yolanda_msgx("HandleWrite for %s", GetDescription().c_str());
    }
}

//应用层调用入口
void TcpConnection::SendDataInLoop(const char *data, int size) {
    assert(InLoopThread());
    size_t writed_socket_size = 0;
    size_t left_size = size;
    bool fault = false;

    // 没有开启WRITE_EVENT && 输出缓冲区为空的情况下
    // 可以直接尝试套接字尝试发送数据
    if (!WriteEventIsEnabled() && output_buffer_->ReadableSize() == 0) {
        writed_socket_size = write(fd_, data, size);
        // writed_socket_size = write(fd_, data, 1); // 可以只发送少量数据，手动触发更新WriteEvent
        if (writed_socket_size >= 0) {
            left_size = left_size - writed_socket_size;
        } else {
            writed_socket_size = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = true;
                }
            }
        }
    }

    if (!fault && left_size > 0) {
        //拷贝到Buffer中，Buffer的数据由框架接管
        output_buffer_->Append(data + writed_socket_size, left_size);
        FocusWriteEvent();
    }

    return;
}

void TcpConnection::SendBufferInLoop(std::shared_ptr<Buffer> buffer) {
    SendDataInLoop(buffer->ReadStartPos(), buffer->ReadableSize());
}

void TcpConnection::SendStringInLoop(std::shared_ptr<std::string> str) {
    SendDataInLoop(str->c_str(), str->size());
}

void TcpConnection::SendString(std::shared_ptr<std::string> str) {
    event_loop_->RunTaskInLoopThread(std::bind(&TcpConnection::SendStringInLoop, this, str));
}

void TcpConnection::SendBuffer(std::shared_ptr<Buffer> buffer) {
    event_loop_->RunTaskInLoopThread(std::bind(&TcpConnection::SendBufferInLoop, this, buffer));
}

void TcpConnection::Shutdown() {
    event_loop_->RunTaskInLoopThread(std::bind(&TcpConnection::ShutdownInLoop, this));
}

void TcpConnection::ShutdownInLoop() {
    assert(InLoopThread());
    bool need_retry = false;
    // 数据全部写入到内核缓冲之后，才允许关闭写
    if (output_buffer_->ReadableSize() > 0) {
        yolanda_msgx("can't ShutDown TcpConnection, output_buffer_ is not empty");
        need_retry = true;
    } else if (shutdown(fd_, SHUT_WR) < 0) {
        yolanda_msgx("TcpConnectionShutdown failed, socket == %d", fd_);
        need_retry = true;
    }
    if (need_retry) {
        event_loop_->QueueInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}



}
