#include "core/builder.h"

#include "core/context.h"
#include "stdext/filesystem.hpp"
#include "stdext/system.hpp"

#include <filesystem>

namespace Builder {
    BuildManager::BuildManager(Build::Context& context, LogVerbosity verbosity)
        : m_verbosity(verbosity), m_context(context) {
    }

    BuildManager::~BuildManager() {
        requestStop();
    }

    void BuildManager::start(int num_workers) {
        for (auto i = 0; i < m_context.nodes.size(); i++) {
            auto& build_node = m_context.nodes.at(i);

            if (build_node.commands.empty()) {
                enqueueTask({ .final_build = true, .node_index = i });
                continue;
            }

            for (auto j = 0; j < build_node.commands.size(); j++) {
                enqueueTask({ .final_build = false, .node_index = i, .command_index = j });
            }
        }

        m_thread_pool.set_error_handler([this](int, auto) {
            m_thread_pool.clear();
        });

        m_thread_pool.start(num_workers);
        m_stop_watch.reset();
    }

    void BuildManager::wait() {
        m_thread_pool.join();
        printBuildResult();
    }

    bool BuildManager::success() const {
        return !m_has_failed;
    }

    bool BuildManager::failed() const {
        return m_has_failed;
    }

    void BuildManager::printBuildResult() {
        auto totalSeconds = m_stop_watch.elapsed_seconds();

        auto hours = static_cast<int>(totalSeconds / 3600);
        auto minutes = static_cast<int>((totalSeconds % 3600) / 60);
        auto seconds = static_cast<int>(totalSeconds % 60);

        auto formatted = std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);

        if (m_verbosity == LogVerbosity::Default) {
            stdext::clear_current_line(stdout);
        }

        if (!m_has_failed) {
            stdext::success("Build successfully: {}", formatted);
        }
        else {
            stdext::error("Build failed: {}", formatted);
        }
    }

    void BuildManager::executeBuildTask(const BuildTask& task) {
        if (m_stop_requested) {
            return;
        }

        auto& build_node = m_context.nodes.at(task.node_index);
        auto& build_command = getCommand(task, build_node);

        if (!prepareOutputDir(build_command) || !executeCommand(build_command)) {
            notifyError();
            return;
        }

        if (build_command.type == Build::CommandType::Compile) {
            updateCounter(task.node_index);
        }
        else {
            updateDependents(build_node);
        }
    }

    auto BuildManager::getCommand(const BuildTask& task, Build::Node& node) -> const Build::Command& {
        if (!task.final_build) {
            return node.commands.at(task.command_index);
        }

        return node.final_command;
    }

    bool BuildManager::prepareOutputDir(const Build::Command& command) {
        auto error_code = std::error_code();

        stdext::fs::create_directories(command.output_dir, error_code);

        if (error_code) {
            outputMessage(MessageKind::Error, "Failed to create output directory: " + command.output_dir);
            return false;
        }

        return true;
    }

    bool BuildManager::executeCommand(const Build::Command& command) {
        outputUpdate(command);

        auto [exit_code, compiler_log] = stdext::system::exec_pipe(command.value);

        if (exit_code != 0) {
            enableErrorLogs();
            outputMessage(MessageKind::Error, command.source_path);
            outputMessage(MessageKind::Plain, compiler_log);
            return false;
        }

        if (!compiler_log.empty()) {
            outputMessage(MessageKind::Warn, command.source_path);
            outputMessage(MessageKind::Plain, compiler_log);
        }

        return true;
    }

    void BuildManager::updateDependents(const Build::Node& node) {
        for (auto node_index : node.dependents) {
            updateCounter(node_index);
        }
    }

    void BuildManager::updateCounter(int node_index) {
        if (--m_context.counters[node_index] == 0) {
            enqueueTask({ .final_build = true, .node_index = node_index });
        }
    }

    void BuildManager::enableErrorLogs() const {
        if (m_verbosity == LogVerbosity::Quiet) {
            stdext::logger::add_sink(stderr);
        }
    }

    void BuildManager::disableErrorLogs() const {
        if (m_verbosity == LogVerbosity::Quiet) {
            stdext::logger::remove_sink(stderr);
        }
    }

    void BuildManager::outputUpdate(const Build::Command& command) {
        auto lock = std::lock_guard(m_console_mutex);

        if (m_verbosity == LogVerbosity::Default) {
            stdext::clear_current_line(stdout);
        }

        switch (command.type) {
            case Build::CommandType::Compile:
                stdext::update("Compiling '{}'", command.source_path);
                break;

            case Build::CommandType::Archive:
                stdext::update("Archiving '{}'", command.output_path);
                break;

            case Build::CommandType::Link:
                stdext::update("Linking '{}'", command.output_path);
                break;
        }

        if (m_verbosity == LogVerbosity::Verbose) {
            std::printf("\n");
        }

        std::fflush(stdout);
    }

    void BuildManager::outputMessage(const MessageKind type, const std::string& message) {
        auto lock = std::lock_guard(m_console_mutex);

        if (m_verbosity == LogVerbosity::Default) {
            stdext::clear_current_line(stdout);
        }

        if (type != MessageKind::Plain) {
            stdext::verbose("{}", message);
        }

        switch (type) {
            case MessageKind::Plain:
                stdext::logf(message);
                break;

            case MessageKind::Info:
                stdext::log("{}", message);
                break;

            case MessageKind::Warn:
                stdext::warn("{}", message);
                break;

            case MessageKind::Error:
            {
                stdext::error("{}", message);
                break;
            }
        }
    }

    void BuildManager::enqueueTask(const BuildTask& task) {
        if constexpr (stdext::is_debug) {
#ifdef SHOW_ENQUEUE_LOGS
            auto lock_guard = std::lock_guard(m_console_mutex);
            auto& build_node = m_context.nodes.at(task.node_index);

            if (task.final_build == false) {
                stdext::debug("[EnqueueTask] '{}': {}", build_node.package_id, build_node.commands.at(task.command_index).value);
            }
            else {
                stdext::debug("[EnqueueTask] '{}': {}", build_node.package_id, build_node.final_command.value);
            }
#endif
        }

        m_thread_pool.enqueue([this, task] {
            executeBuildTask(task);
        });
    }

    void BuildManager::requestStop() {
        m_stop_requested = true;
    }

    void BuildManager::notifyError() {
        m_has_failed = true;
        requestStop();
    }
}
