#include "handlers.h"
#include "commands.h"
#include "core/utils.h"
#include "help.h"

#include "core/constants.h"
#include "core/generator.h"
#include "core/project.h"

#include "runner.h"
#include "stdext/arg_parser.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/string.hpp"
#include "stdext/time.hpp"
#include "stdext/version.hpp"

#include <chrono>
#include <filesystem>

namespace Handlers {
    constexpr const char kMainSourceContent[] = {
        #embed "./resources/template/src/main.cpp"
        , 0
    };

    constexpr const char kGitignoreContent[] = {
        #embed "./resources/template/gitignore.txt"
        , 0
    };

    constexpr const char kProjectConfigContent[] = {
        #embed "./resources/template/forge.json"
        , 0
    };

    const auto kPackageTypeMap = stdext::hash_map<Project::PackageType, std::string> {
        { Project::PackageType::Executable, "<executable>" },
        { Project::PackageType::StaticLibrary, "<static>" },
        { Project::PackageType::SharedLibrary, "<shared>" }
    };

    void HandleHelpCommand(stdext::arg_parser& arg_parser) {
        if (arg_parser.is_end()) {
            return Help::PrintMessage();
        }

        return Help::PrintMessage(arg_parser.next());
    }

    void HandleInfoCommand(stdext::arg_parser& arg_parser) {
        auto simple_command = Command::ProjectCommand(arg_parser.current(), arg_parser);
        auto project_context = Project::InitContext(Project::Profile::Debug, simple_command.m_environment, simple_command.m_defines, simple_command.m_project);
        auto local_time = stdext::time::to_zoned_time(project_context.version.timestamp);

        stdext::println("Directory:   {}", project_context.base_path);
        stdext::println("Packages:    {}", project_context.packages.size());
        stdext::println("Version:     {}.{}.{}", project_context.version.first, project_context.version.second, project_context.version.build);
        stdext::println("Timestamp:   {:%d/%m/%Y %H:%M:%S}", local_time);

        stdext::println();

        for (const auto& [package_id, package] : project_context.packages) {
            stdext::println("{:12} {}", kPackageTypeMap.at(package.type), package_id);
        }
    }

    auto resolvePackageId(const std::string& project_directory, const std::string& package_name) {
        if (!package_name.empty()) {
            return package_name;
        }

        auto directory_name = stdext::fs::stem(project_directory);

        if (stdext::string::contains(directory_name, " ")) {
            return stdext::string::replace_all(directory_name, " ", "_");
        }

        return directory_name;
    }

    void HandleInitCommand(stdext::arg_parser& arg_parser) {
        auto simple_command = Command::SimpleCommand(arg_parser.current(), arg_parser);
        auto project_directory = stdext::fs::sanitize_posix(simple_command.m_project);
        auto package_name = arg_parser.is_end() ? std::string() : arg_parser.next();
        auto package_id = resolvePackageId(project_directory, package_name);

        auto source_path = stdext::fs::join_path(project_directory, "src/main.cpp");
        auto gitignore_path = stdext::fs::join_path(project_directory, ".gitignore");
        auto config_path = stdext::fs::join_path(project_directory, FORGE_PROJECT_FILE);
        auto version_path = stdext::fs::join_path(project_directory, FORGE_VERSION_FILE);

        auto resolved_config = stdext::string::replace_all(kProjectConfigContent, "{package_id}", package_id);
        auto version_file = stdext::version_info(0, 0, 0, 0);

        stdext::fs::write_file(source_path, kMainSourceContent);
        stdext::fs::write_file(gitignore_path, kGitignoreContent);
        stdext::fs::write_file(config_path, resolved_config);
        stdext::store_version_file(version_file, version_path);

        stdext::success("Project initialized at: '{}'", project_directory);
    }

    void HandleGenCommand(stdext::arg_parser& arg_parser) {
        auto gen_command = Command::GenCommand(arg_parser.current(), arg_parser);
        auto project_context = Project::InitContext(gen_command.m_profile, gen_command.m_environment, gen_command.m_defines, gen_command.m_project);
        auto dirty_packages = Planner::GetDirtyPackages(project_context, gen_command.m_package, true);
        auto build_context = Build::Context(project_context, dirty_packages);

        switch (gen_command.m_format) {
            case Command::GenFormat::Ninja:
                Generator::GenerateNinja(build_context, gen_command.m_output.empty() ? "build.ninja" : gen_command.m_output);
                break;

            case Command::GenFormat::Makefile:
                Generator::GenerateMakefile(build_context, gen_command.m_output.empty() ? "Makefile" : gen_command.m_output);
                break;

            case Command::GenFormat::Shell:
                Generator::GenerateShellScript(build_context, gen_command.m_output.empty() ? "build.sh" : gen_command.m_output);
                break;

            case Command::GenFormat::Batch:
                Generator::GenerateBatchFile(build_context, gen_command.m_output.empty() ? "build.bat" : gen_command.m_output);
                break;
        }
    }

    void HandleSetupCommand(stdext::arg_parser& arg_parser) {
        auto setup_command = Command::SetupCommand(arg_parser.current(), arg_parser);
        auto project_context = Project::InitContext(setup_command.m_profile, setup_command.m_environment, setup_command.m_defines, setup_command.m_project);
        auto dirty_packages = Planner::GetDirtyPackages(project_context, setup_command.m_package, true);
        auto build_context = Build::Context(project_context, dirty_packages);
        auto output_path = setup_command.m_output;

        if (output_path.empty()) {
            output_path = stdext::fs::join_path(project_context.base_path, FORGE_DIRECTORY, "compile_commands.json");
        }

        Generator::GenerateCompileCommands(build_context, output_path);
    }

