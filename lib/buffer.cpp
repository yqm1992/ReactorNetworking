#include "buffer.h"

namespace networking {

void Buffer::MakeRoom(int size) {
    if (WritableSize() >= size) {
        return;
    }
    //如果front_spare和writeable的大小加起来可以容纳数据，则把可读数据往前面拷贝
    if (FrontSpareSize() + WritableSize() >= size) {
        int readable_size = ReadableSize();
        for (int i = 0; i < readable_size; i++) {
            memcpy(data_ + i, data_ + read_index_ + i, 1);
        }
        read_index_ = 0;
        write_index_ = readable_size;
    } else {
        //扩大缓冲区
        void *tmp = realloc(data_, total_size_ + size);
        if (tmp == nullptr) {
            return;
        }
        data_ = (char *)tmp;
        total_size_ += size;
    }
}

int Buffer::Append(const char *data, int size) {
    if (data != nullptr) {
        MakeRoom(size);
        //拷贝数据到可写空间中
        memcpy(data_ + write_index_, data, size);
        write_index_ += size;
    }
}

int Buffer::AppendChar(char data) {
    MakeRoom(1);
    data_[write_index_++] = data;
}

int Buffer::AppendString(const char *data) {
    if (data != nullptr) {
        int size = strlen(data);
        Append(data, size);
    }
}

int Buffer::SocketRead(int fd) {
    char additional_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];
    int max_writable = WritableSize();
    vec[0].iov_base = data_ + write_index_;
    vec[0].iov_len = max_writable;
    vec[1].iov_base = additional_buffer;
    vec[1].iov_len = sizeof(additional_buffer);
    int result = readv(fd, vec, 2);
    if (result < 0) {
        return -1;
    } else if (result <= max_writable) {
        write_index_ += result;
    } else {
        write_index_ = total_size_;
        Append(additional_buffer, result - max_writable);
    }
    return result;
}

ssize_t Buffer::SocketWrite(int fd) {
    ssize_t writed_socket_size = write(fd, data_ + read_index_, ReadableSize());
    if (writed_socket_size > 0) {
        //已读writed_socket_size字节
        read_index_ += writed_socket_size;
        yolanda_msgx("socket write size: %d", writed_socket_size);
    }
    return writed_socket_size;
}

char Buffer::ReadChar() {
    char c = data_[read_index_++];
    return c;
}

}
