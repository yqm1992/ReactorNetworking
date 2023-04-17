#pragma once

#include <mutex>
#include <condition_variable>

namespace networking {

class SyncCond {
 public:

  void Notify() { Notify(""); }

  bool Wait() { return Wait(nullptr); }

  void Notify(const std::string& msg);

  void NotifyStop();

  bool Wait(std::string* msg);

  void ClearFlag();

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  bool flag_ = false;
  bool stop_ = false;
  std::string message_;
};

}