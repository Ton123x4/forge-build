#include "core/planner.h"

#include "core/project.h"
#include "core/source.h"
#include "core/utils.h"
#include "stdext/filesystem.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/hash_set.hpp"

#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace Planner {
    using Candidates = stdext::hash_set<std::string>;
    using DirtyPackageRefs = stdext::hash_map<std::string, int>;

    struct SortContext {
        const DirtyPackage& current;
        const DirtyPackages& packages;
        const DirtyPackageRefs& package_refs;
        const Project::Package& project_package;
        const Project::Context& project_context;
    };

    // Recursively collects a package and all of its local dependencies.
    static void collectDependencies(const Project::Context& context, const std::string& package_id, Candidates& candidates) {
        if (!candidates.contains(package_id)) {
            candidates.insert(package_id);

            const auto& package = context.packages.at(package_id);

            for (const auto& dependency_id : package.dependencies) {
                if (context.packages.contains(dependency_id)) {
                    collectDependencies(context, dependency_id, candidates);
                }
            }
        }
    }

    // Returns the package plus every local package it depends on.
    static auto collectWithDependencies(const Project::Context& context, const std::string& package_id) {
        auto candidates = Candidates();

        if (context.packages.contains(package_id)) {
            collectDependencies(context, package_id, candidates);
        }

        return candidates;
    }

    static auto loadCandidates(const Project::Context& context, const std::optional<std::string> package_id) {
        if (package_id.has_value()) {
            return collectWithDependencies(context, package_id.value());
        }

        auto candidates = Candidates();

        for (const auto& entry : context.packages) {
            candidates.insert(entry.first);
        }

        return candidates;
    }

    static void topologicalSort(SortContext& sort_context, DirtyPackages& sorted_packages, Candidates& visiting, Candidates& visited) {
        visiting.insert(sort_context.project_package.id);

        for (const auto& dependency_id : sort_context.project_package.dependencies) {
            if (!sort_context.project_context.packages.contains(dependency_id)) {
                continue;
            }

            if (!sort_context.package_refs.contains(dependency_id)) {
                continue;
            }

            if (visiting.contains(dependency_id)) {
                throw Project::ProjectLoaderError(std::format("dependency cycle detected involving package '{}'", dependency_id));
            }

            if (!visited.contains(dependency_id)) {
                auto package_index = sort_context.package_refs.at(dependency_id);
                auto package_dirty = sort_context.packages.at(package_index);
                auto package_info = sort_context.project_context.packages.at(dependency_id);
                auto temp_context = SortContext(package_dirty, sort_context.packages, sort_context.package_refs, package_info, sort_context.project_context);

                topologicalSort(temp_context, sorted_packages, visiting, visited);
            }
        }

        sorted_packages.push_back(sort_context.current);

        visiting.remove(sort_context.project_package.id);
        visited.insert(sort_context.project_package.id);
    }

    static bool hasDirtyDependency(const Project::Context& context, const Project::Package& package, const DirtyPackageRefs& references) {
        for (const auto& dependency_id : package.dependencies) {
            if (references.contains(dependency_id)) {
                return true;
            }

            if (!context.packages.contains(dependency_id)) {
                continue;
            }

            auto& dependency = context.packages.at(dependency_id);

            if (dependency.type == Project::PackageType::StaticLibrary && hasDirtyDependency(context, dependency, references)) {
                return true;
            }
        }

        return false;
    }

    static void propagateStaticLibraryChanges(
        const Project::Context& context,
        const Source::Context& sources,
        const std::vector<std::string>& ignored_packages,
        DirtyPackages& packages,
        DirtyPackageRefs& references) {
        for (const auto& package_id : ignored_packages) {
            auto& project_package = context.packages.at(package_id);

            if (project_package.type == Project::PackageType::StaticLibrary) {
                continue;
            }

            if (!hasDirtyDependency(context, project_package, references)) {
                continue;
            }

            auto dirty_package = DirtyPackage(package_id);

            for (const auto& [source_path, source_info] : sources.at(package_id)) {
                dirty_package.objects.push_back(source_info.output);
            }

            references.insert(package_id, packages.size());
            packages.push_back(dirty_package);
        }
    }

    DirtyPackages GetDirtyPackages(const Project::Context& project_context, std::optional<std::string> package_id, bool force_build) {
        auto source_context = Source::InitContext(project_context);
        auto candidates = loadCandidates(project_context, package_id);

        auto packages = DirtyPackages();
        auto references = DirtyPackageRefs();
        auto ignored_packages = std::vector<std::string>();

        for (const auto& package_id : candidates) {
            auto dirty_package = DirtyPackage(package_id);
            auto project_package = project_context.packages.at(package_id);
            auto package_output = Utils::GetOutputFile(project_context, project_package);
            auto should_rebuild = force_build || !stdext::fs::is_regular_file(package_output);

            for (const auto& [source_path, source_info] : source_context.at(package_id)) {
                auto source_output = Utils::GetObjectFile(project_context, package_id, source_info.output);
                auto source_rebuild = should_rebuild || !stdext::fs::is_regular_file(source_output);

                dirty_package.objects.push_back(source_info.output);

                if (source_rebuild || source_info.current != source_info.previous) {
                    dirty_package.sources.push_back(source_info);
                }
            }

            if (dirty_package.sources.empty()) {
                ignored_packages.push_back(package_id);
                continue;
            }

            references.insert(package_id, packages.size());
            packages.push_back(dirty_package);
        }

        propagateStaticLibraryChanges(project_context, source_context, ignored_packages, packages, references);

        auto sorted_packages = DirtyPackages();
        auto visiting = Candidates();
        auto visited = Candidates();

        for (const auto& package : packages) {
            if (!visited.contains(package.id)) {
                auto package_info = project_context.packages.at(package.id);
                auto sort_context = SortContext(package, packages, references, package_info, project_context);

                topologicalSort(sort_context, sorted_packages, visiting, visited);
            }
        }

        return sorted_packages;
    }
}
