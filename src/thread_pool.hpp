#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace thd {

    class thread_pool {
    public:
        thread_pool(size_t num_threads = std::thread::hardware_concurrency());
        ~thread_pool();

        void enque(std::function<void()>&& task);

        size_t thread_count() const;

        // wait until thread_pool has finished all it's tasks
        void wait();

    private:
        // Vector to store worker threads
        std::vector<std::thread> threads;
        std::vector<bool> running;

        // Queue of tasks
        std::queue<std::function<void()>> jobs;

        // Mutex to synchronize access to shared data
        std::mutex mutex_;

        // Condition variable to signal changes in the state of the tasks queue
        std::condition_variable cv;

        // Condition variable to signal whether thread_pool has finished all the tasks
        std::condition_variable cvf;

        // Flag to indicate whether the thread pool should stop or not
        bool stop = false;


        // method is not thread safe
        // mutex_ should be aquired
        bool busy();
    };
}
