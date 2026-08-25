#pragma once

#include <string>
#include <vector>
#include <utility>

namespace stdext::system {
    using pipe_result = std::pair<int, std::string>;

    struct fork_result {
#ifdef _WIN32
        void* process;
        void* thread;

        unsigned long id;
#else
        int pid;
#endif
    };

    int exec(const std::string& command);
    pipe_result exec_pipe(const std::string& command);

    fork_result fork(const std::string& program, const std::vector<std::string>& arguments = {});
    int wait(const fork_result& process);

    std::string username();
    std::string hostname();

    char* get_env(const std::string& name);
    std::string get_env(const std::string& name, const std::string& default_val);

    void set_env(const std::string& name, const std::string& value);
    void remove_env(const std::string& name);
}
