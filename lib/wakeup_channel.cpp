#include "common.h"
#include "wakeup_channel.h"

namespace networking {

void WakeupChannel::Init(EventLoop* event_loop, int fd) {
    event_loop_ = event_loop;
    Set(fd, CHANNEL_EVENT_READ);
}


int WakeupChannel::EventReadCallback() {
    char one;
    ssize_t n = read(fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("HandleWakeup failed");
    }
    yolanda_msgx("Wakeup, %s", event_loop_->GetName().c_str());
    return 0;
}

}