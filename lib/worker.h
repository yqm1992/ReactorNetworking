#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <functional>

#include <list>

class WorkThread {

public:
    typedef std::function<void()> Task;

    WorkThread(): thread_(nullptr), work_(true) {}

    ~WorkThread() { Stop(); }

    void Start() {
        thread_ = new std::thread(&WorkThread::Work, this);
    }

    void Stop() {
        if (thread_ != nullptr) {
            work_ = false;
            consumer_cond_.notify_one();
            thread_->join();
            delete thread_;
        }
    }

    void Push(Task task) {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push_back(task);
        consumer_cond_.notify_one();
    }

private:
    void Work() {
        while (work_) {
            Task task = [](){};
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (work_ && tasks_.empty()) {
                    consumer_cond_.wait(lock);
                }
                if (!work_) {
                    break;
                }
                task = tasks_.front();
                tasks_.pop_front();
            }
            task();
        }
    }

    std::thread * thread_;
    std::list<Task> tasks_;
    std::atomic_bool work_;

    std::mutex mutex_;
    std::condition_variable consumer_cond_;
    // std::condition_variable pruducer_cond_;
};
