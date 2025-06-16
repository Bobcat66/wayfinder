#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <concepts>
#include <atomic>

namespace wf {
    class ThreadPool {
    public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        // Prevent copying and moving
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        // Enqueue a task and get a future to its result
        template<std::invocable F, typename... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
            using ReturnType = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<ReturnType> result = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                if (stop)
                    throw std::runtime_error("Enqueue on stopped ThreadPool");
                tasks.emplace([task]() { (*task)(); });
            }
            condition.notify_one();
            return result;
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        std::atomic<bool> stop;
    };
}