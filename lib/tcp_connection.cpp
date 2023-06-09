#include <iostream>
#include "tcp_connection.h"

namespace networking {
// TODO: 想一下TcpConnection的生命周期是怎样的

std::shared_ptr<Channel> TcpConnection::MakeTcpConnectionChannel(int connected_fd, std::shared_ptr<TcpApplication> tcp_application) {
    // 新建tcp_connction
    TcpConnection* tcp_connection = new TcpConnection(connected_fd);
    // 用tcp_application初始化tcp_connection
    tcp_connection->Init(tcp_application);

    std::shared_ptr<Channel> tcp_connection_channel;
    tcp_connection_channel.reset(static_cast<Channel*>(tcp_connection));

    return tcp_connection_channel;
}

void TcpConnection::FocusWriteEvent() {
    if (!WriteEventIsEnabled()) {
        EnableWriteEvent();
        GetEventLoop()->UpdateChannelEvent(fd_);
    }
}

void TcpConnection::CancelFocusWriteEvent() {
    if (WriteEventIsEnabled()) {
        DisableWriteEvent();
        GetEventLoop()->UpdateChannelEvent(fd_);
    }
}

// 从socket读取数据，写入buffer
int TcpConnection::EventReadCallback() {
    if (input_buffer_->SocketRead(fd_) > 0) {
        //应用程序真正读取Buffer里的数据
        application_->MessageCallBack();
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
    application_->ConnectionClosedCallBack();
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
        application_->WriteCompletedCallBack();
    } else {
        yolanda_msgx("HandleWrite for %s", GetDescription().c_str());
    }
}

//应用层调用入口
int TcpConnection::SendData(const char *data, int size) {
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

    return writed_socket_size;
}

int TcpConnection::SendData(const std::string& data) {
    return SendData(data.c_str(), data.size());
}

// 不会改变buffer
int TcpConnection::SendBuffer(const Buffer& buffer) {
    return SendData(buffer.ReadStartPos(), buffer.ReadableSize());
}

void TcpConnection::Shutdown() {
    if (shutdown(fd_, SHUT_WR) < 0) {
        yolanda_msgx("TcpConnectionShutdown failed, socket == %d", fd_);
    }
}

}
