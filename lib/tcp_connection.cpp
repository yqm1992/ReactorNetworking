#include "tcp_connection.h"

namespace networking {
// TODO: 想一下TcpConnection的生命周期是怎样的

int TcpConnection::HandleConnectionClosed() {
    ConnectionClosedCallBack();
    GetEventLoop()->RemoveChannel(fd_);
    Shutdown();
}

int TcpConnection::ConnectionClosedCallBack() {
    yolanda_msgx("close %s", GetDescription().c_str());
    return 0;
}

int TcpConnection::MessageCallBack() {
    yolanda_msgx("read from %s, %s", GetDescription().c_str(), input_buffer_->data_);
    return 0;
}

int TcpConnection::EventReadCallback() {
    if (input_buffer_->SocketRead(fd_) > 0) {
        //应用程序真正读取Buffer里的数据
        MessageCallBack();
    } else {
        HandleConnectionClosed();
    }
    return 0;
}

//发送缓冲区可以往外写
//把channel对应的output_buffer不断往外发送
int TcpConnection::EventWriteCallback() {
    // TODO: check
    // assertInSameThread(eventLoop);

    ssize_t nwrited = write(fd_, output_buffer_->data_ + output_buffer_->read_index_, output_buffer_->ReadableSize());
    if (nwrited > 0) {
        //已读nwrited字节
        output_buffer_->read_index_ += nwrited;
        //如果数据完全发送出去，就不需要继续了
        if (output_buffer_->ReadableSize() == 0) {
            DisableWriteEvent();
        }
        //回调WriteCompletedCallBack
        WriteCompletedCallBack();
    } else {
        yolanda_msgx("HandleWrite for tcp connection %s", name_.c_str());
    }

}

//应用层调用入口
int TcpConnection::SendData(void *data, int size) {
    size_t nwrited = 0;
    size_t nleft = size;
    int fault = 0;

    //先往套接字尝试发送数据
    if (!WriteEventIsEnabled() && output_buffer_->ReadableSize() == 0) {
        nwrited = write(fd_, data, size);
        if (nwrited >= 0) {
            nleft = nleft - nwrited;
        } else {
            nwrited = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }

    if (!fault && nleft > 0) {
        //拷贝到Buffer中，Buffer的数据由框架接管
        output_buffer_->Append(data + nwrited, nleft);
        if (!WriteEventIsEnabled()) {
            EnableWriteEvent();
        }
    }

    return nwrited;
}

int TcpConnection::SendBuffer(Buffer *buffer) {
    int size = buffer->ReadableSize();
    int result = SendData(buffer->data_ + buffer->read_index_, size);
    buffer->read_index_ += size;
    return result;
}

void TcpConnection::Shutdown() {
    if (shutdown(fd_, SHUT_WR) < 0) {
        yolanda_msgx("TcpConnectionShutdown failed, socket == %d", fd_);
    }
}

}