#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

namespace Runner {
    struct TestCase {
        std::string name;
        std::string command;
    };

    class TestRunner {
    private:
        std::vector<TestCase> m_tests;
        std::atomic<int> m_passed = 0;
        std::atomic<int> m_failed = 0;
        std::mutex m_console_mutex;

    public:
        void add(const TestCase& test);
        bool run(int jobs);
    };
}
