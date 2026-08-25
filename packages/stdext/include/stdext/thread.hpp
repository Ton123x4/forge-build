#pragma once

#include "stdext/logger.hpp"
#include "stdext/forge.hpp"

#include <functional>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace stdext {
    class thread_pool {
    public:
        using task_func_t = std::function<void()>;
        using error_handler_t = std::function<void(int, std::exception_ptr)>;

        thread_pool() = default;
        thread_pool(int workers);

        ~thread_pool() {
            if constexpr (stdext::is_debug) {
                if (is_pending()) {
                    stdext::warn("thread_pool destroyed with pending tasks. call join() before destruction");
                }
            }

            stop();
        }

        void start(int workers);
        void enqueue(task_func_t func);

        void wait();
        void join();
        void stop();

        void clear();
        void reset();

        void set_error_handler(error_handler_t handler);
        bool is_running() const;
        bool is_pending() const;

    private:
        error_handler_t m_error_handler = nullptr;
        std::vector<std::jthread> m_threads;
        std::condition_variable m_condition;
        std::atomic<int> m_active_tasks = 0;
        std::queue<task_func_t> m_task_queue;
        mutable std::mutex m_queue_mutex;

        void worker(int index, std::stop_token stoken);
    };

    namespace thread {
        void sleep(std::uint64_t milliseconds);
    }
}
