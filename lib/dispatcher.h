#pragma once

#include <memory>
#include "channel.h"

namespace networking {

/** 抽象的Dispatcher结构体，对应的实现如select,poll,epoll等I/O复用. */
class Dispatcher {
public:

    Dispatcher(const std::string& name): name_(name) {}

    virtual ~Dispatcher() {};

    virtual bool Init(void* data) = 0;

    /** 通知dispatcher新增一个channel事件*/
    virtual bool Add(const Channel& channel) = 0;

    /** 通知dispatcher删除一个channel事件*/
    virtual bool Del(const Channel& channel) = 0;

    /** 通知dispatcher更新channel对应的事件*/
    virtual bool Update(const Channel& channel) = 0;

    /** 实现事件分发，然后调用event_loop的event_activate方法执行callback*/
    virtual bool Dispatch(struct timeval *) = 0;

    /** 清除数据 */
    virtual void Clear() = 0;

protected:
    std::string name_;
};

}


