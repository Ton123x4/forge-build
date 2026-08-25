#include <array>
#include <format>
#include <cstdlib>
#include <stdexcept>

#include "stdext/string.hpp"
#include "stdext/system.hpp"

#ifdef _WIN32
#include <windows.h>

#define popen  _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace stdext::system {
    int exec(const std::string& command) {
        return std::system(command.c_str());
    }

    pipe_result exec_pipe(const std::string& command) {
        auto cmd = std::format("{} 2>&1", command);
        auto pipe = popen(cmd.c_str(), "r");

        if (!pipe) {
            throw std::runtime_error("Failed to create pipe process.");
        }

        auto buffer = std::array<char, 4096>();
        auto result = std::string();

        while (true) {
            auto read = fread(buffer.data(), 1, buffer.size(), pipe);

            if (read == 0) {
                break;
            }

            result.append(buffer.data(), read);
        }

        int exit_code = pclose(pipe);

#ifndef _WIN32
        if (WIFEXITED(exit_code)) {
            exit_code = WEXITSTATUS(exit_code);
        }
#endif

        return { exit_code, result };
    }

    fork_result fork(const std::string& program, const std::vector<std::string>& args) {
#ifdef _WIN32
        auto executable = std::format("\"{}\"", program);
        auto arguments = stdext::string::replace_all(stdext::string::wrap(args, "\""), "\\", "\\\\");
        auto command = std::format("{} {}", executable, arguments);

        auto startup_info = STARTUPINFOA();
        auto process_info = PROCESS_INFORMATION();

        startup_info.cb = sizeof(startup_info);

        auto success = CreateProcessA(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            true,
            0,
            nullptr,
            nullptr,
            &startup_info,
            &process_info
        );

        if (!success) {
            throw std::runtime_error("Failed to create process.");
        }

        return {
            reinterpret_cast<void*>(process_info.hProcess),
            reinterpret_cast<void*>(process_info.hThread),
            process_info.dwProcessId
        };
#else
        auto pid = ::fork();

        if (pid < 0) {
            throw std::runtime_error("Failed to fork process.");
        }

        if (pid == 0) {
            auto argv = std::vector<char*>();

            argv.push_back(const_cast<char*>(program.c_str()));

            for (const auto& argument : args) {
                argv.push_back(const_cast<char*>(argument.c_str()));
            }

            argv.push_back(nullptr);

            execvp(program.c_str(), argv.data());
            _exit(127);
        }

        return { pid };
#endif
    }

    int wait(const fork_result& process) {
        int exit_code = 0;

#ifdef _WIN32
        WaitForSingleObject(reinterpret_cast<HANDLE>(process.process), INFINITE);
        GetExitCodeProcess(reinterpret_cast<HANDLE>(process.process), (DWORD*)&exit_code);
        CloseHandle(reinterpret_cast<HANDLE>(process.thread));
        CloseHandle(reinterpret_cast<HANDLE>(process.process));
#else
        waitpid(process.pid, &exit_code, 0);

        if (WIFEXITED(exit_code)) {
            return WEXITSTATUS(exit_code);
        }
#endif

        return exit_code;
    }

    std::string username() {
#ifdef _WIN32
        auto buffer = std::array<char, 256>();
        auto size = DWORD(buffer.size());

        GetUserNameA(buffer.data(), &size);

        return std::string(buffer.data());
#else
        auto result = std::getenv("USER");

        if (!result) {
            return "";
        }

        return std::string(result);
#endif
    }

    std::string hostname() {
        auto buffer = std::array<char, 256>();
        auto length = buffer.size();

#ifdef _WIN32
        GetComputerNameA(buffer.data(), (DWORD*)&length);
        return buffer.data();
#else
        gethostname(buffer.data(), length);
        return buffer.data();
#endif
    }

    char* get_env(const std::string& name) {
        return std::getenv(name.c_str());
    }

    std::string get_env(const std::string& name, const std::string& default_val) {
        auto result = std::getenv(name.c_str());

        if (!result) {
            return default_val;
        }

        return std::string(result);
    }

    void set_env(const std::string& name, const std::string& value) {
#ifdef _WIN32
        SetEnvironmentVariableA(name.c_str(), value.c_str());
#else
        setenv(name.c_str(), value.c_str(), 1);
#endif
    }

    void remove_env(const std::string& name) {
#ifdef _WIN32
        SetEnvironmentVariableA(name.c_str(), nullptr);
#else
        unsetenv(name.c_str());
#endif
    }
}
