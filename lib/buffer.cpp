#include "buffer.h"
#include "common.h"

namespace networking {

const char *CRLF = "\r\n";

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

int Buffer::Append(void *data, int size) {
    if (data != nullptr) {
        MakeRoom(size);
        //拷贝数据到可写空间中
        memcpy(data_ + write_index_, data_, size);
        write_index_ += size;
    }
}

int Buffer::AppendChar(char data) {
    MakeRoom(1);
    data_[write_index_++] = data;
}

int Buffer::AppendString(char *data) {
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

char Buffer::ReadChar(struct buffer *buffer) {
    char c = data_[read_index_++];
    return c;
}

char *Buffer::FindCRLF() {
    char *crlf = (char *)memmem(data_ + read_index_, ReadableSize(), CRLF, 2);
    return crlf;
}

}