#include "http_response.h"
#include "common.h"

#include <iostream>

#define INIT_RESPONSE_HEADER_SIZE 128

namespace networking {

void HttpResponse::EncodeBuffer(Buffer *output) {
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    output->AppendString(buf);
    output->AppendString(status_message_.c_str());
    output->AppendString("\r\n");

    if (keep_connected_) {
        output->AppendString("Connection: close\r\n");
    } else {
        output->AppendString("Connection: Keep-Alive\r\n");
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", strlen(body_.c_str()));
        output->AppendString(buf);
        snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", content_type_.c_str());
        output->AppendString(buf);
    }

    for (auto& header: response_headers_) {
        output->AppendString(header.key.c_str());
        output->AppendString(": ");
        output->AppendString(header.value.c_str());
        output->AppendString("\r\n");
    }

    output->AppendString("\r\n");
    output->AppendString(body_.c_str());
}

void HttpResponse::Display() {
    std::string line_end = "\\r\\n\n";
    std::cout << "---------------- response start ----------------" << std::endl;
    char buf[32];

    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    std::cout << buf;
    std::cout << status_message_.c_str();
    std::cout << line_end;

    if (keep_connected_) {
        std::cout << "Connection: close" << line_end;
    } else {
        std::cout << "Connection: Keep-Alive" << line_end;
        snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", content_type_.c_str());
        std::cout << buf;
        snprintf(buf, sizeof buf, "Content-Length: %zd\\r\\n\n", strlen(body_.c_str()));
        std::cout << buf;
    }

    for (auto& header: response_headers_) {
        std::cout << header.key.c_str();
        std::cout << ": ";
        std::cout << header.value.c_str();
        std::cout << line_end;
    }

    std::cout << line_end;
    std::cout << body_.c_str();
    std::cout << std::endl << "---------------- response end ----------------" << std::endl;
}

}