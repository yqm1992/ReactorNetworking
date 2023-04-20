#pragma once

#define INIT_BUFFER_SIZE 65536

namespace networking {

// 可以考虑做成虚类，方便兼容不同类型的buffer
class Buffer {
public:
    friend class TcpConnection;

    Buffer(): read_index_(0), write_index_(0) {
        data_ = new char[INIT_BUFFER_SIZE];
        total_size_ = INIT_BUFFER_SIZE;
    }

    ~Buffer() { delete data_; }
    
    int WritableSize() { return total_size_ - write_index_; }

    int ReadableSize() { return write_index_ - read_index_; }

    int FrontSpareSize() { return read_index_; }

    void MakeRoom(int size);

    int Append(void *data, int size);

    int AppendChar(char data);

    int AppendString(char *data);

    int SocketRead(int fd);

    char ReadChar(struct buffer *buffer);
    
    char *FindCRLF();

private:
    char *data_;
    int read_index_;       //缓冲读取位置
    int write_index_;      //缓冲写入位置
    int total_size_;      //总大小
};

}


