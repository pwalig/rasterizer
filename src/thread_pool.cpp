#include "thread_pool.hpp"

thd::thread_pool::thread_pool(size_t num_threads)
{
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
                    cv.wait(lock, [this] {
                        return !jobs.empty() || stop;
                        });

                    // exit the thread in case the pool is stopped and there are no tasks
                    if (stop && jobs.empty()) {
                        return;
                    }

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

thd::thread_pool::~thread_pool()
{
    {
        // Lock the queue to update the stop flag safely
        std::unique_lock<std::mutex> lock(mutex_);
        stop = true;
    }

    // Notify all threads
    cv.notify_all();

    // Joining all worker threads to ensure they have completed their tasks
    for (std::thread& thread : threads) {
        thread.join();
    }
}

void thd::thread_pool::enque(std::function<void()>&& task)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        jobs.emplace(task);
    }
    cv.notify_one();
}

size_t thd::thread_pool::thread_count() const
{
    return threads.size();
}

void thd::thread_pool::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cvf.wait(lock, [this] {
        return !(this->busy());
        });
}

bool thd::thread_pool::busy() {
    if (!jobs.empty()) return true;
    for (const bool& t : running) {
        if (t) return true;
    }
    return false;
}