#include "channel.h"

namespace networking {


bool Channel::WriteEventIsEnabled() { 
    return events_ & CHANNEL_EVENT_WRITE; 
}

void Channel::EnableWriteEvent() { 
    events_ |= CHANNEL_EVENT_WRITE; 
}

void Channel::DisableWriteEvent() { 
    events_ &= ~CHANNEL_EVENT_WRITE; 
}

int Channel::EventReadCallback() { 
    return 0; 
}

int Channel::EventWriteCallback() { 
    return 0; 
}

}
