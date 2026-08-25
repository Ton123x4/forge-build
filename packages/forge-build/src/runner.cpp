#include "runner.h"

#include "stdext/logger.hpp"
#include "stdext/system.hpp"
#include "stdext/thread.hpp"
#include "stdext/time.hpp"

namespace Runner {
    void TestRunner::add(const TestCase& test) {
        m_tests.push_back(test);
    }

    bool TestRunner::run(int jobs) {
        auto pool = stdext::thread_pool();

        pool.start(jobs);

        for (const auto& test : m_tests) {
            pool.enqueue([this, test] {
                auto start_time = stdext::time::stopwatch();
                auto [exit_code, output] = stdext::system::exec_pipe(test.command);
                auto duration = start_time.elapsed_milliseconds();

                if (exit_code == 0) {
                    stdext::success("Test '{}': OK [{}ms]", test.name, duration);
                    m_passed++;
                }
                else {
                    stdext::error("Test '{}': FAILED [{}ms]", test.name, duration);
                    m_failed++;
                }

                if (!output.empty() && exit_code > 0) {
                    stdext::println("{}", output);
                }
            });
        }

        pool.wait();

        if (m_failed == 0) {
            stdext::success("Test result: {} passed | {} failed.", m_passed.load(), m_failed.load());
        }
        else {
            stdext::error("Test result: {} passed | {} failed.", m_passed.load(), m_failed.load());
        }

        return m_failed > 0;
    }
}
