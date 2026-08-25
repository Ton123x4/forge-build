#include "packages/forge-core/include/core/constants.h"
#include "stdext/arg_parser.hpp"
#include "stdext/console.hpp"
#include "stdext/forge.hpp"
#include "stdext/logger.hpp"
#include "stdext/filesystem.hpp"
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string_view>

constexpr const char khelpMessage[] = {
    #embed "./resources/help/help_link.txt"
    , 0
};

auto getModulePath(std::string_view identifier) -> std::string {
    return stdext::fs::absolute(stdext::fs::join_path(FORGE_DEPENDENCIES_PATH, identifier));
}

void createModuleLink(std::string_view target_path, std::string_view link_path) {
    auto resolved_path = stdext::fs::sanitize(stdext::fs::absolute(target_path));
    auto parent_path = stdext::fs::parent_path(link_path);

    stdext::debug("Creating symlink: {} => {}", link_path, resolved_path);

    stdext::fs::create_directories(parent_path);
    stdext::fs::create_directory_symlink(resolved_path, link_path);
}

void handleLink(stdext::arg_parser& parser) {
    auto identifier = parser.expect("Missing module identifier");
    auto directory_path = parser.expect("Missing directory path");

    if (!stdext::fs::is_directory(directory_path)) {
        throw std::runtime_error(std::format("Directory does not exist: '{}'", directory_path));
    }

    auto module_path = getModulePath(identifier);

    if (stdext::fs::exists(module_path)) {
        if (!stdext::confirm("Module '{}' is already installed. Do you want to reinstall?", identifier)) {
            stdext::warn("Aborting...");
            return;
        }

        stdext::fs::remove(module_path);
    }

    createModuleLink(directory_path, module_path);

    stdext::success("Module '{}' linked successfully!", identifier);
}

void handleRemove(stdext::arg_parser& parser) {
    auto identifier = parser.expect("Missing module identifier");
    auto module_path = getModulePath(identifier);

    if (!stdext::fs::is_symlink(module_path)) {
        stdext::warn("Module '{}' is not installed.", identifier);
        return;
    }

    stdext::fs::remove(module_path);
    stdext::success("Module '{}' successfully uninstalled.", identifier);
}

void handleList(stdext::arg_parser& parser) {
    if (!stdext::fs::is_directory(FORGE_DEPENDENCIES_PATH) || stdext::fs::is_empty(FORGE_DEPENDENCIES_PATH)) {
        stdext::info("No modules installed.");
        return;
    }

    stdext::println(stdext::ansi_color::blue, "Installed modules:");

    for (const auto& entry : stdext::fs::directory_iterator(FORGE_DEPENDENCIES_PATH)) {
        if (stdext::fs::is_symlink(entry) || stdext::fs::is_directory(entry)) {
            auto target_path = entry.path();
            auto filename = target_path.filename();

            if (stdext::fs::is_symlink(entry)) {
                target_path = stdext::fs::read_symlink(target_path);
            }


            stdext::println("  {} ==> {}", filename.string(), target_path.string());
        }
    }
}

void helpMessage() {
    stdext::println("{}", khelpMessage);
}

int main(int argc, char* argv[]) {
    stdext::init_console();

    if (argc == 1) {
        helpMessage();
        return EXIT_SUCCESS;
    }

    try {
        auto arg_parser = stdext::arg_parser(argc, argv);

        if (arg_parser.match({ "--version", "-v" })) {
            stdext::forge::print_build_info("Forge Package Manager", true);
        }
        else if (arg_parser.match({ "--help", "-h" })) {
            helpMessage();
        }
        else if (arg_parser.match({ "remove" })) {
            handleRemove(arg_parser);
        }
        else if (arg_parser.match({ "list" })) {
            handleList(arg_parser);
        }
        else {
            handleLink(arg_parser);
        }
    } catch (const std::exception& error) {
        stdext::error("{}", error.what());
        return EXIT_FAILURE;
    }
}
