#pragma once

#include "context.h"
#include "stdext/thread.hpp"
#include "stdext/time.hpp"

namespace Builder {
    enum class LogVerbosity {
        Quiet,
        Default,
        Verbose
    };

    struct BuildTask {
        bool final_build;
        int node_index;
        int command_index;
    };

    class BuildManager {
    public:
        BuildManager(Build::Context& context, LogVerbosity verbosity);
        ~BuildManager();

        void start(int num_workers);

        void wait();

        bool success() const;

        bool failed() const;

    private:
        enum class MessageKind {
            Plain,
            Info,
            Warn,
            Error
        };

        LogVerbosity m_verbosity;
        Build::Context& m_context;
        stdext::thread_pool m_thread_pool;
        stdext::time::stopwatch m_stop_watch;

        std::mutex m_console_mutex;
        std::atomic<bool> m_has_failed {false};
        std::atomic<bool> m_stop_requested {false};

        void printBuildResult();

        void executeBuildTask(const BuildTask& task);

        auto getCommand(const BuildTask& task, Build::Node& node) -> const Build::Command&;

        bool prepareOutputDir(const Build::Command& command);
        bool executeCommand(const Build::Command& command);

        void updateDependents(const Build::Node& node);
        void updateCounter(int node_index);

        void outputUpdate(const Build::Command& command);
        void outputMessage(const MessageKind type, const std::string& message);

        void enableErrorLogs() const;
        void disableErrorLogs() const;


        void enqueueTask(const BuildTask& task);
        void requestStop();
        void notifyError();
    };
}
