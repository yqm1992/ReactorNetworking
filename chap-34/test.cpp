#include <vector>
#include <memory>
#include "lib/event_loop_thread.h"
#include "lib/event_loop_thread_pool.h"

int main() {
    // std::vector<std::shared_ptr<networking::EventLoopThread>> thread_pool;
    // for (int i = 0; i < 4; ++i) {
    //     auto cur_thread = std::make_shared<networking::EventLoopThread>(i);
    //     cur_thread->Start();
    //     thread_pool.push_back(cur_thread);
    // }
    networking::EventLoopThreadPool thread_pool(4);
    thread_pool.Start();
    return 0;
}
