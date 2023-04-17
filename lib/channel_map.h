#pragma once

//#include "common.h"
//#include "event_loop.h"
//#include "buffer.h"

namespace networking {

// channel映射表, key为对应的socket描述字
class ChannelMap {
public:
    ChannelMap(): entries_(nullptr), nentries_(0) {}

private:
    void **entries_;
    /* The number of entries available in entries */
    int nentries_;
};

}


