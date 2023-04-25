#include "http_response.h"
#include "common.h"

#define INIT_RESPONSE_HEADER_SIZE 128

namespace networking {

void HttpResponse::EncodeBuffer(Buffer *output) {
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    output->AppendString(buf);
    output->AppendString(buf);
    output->AppendString(status_message_.c_str());
    output->AppendString("\r\n");

    if (keep_connected_) {
        output->AppendString("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", strlen(body_.c_str()));
        output->AppendString(buf);
        output->AppendString("Connection: Keep-Alive\r\n");
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

}