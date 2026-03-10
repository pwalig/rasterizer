#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace rast {
    class thread_pool {
    public:
        inline thread_pool(size_t num_threads = std::thread::hardware_concurrency()) {
			threads.reserve(num_threads);
			running.reserve(num_threads);
			for (size_t i = 0; i < num_threads; ++i) {
				running.push_back(false);
				threads.emplace_back([this, i]() {
					while (true) {
						std::function<void()> job;
						{
							std::unique_lock<std::mutex> lock(mutex_);
							// Waiting until there is a task to execute or the pool is stopped
							cv.wait(lock, [this] { return !jobs.empty() || stop; });
							// exit the thread in case the pool is stopped and there are no tasks
							if (stop && jobs.empty()) { return; }
							// Get the next task from the queue
							job = std::move(jobs.front());
							jobs.pop();
							running[i] = true;
						}
						job();
						{
							std::unique_lock<std::mutex> lock(mutex_);
							running[i] = false;
						}
						cvf.notify_all();
					}
					});
			}
		}
        inline ~thread_pool() {
			{
				// Lock the queue to update the stop flag safely
				std::unique_lock<std::mutex> lock(mutex_);
				stop = true;
			}
			// Notify all threads
			cv.notify_all();
			// Joining all worker threads to ensure they have completed their tasks
			for (std::thread& thread : threads) { thread.join(); }
		}

        inline void enque(std::function<void()>&& task) {
			{
				std::unique_lock<std::mutex> lock(mutex_);
				jobs.emplace(task);
			}
			cv.notify_one();
		}

        inline size_t thread_count() const { return threads.size(); }

        // wait until thread_pool has finished all it's tasks
        inline void wait() {
			std::unique_lock<std::mutex> lock(mutex_);
			cvf.wait(lock, [this] {
				return !(this->busy());
				});
		}

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
        inline bool busy() {
			if (!jobs.empty()) return true;
			for (const bool& t : running) {
				if (t) return true;
			}
			return false;
		}
    };
}