    bool HandleBuildCommand(stdext::arg_parser& arg_parser) {
        auto build_command = Command::BuildCommand(arg_parser.current(), arg_parser);
        auto project_context = Project::InitContext(build_command.m_profile, build_command.m_environment, build_command.m_defines, build_command.m_project);

        if (build_command.m_increment) {
            project_context.version.build++;
            project_context.version.timestamp = stdext::time::unix_seconds();
        }

        auto dirty_packages = Planner::GetDirtyPackages(project_context, build_command.m_package, build_command.m_rebuild);

        if (dirty_packages.empty()) {
            stdext::success("No changes detected. Everything is already up to date.");
            return true;
        }

        auto build_context = Build::Context(project_context, dirty_packages);
        auto build_manager = Builder::BuildManager(build_context, build_command.m_verbosity);

        build_manager.start(build_command.m_workers);
        build_manager.wait();

        if (build_manager.success()) {
            auto current_state = Source::InitContext(project_context);
            auto version_file = stdext::fs::join_path(project_context.base_path, FORGE_VERSION_FILE);

            Source::StoreContext(current_state, project_context.cache_path);

            if (build_command.m_increment) {
                stdext::store_version_file(project_context.version, version_file);
            }
        }

        return build_manager.success();
    }

    bool HandleTestCommand(stdext::arg_parser& arg_parser) {
        auto test_runner = Runner::TestRunner();
        auto test_command = Command::TestCommand(arg_parser.current(), arg_parser);
        auto project_context = Project::InitContext(test_command.m_profile, test_command.m_environment, {}, test_command.m_project);

        if (project_context.tests.empty()) {
            stdext::error("No test packages configured for this project.");
            return false;
        }

        for (const auto& package_id : project_context.tests) {
            if (test_command.m_package.has_value() && test_command.m_package != package_id) {
                continue;
            }

            if (!project_context.packages.contains(package_id)) {
                stdext::error("Test package '{}' not found.", package_id);
                return false;
            }

            auto& project_package = project_context.packages.at(package_id);

            if (project_package.type != Project::PackageType::Executable) {
                if (test_command.m_package != package_id) {
                    stdext::error("Package '{}' is not an executable package.", package_id);
                    return false;
                }

                continue;
            }

            auto package_output = Utils::GetOutputFile(project_context, project_package);

            if (!stdext::fs::is_regular_file(package_output)) {
                stdext::error("Test package '{}' has not been built. Run 'forge build' first.", package_id);
                return false;
            }

            test_runner.add({ .name = package_id, .command = package_output });
        }

        return test_runner.run(test_command.m_workers);
    }

    void HandleCopyCommand(stdext::arg_parser& arg_parser) {
        auto copy_command = Command::SelectCommand(arg_parser.current(), arg_parser);

        if (copy_command.m_profile == Project::Profile::Complete) {
            copy_command.m_profile = Project::Profile::Debug;
        }

        auto project_context = Project::InitContext(copy_command.m_profile, copy_command.m_environment, {}, copy_command.m_project);

        for (const auto& resource : project_context.resources) {
            auto resource_path = stdext::fs::join_path(project_context.base_path, resource.from);

            if (!stdext::fs::exists(resource_path)) {
                stdext::warn("Resource '{}' does not exist, skipping...", resource_path);
                continue;
            }

            auto output_path = stdext::fs::join_path(project_context.output_path, resource.to);
            auto parent_path = stdext::fs::parent_path(output_path);

            if (!stdext::fs::is_directory(parent_path)) {
                stdext::fs::create_directories(parent_path);
            }

            if (stdext::fs::is_directory(resource_path)) {
                stdext::fs::copy(resource_path, output_path, stdext::fs::copy_options::recursive | stdext::fs::copy_options::overwrite_existing);
            }
            else {
                stdext::fs::copy_file(resource_path, output_path, stdext::fs::copy_options::overwrite_existing);
            }
        }
    }

    void removeForgeDirectory(const Command::SelectCommand& select_command) {
        auto forge_directory = stdext::fs::join_path(select_command.m_project, FORGE_DIRECTORY);

        if (!stdext::fs::is_directory(forge_directory)) {
            stdext::info("Nothing to reset.");
            return;
        }

        if (!stdext::confirm(stdext::confirm_default::no, "This will delete all build artifacts and dependencies. Continue?")) {
            stdext::warn("Aborting...");
            return;
        }

        stdext::warn("Removing '{}' directory...", forge_directory);
        stdext::fs::remove_all(forge_directory);
        stdext::success("Project reset complete.");
    }

    void HandleCleanCommand(stdext::arg_parser& arg_parser) {
        auto select_command = Command::SelectCommand(arg_parser.current(), arg_parser);

        if (select_command.m_profile == Project::Profile::Complete) {
            return removeForgeDirectory(select_command);
        }

        auto project_context = Project::InitContext(select_command.m_profile, select_command.m_environment, {}, select_command.m_project);

        if (stdext::fs::is_directory(project_context.build_path)) {
            stdext::warn("Removing '{}' directory...", project_context.build_path);
            stdext::fs::remove_all(project_context.build_path);
        }

        if (stdext::fs::is_directory(project_context.output_path)) {
            stdext::warn("Removing '{}' directory...", project_context.output_path);
            stdext::fs::remove_all(project_context.output_path);
        }
    }
}
