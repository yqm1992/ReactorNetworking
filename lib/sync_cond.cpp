#include "sync_cond.h"

namespace networking {

void SyncCond::Notify(const std::string& msg) {
  std::unique_lock<std::mutex> lock(mutex_);
  flag_ = true;
  message_ = msg;
  cond_.notify_one();
}

void SyncCond::NotifyStop() {
  std::unique_lock<std::mutex> lock(mutex_);
  stop_ = true;
  cond_.notify_one();
}

bool SyncCond::Wait(std::string* msg) {
  std::unique_lock<std::mutex> lock(mutex_);
  while (! stop_ && ! flag_) {
    cond_.wait(lock);
  }
  if (stop_) {
    return false;
  }
  flag_ = false;
  if (msg != nullptr) {
    *msg = message_;
  }
  return true;
}

void SyncCond::ClearFlag() {
  std::unique_lock<std::mutex> lock(mutex_);
  flag_ = false;
  message_.clear();
}
}