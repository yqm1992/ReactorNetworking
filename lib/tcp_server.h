#pragma once

#include "event_loop_thread.h"

namespace networking {

class TcpConnection;

class TcpApplication {
public:
    TcpApplication(TcpConnection* connection, const std::string& name): connection_(connection), name_(name) {}

    virtual ~TcpApplication() {}

    virtual int ConnectionCompletedCallBack() = 0;
    virtual int ConnectionClosedCallBack() = 0;
    virtual int MessageCallBack() = 0;
    virtual int WriteCompletedCallBack() = 0;

protected:
    TcpConnection* connection_ = nullptr;
    std::string name_;
};

class TcpApplicationFactory {
public:
    virtual ~TcpApplicationFactory() {};
    virtual std::shared_ptr<TcpApplication> MakeTcpApplication(TcpConnection* connection) = 0;
};


class TcpServer {
public:
    TcpServer(int thread_num, int listen_port, TcpApplicationFactory* application_factory): 
        thread_num_(std::max(1, thread_num)),
        listen_port_(listen_port),
        application_factory_(application_factory) {}

    void Start();

    EventLoop* SelectSubEventLoop();

    TcpApplicationFactory* GetTcpApplicationFactory() { return application_factory_; }

protected:

    std::shared_ptr<EventLoop> GetMainEventLoop() { return main_loop_thread_->GetEventLoop(); }

    // 创建thread_pool的主线程
    std::shared_ptr<networking::EventLoopThread> main_loop_thread_;
    // 是否已经启动
    int started_;
    // 线程数目
    int thread_num_;
    // 监听端口
    int listen_port_;
    // event_loop_thread数组
    std::vector<std::shared_ptr<networking::EventLoopThread>> sub_loop_threads_;

    // 当前选用的event_loop对应的index，用来决定选择哪个event_loop_thread服务
    int position_ = 0;
    // TcpApplication* application_ = nullptr;
    TcpApplicationFactory* application_factory_ = nullptr;
};

}