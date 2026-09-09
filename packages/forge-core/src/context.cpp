#include "core/context.h"

#include "core/project.h"
#include "core/source.h"
#include "core/utils.h"
#include "stdext/filesystem.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/string.hpp"

#include <format>
#include <string>

namespace Build {
    static inline bool isTransitive(const Project::PackageType& type) {
        return type == Project::PackageType::StaticLibrary;
    }

    static void collectIncludes(const Project::Context& context, const std::string& package_id, Project::StringArray& includes) {
        if (context.packages.contains(package_id)) {
            auto& package = context.packages.at(package_id);

            for (const auto& include_dir : package.includes) {
                includes.insert(stdext::fs::join_path(package.path, include_dir));
            }

            for (const auto& dependency_id : package.dependencies) {
                collectIncludes(context, dependency_id, includes);
            }

            return;
        }

        if (context.dependencies.contains(package_id)) {
            auto& dependency = context.dependencies.at(package_id);
            auto dependency_parent = dependency.parent.empty() ? package_id : dependency.parent;
            auto dependency_home = Utils::GetDependencyDirectory(context, dependency_parent);

            for (const auto& include_dir : dependency.includes) {
                includes.insert(stdext::fs::join_path(dependency_home, include_dir));
            }

            for (const auto& dependency_id : dependency.dependencies) {
                collectIncludes(context, dependency_id, includes);
            }
        }
    }

    static void addForgeDefines(const Project::Context& context, Project::StringArray& defines) {
        defines.insert("FORGE_BUILD_TOOL");

        defines.insert(std::format("FORGE_VERSION_FIRST={}", context.version.first));
        defines.insert(std::format("FORGE_VERSION_SECOND={}", context.version.second));
        defines.insert(std::format("FORGE_VERSION_BUILD={}", context.version.build));
        defines.insert(std::format("FORGE_VERSION_TIMESTAMP={}", static_cast<long long>(context.version.timestamp)));
    }

    static void addProfileDefines(const Project::Context& context, Project::StringArray& defines) {
        switch (context.profile) {
            case Project::Profile::Debug:
                defines.insert("DEBUG");
                break;

            case Project::Profile::Preview:
                defines.insert("NDEBUG");
                break;

            case Project::Profile::Release:
                defines.insert("NDEBUG");
                defines.insert("RELEASE");
                break;

            default:
                break;
        }
    }

    static void addProfileFlags(const Project::Context& context, std::vector<std::string>& command_parts) {
        switch (context.profile) {
            case Project::Profile::Debug:
                command_parts.push_back("-g");
                break;

            default:
                break;
        }
    }

    static void collectDefines(const Project::Context& context, const std::string& package_id, Project::StringArray& defines) {
        if (context.packages.contains(package_id)) {
            auto& package = context.packages.at(package_id);

            for (const auto& define : package.defines) {
                defines.insert(define);
            }

            for (const auto& dependency_id : package.dependencies) {
                collectDefines(context, dependency_id, defines);
            }
        }

        if (context.dependencies.contains(package_id)) {
            auto& dependency = context.dependencies.at(package_id);

            for (const auto& define : dependency.defines) {
                defines.insert(define);
            }

            for (const auto& dependency_id : dependency.dependencies) {
                collectDefines(context, dependency_id, defines);
            }
        }
    }

    static void collectLibraryPaths(const Project::Context& context, const std::string& package_id, Project::StringArray& libpaths) {
        if (context.packages.contains(package_id)) {
            auto& package = context.packages.at(package_id);

            for (const auto& libpath : package.libpaths) {
                libpaths.insert(libpath);
            }

            if (!isTransitive(package.type)) {
                return;
            }

            for (const auto& dependency_id : package.dependencies) {
                collectLibraryPaths(context, dependency_id, libpaths);
            }

            return;
        }

        if (context.dependencies.contains(package_id)) {
            auto& dependency = context.dependencies.at(package_id);
            auto dependency_parent = dependency.parent.empty() ? package_id : dependency.parent;
            auto dependency_home = Utils::GetDependencyDirectory(context, dependency_parent);

            for (const auto& libpath : dependency.libpaths) {
                libpaths.insert(stdext::fs::join_path(dependency_home, libpath));
            }

            for (const auto& dependency_id : dependency.dependencies) {
                collectLibraryPaths(context, dependency_id, libpaths);
            }
        }
    }

