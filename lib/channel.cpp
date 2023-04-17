#include "channel.h"

namespace networking {

Channel::Channel(int fd, int events, void *data) {}

bool Channel::WriteEventIsEnabled() { 
    return false; 
}

bool Channel::EnableWriteEvent() { 
    return false; 
}

bool Channel::DisableWriteEvent() { 
    return false; 
}

int Channel::EventReadCallback() { 
    return 0; 
}

int Channel::EventWriteCallback() { 
    return 0; 
}

}
