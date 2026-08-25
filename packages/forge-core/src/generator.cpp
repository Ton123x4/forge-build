#include "core/generator.h"

#include "stdext/filesystem.hpp"
#include "stdext/handler.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/hash_set.hpp"
#include "stdext/string.hpp"

#include <format>
#include <vector>

namespace Generator {
    static auto buildIndexMap(const Build::Context& context) {
        auto indexes = stdext::hash_map<std::string, int>();

        for (auto index = 0; index < context.nodes.size(); ++index) {
            indexes.insert(context.nodes[index].package_id, index);
        }

        return indexes;
    }

    static auto resolveDependencyOutputs(const Build::Context& context, const stdext::hash_map<std::string, int>& indexes, const Build::Node& node) {
        auto outputs = std::vector<std::string>();
        auto& project_package = context.context.packages.at(node.package_id);

        for (const auto& dependency_id : project_package.dependencies) {
            if (indexes.contains(dependency_id)) {
                auto dependency_index = indexes.at(dependency_id);
                const auto& dependency_node = context.nodes.at(dependency_index);

                outputs.push_back(dependency_node.final_command.output_path);
            }
        }

        return outputs;
    }

    static auto generateNinjaRule() {
        auto lines = std::vector<std::string>();

        lines.push_back("rule run");
        lines.push_back("  command = $cmd");
        lines.push_back("  description = $desc");

        return lines;
    }

    static auto generateNinjaBuild(const Build::Command& command, const std::string& description, const std::vector<std::string>& order_deps) {
        auto lines = std::vector<std::string>();
        auto header = std::format("build {}: run", command.output_path);

        if (!order_deps.empty()) {
            header += std::format(" || {}", stdext::string::join(order_deps, " "));
        }

        lines.push_back(header);
        lines.push_back(std::format("  cmd = {}", command.value));
        lines.push_back(std::format("  desc = {}", description));

        return lines;
    }

    void GenerateNinja(const Build::Context& context, const std::string& output) {
        auto indexes = buildIndexMap(context);
        auto lines = generateNinjaRule();
        auto all_targets = std::vector<std::string>();

        for (const auto& node : context.nodes) {
            for (const auto& command : node.commands) {
                auto compile_lines = generateNinjaBuild(command, std::format("Compiling {}", command.output_path), {});

                lines.insert(lines.end(), compile_lines.begin(), compile_lines.end());
            }

            auto object_outputs = std::vector<std::string>();

            for (const auto& command : node.commands) {
                object_outputs.push_back(command.output_path);
            }

            auto dependency_outputs = resolveDependencyOutputs(context, indexes, node);

            for (const auto& dependency_output : dependency_outputs) {
                object_outputs.push_back(dependency_output);
            }

            auto final_lines = generateNinjaBuild(node.final_command, std::format("Linking {}", node.final_command.output_path), object_outputs);

            lines.insert(lines.end(), final_lines.begin(), final_lines.end());
            all_targets.push_back(node.final_command.output_path);
        }

        lines.push_back(std::format("build all: phony {}", stdext::string::join(all_targets, " ")));
        lines.push_back("default all");

        stdext::fs::write_file(output, stdext::string::join(lines, "\n"));
    }

    void GenerateMakefile(const Build::Context& context, const std::string& output) {
        auto indexes = buildIndexMap(context);
        auto lines = std::vector<std::string>();
        auto all_targets = std::vector<std::string>();

        for (const auto& node : context.nodes) {
            for (const auto& command : node.commands) {
                lines.push_back(std::format("{}:", command.output_path));
                lines.push_back("\t@mkdir -p $(dir $@)");
                lines.push_back(std::format("\t{}", command.value));
            }

            auto prereqs = std::vector<std::string>();

            for (const auto& command : node.commands) {
                prereqs.push_back(command.output_path);
            }

            auto dependency_outputs = resolveDependencyOutputs(context, indexes, node);

            for (const auto& dependency_output : dependency_outputs) {
                prereqs.push_back(dependency_output);
            }

            lines.push_back(std::format("{}: {}", node.final_command.output_path, stdext::string::join(prereqs, " ")));
            lines.push_back("\t@mkdir -p $(dir $@)");
            lines.push_back(std::format("\t{}", node.final_command.value));

            all_targets.push_back(node.final_command.output_path);
        }

        lines.push_back(std::format("all: {}", stdext::string::join(all_targets, " ")));

        stdext::fs::write_file(output, stdext::string::join(lines, "\n"));
    }

    void GenerateShellScript(const Build::Context& context, const std::string& output) {
        auto lines = std::vector<std::string>();
        auto created = stdext::hash_set<std::string>();

        lines.push_back("#!/bin/sh");
        lines.push_back("set -e");

        for (const auto& node : context.nodes) {
            for (const auto& command : node.commands) {
                if (!created.contains(command.output_dir)) {
                    lines.push_back(std::format("mkdir -p \"{}\"", command.output_dir));
                    created.insert(command.output_dir);
                }

                lines.push_back(command.value);
            }

            lines.push_back(std::format("mkdir -p \"{}\"", node.final_command.output_dir));
            lines.push_back(node.final_command.value);
        }

        stdext::fs::write_file(output, stdext::string::join(lines, "\n"));
    }

    void GenerateBatchFile(const Build::Context& context, const std::string& output) {
        auto lines = std::vector<std::string>();
        auto created = stdext::hash_set<std::string>();

        lines.push_back("@echo off");
        lines.push_back("setlocal enabledelayedexpansion");

        for (const auto& node : context.nodes) {
            for (const auto& command : node.commands) {
                if (!created.contains(command.output_dir)) {
                    lines.push_back(std::format("if not exist \"{}\" mkdir \"{}\"", command.output_dir, command.output_dir));
                    created.insert(command.output_dir);
                }

                lines.push_back(command.value);
                lines.push_back("if errorlevel 1 exit /b 1");
            }

            if (!created.contains(node.final_command.output_dir)) {
                lines.push_back(std::format("if not exist \"{}\" mkdir \"{}\"", node.final_command.output_dir, node.final_command.output_dir));
                created.insert(node.final_command.output_dir);
            }

            lines.push_back(node.final_command.value);
            lines.push_back("if errorlevel 1 exit /b 1");
        }

        stdext::fs::write_file(output, stdext::string::join(lines, "\n"));
    }

    void GenerateCompileCommands(const Build::Context& context, const std::string& output) {
        auto entries = std::vector<std::string>();

        for (const auto& node : context.nodes) {
            auto& project_package = context.context.packages.at(node.package_id);

            for (const auto& command : node.commands) {
                if (command.type != Build::CommandType::Compile) {
                    continue;
                }

                auto project_dir = project_package.path;

                if (project_dir.empty()) {
                    project_dir = "./";
                }

                auto project_base = stdext::pipe(context.context.base_path, stdext::fs::absolute, stdext::fs::sanitize_posix);
                auto directory_entry = std::format("\"directory\":\"{}\"", project_base);
                auto command_entry = std::format("\"command\":\"{}\"", command.value);
                auto file_entry = std::format("\"file\":\"{}\"", command.source_path);

                entries.push_back(std::format("{{{},{},{}}}", directory_entry, command_entry, file_entry));
            }
        }

        stdext::fs::write_file(output, std::format("[{}]", stdext::string::join(entries, ",")));
    }
}