    static void collectLibraries(const Project::Context& context, const std::string& package_id, Project::StringArray& libraries) {
        if (context.packages.contains(package_id)) {
            auto& package = context.packages.at(package_id);

            for (const auto& library : package.libraries) {
                libraries.insert(library);
            }

            if (!isTransitive(package.type)) {
                return;
            }

            for (const auto& dependency_id : package.dependencies) {
                collectLibraries(context, dependency_id, libraries);
            }

            return;
        }

        if (context.dependencies.contains(package_id)) {
            auto& dependency = context.dependencies.at(package_id);

            for (const auto& library : dependency.libraries) {
                libraries.insert(library);
            }

            for (const auto& dependency_id : dependency.dependencies) {
                collectLibraries(context, dependency_id, libraries);
            }
        }
    }

    static void collectDependencyOutputs(const Project::Context& context, const std::string& package_id, std::vector<std::string>& outputs) {
        if (!context.packages.contains(package_id)) {
            return;
        }

        auto& package = context.packages.at(package_id);

        if (package.type == Project::PackageType::Executable) {
            return;
        }

        outputs.push_back(Utils::GetLibraryFile(context, package.output));

        if (!isTransitive(package.type)) {
            return;
        }

        for (const auto& dependency_id : package.dependencies) {
            collectDependencyOutputs(context, dependency_id, outputs);
        }
    }

    static auto getIncludes(const Project::Context& context, const Project::Package& package) {
        auto includes = Project::StringArray();

        for (const auto& include_dir : context.global_includes) {
            includes.insert(stdext::fs::join_path(context.base_path, include_dir));
        }

        for (const auto& include_dir : package.includes) {
            includes.insert(stdext::fs::join_path(package.path, include_dir));
        }

        for (const auto& dependency_id : package.dependencies) {
            collectIncludes(context, dependency_id, includes);
        }

        return includes;
    }

    static auto getDefines(const Project::Context& context, const Project::Package& package) {
        auto defines = Project::StringArray();

        addProfileDefines(context, defines);
        addForgeDefines(context, defines);

        for (const auto& define : context.global_defines) {
            defines.insert(define);
        }

        for (const auto& define : package.defines) {
            defines.insert(define);
        }

        for (const auto& dependency_id : package.dependencies) {
            collectDefines(context, dependency_id, defines);
        }

        return defines;
    }

    static auto getLibraryPaths(const Project::Context& context, const Project::Package& package) {
        auto libpaths = Project::StringArray();

        for (const auto& libpath : package.libpaths) {
            libpaths.insert(libpath);
        }

        for (const auto& dependency_id : package.dependencies) {
            collectLibraryPaths(context, dependency_id, libpaths);
        }

        return libpaths;
    }

    static auto getLibraries(const Project::Context& context, const Project::Package& package) {
        auto libraries = Project::StringArray();

        for (const auto& library : package.libraries) {
            libraries.insert(library);
        }

        for (const auto& dependency_id : package.dependencies) {
            collectLibraries(context, dependency_id, libraries);
        }

        return libraries;
    }

    static auto getDependencyOutputs(const Project::Context& context, const Project::Package& package) {
        auto outputs = std::vector<std::string>();

        for (const auto& dependency_id : package.dependencies) {
            collectDependencyOutputs(context, dependency_id, outputs);
        }

        return outputs;
    }

    static auto generateCompileCommand(const Project::Context& context, const Planner::DirtyPackage& package, const Source::SourceInfo& source) {
        auto& project_package = context.packages.at(package.id);
        auto command_parts = std::vector<std::string>();
        auto output_file = Utils::GetObjectFile(context, package.id, source.output);
        auto output_dir = Utils::GetOutputDirectory(output_file);

        command_parts.push_back(context.build_config.compiler);
        command_parts.push_back("-MMD");

        addProfileFlags(context, command_parts);

        for (const auto& flag : project_package.flags) {
            command_parts.push_back(flag);
        }

        for (const auto& flag : project_package.cflags) {
            command_parts.push_back(flag);
        }

        for (const auto& define : getDefines(context, project_package)) {
            command_parts.push_back(std::format("-D {}", define));
        }

        for (const auto& include_dir : getIncludes(context, project_package)) {
            command_parts.push_back(std::format("-I {}", stdext::fs::sanitize_posix(include_dir)));
        }

        command_parts.push_back(std::format("-c {}", source.path));
        command_parts.push_back(std::format("-o {}", output_file));

        auto command = Command(CommandType::Compile);

        command.value = stdext::string::join(command_parts, " ");
        command.output_dir = output_dir;
        command.output_path = output_file;
        command.source_path = source.path;

        return command;
    }

