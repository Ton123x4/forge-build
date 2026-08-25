#pragma once

#include "planner.h"
#include "project.h"
#include <atomic>
#include <deque>
#include <vector>

namespace Build {
    enum struct CommandType {
        Compile,
        Archive,
        Link
    };

    struct Command {
        CommandType type;
        std::string value;
        std::string output_dir;
        std::string output_path;
        std::string source_path;
    };

    struct Node {
        std::string package_id;
        std::vector<int> dependents;
        std::vector<Command> commands;
        Command final_command;
    };

    struct Context {
        const Project::Context& context;
        std::vector<Node> nodes;
        std::atomic<int> remaining;
        std::deque<std::atomic<int>> counters;

        explicit Context(const Project::Context& context, Planner::DirtyPackages& dirty_packages);
    };
}
