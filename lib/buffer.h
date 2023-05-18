#pragma once

#include "common.h"

#define INIT_BUFFER_SIZE 65536

namespace networking {

// 可以考虑做成虚类，方便兼容不同类型的buffer
class Buffer {
public:
    friend class TcpConnection;
    friend class TcpApplication;

    Buffer(): read_index_(0), write_index_(0) {
        data_ = new char[INIT_BUFFER_SIZE];
        total_size_ = INIT_BUFFER_SIZE;
    }

    ~Buffer() { delete data_; }

    char* ReadStartPos() const { return data_ + read_index_; }

    void DiscardReadableData(int size) { read_index_ += std::min(ReadableSize(), size); }

    int ReadableSize() const { return write_index_ - read_index_; }

    int Append(const char *data, int size);

    int AppendString(const char *data);

    int SocketRead(int fd);

    ssize_t SocketWrite(int fd);

private:

    int WritableSize() const { return total_size_ - write_index_; }

    int AppendChar(char data);

    char ReadChar();

    void MakeRoom(int size);

    int FrontSpareSize() { return read_index_; }

    char *data_;
    int read_index_;       //缓冲读取位置
    int write_index_;      //缓冲写入位置
    int total_size_;      //总大小
};

}