    static auto generateArchiveCommand(const Project::Context& context, const Planner::DirtyPackage& package) {
        auto& project_package = context.packages.at(package.id);
        auto command_parts = std::vector<std::string>();
        auto output_file = Utils::GetOutputFile(context, project_package);
        auto output_dir = Utils::GetOutputDirectory(output_file);

        command_parts.push_back(context.build_config.archiver);
        command_parts.push_back("rcs");
        command_parts.push_back(output_file);

        for (const auto& object : package.objects) {
            command_parts.push_back(Utils::GetObjectFile(context, package.id, object));
        }

        auto command = Command(CommandType::Archive);

        command.value = stdext::string::join(command_parts, " ");
        command.source_path = output_file;
        command.output_dir = output_dir;
        command.output_path = output_file;

        return command;
    }

    static auto generateLinkCommand(const Project::Context& context, const Planner::DirtyPackage& package) {
        auto& project_package = context.packages.at(package.id);
        auto command_parts = std::vector<std::string>();
        auto output_file = Utils::GetOutputFile(context, project_package);
        auto output_dir = Utils::GetOutputDirectory(output_file);

        command_parts.push_back(context.build_config.compiler);
        command_parts.push_back(std::format("-o {}", output_file));

        addProfileFlags(context, command_parts);

        if (project_package.type == Project::PackageType::SharedLibrary) {
            command_parts.push_back("-shared");
        }

        for (const auto& flag : project_package.flags) {
            command_parts.push_back(flag);
        }

        for (const auto& flag : project_package.bflags) {
            command_parts.push_back(flag);
        }

        for (const auto& libpath : getLibraryPaths(context, project_package)) {
            command_parts.push_back(std::format("-L {}", stdext::fs::sanitize_posix(libpath)));
        }

        for (const auto& object : package.objects) {
            command_parts.push_back(Utils::GetObjectFile(context, package.id, object));
        }

        for (const auto& output : getDependencyOutputs(context, project_package)) {
            command_parts.push_back(output);
        }

        for (const auto& library : getLibraries(context, project_package)) {
            command_parts.push_back(std::format("-l {}", library));
        }

        auto command = Command(CommandType::Link);

        command.value = stdext::string::join(command_parts, " ");
        command.source_path = output_file;
        command.output_dir = output_dir;
        command.output_path = output_file;

        return command;
    }

    static auto generateBuildCommand(const Project::Context& context, const Planner::DirtyPackage& package) {
        auto& project_package = context.packages.at(package.id);

        if (project_package.type == Project::PackageType::StaticLibrary) {
            return generateArchiveCommand(context, package);
        }

        return generateLinkCommand(context, package);
    }

    Context::Context(const Project::Context& context, Planner::DirtyPackages& dirty_packages) : context(context), remaining(0) {
        auto references = stdext::hash_map<std::string, int>();

        counters.resize(dirty_packages.size());
        remaining.store(dirty_packages.size());

        for (const auto& package : dirty_packages) {
            auto& project_package = context.packages.at(package.id);
            auto build_node = Build::Node(package.id);

            for (const auto& source : package.sources) {
                build_node.commands.push_back(generateCompileCommand(context, package, source));
            }

            build_node.final_command = generateBuildCommand(context, package);

            auto build_index = nodes.size();
            auto dirty_dependencies = 0;

            for (const auto& dependency_id : project_package.dependencies) {
                if (references.contains(dependency_id)) {
                    auto dependency_index = references.at(dependency_id);

                    nodes[dependency_index].dependents.push_back(build_index);
                    dirty_dependencies++;
                }
            }

            counters[build_index].store(build_node.commands.size() + dirty_dependencies);

            references.insert(package.id, build_index);
            nodes.push_back(std::move(build_node));
        }
    }
}
