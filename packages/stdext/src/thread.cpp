#include "stdext/thread.hpp"
#include <exception>

namespace stdext {
    thread_pool::thread_pool(int workers) {
        start(workers);
    }

    void thread_pool::start(int workers) {
        for (int i = 0; i < workers; i++) {
            m_threads.emplace_back([this, i](std::stop_token stoken){
                worker(i, stoken);
            });
        }
    }

    void thread_pool::enqueue(task_func_t task) {
        {
            std::lock_guard lock(m_queue_mutex);
            m_task_queue.push(std::move(task));
        }

        m_condition.notify_one();
    }

    void thread_pool::wait() {
        std::unique_lock lock(m_queue_mutex);

        m_condition.wait(lock, [&] {
            return m_task_queue.empty() && m_active_tasks == 0;
        });
    }

    void thread_pool::join() {
        wait();

        for (auto& thread : m_threads) {
            thread.request_stop();
        }

        m_condition.notify_all();
        m_threads.clear();
    }

    void thread_pool::stop() {
        clear();

        for (auto& thread : m_threads) {
            thread.request_stop();
        }

        m_condition.notify_all();

        reset();
    }

    void thread_pool::clear() {
        {
            std::lock_guard lock(m_queue_mutex);

            while (!m_task_queue.empty()) {
                m_task_queue.pop();
            }
        }
    }

    void thread_pool::reset() {
        m_threads.clear();
    }



    void thread_pool::set_error_handler(error_handler_t handler) {
        if (is_running()) {
            return;
        }

        m_error_handler = handler;
    }

    bool thread_pool::is_running() const {
        return !m_threads.empty();
    }

    bool thread_pool::is_pending() const {
        std::lock_guard lock(m_queue_mutex);
        return !m_task_queue.empty() || m_active_tasks > 0;
    }


    void thread_pool::worker(int index, std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            auto task = task_func_t();

            {
                std::unique_lock lock(m_queue_mutex);

                m_condition.wait(lock, [&] {
                    return stoken.stop_requested() || !m_task_queue.empty();
                });

                if (stoken.stop_requested() && m_task_queue.empty()) {
                    return;
                }

                task = m_task_queue.front();
                m_task_queue.pop();
                m_active_tasks++;
            }

            try {
                task();
            }
            catch (...) {
                auto current = std::current_exception();

                if (m_error_handler) {
                    m_error_handler(index, current);
                }
                else {
                    std::rethrow_exception(current);
                }
            }

            {
                std::unique_lock lock(m_queue_mutex);

                m_active_tasks--;

                if (m_task_queue.empty() && m_active_tasks == 0) {
                    m_condition.notify_all();
                }
            }
        }
    }

    void thread::sleep(std::uint64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
}
