#include "core/source.h"

#include "core/constants.h"
#include "core/dependency.h"
#include "core/project.h"
#include "stdext/crypto.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/string.hpp"

#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>

namespace Source {
    const stdext::hash_set<std::string_view> kSourceExtensions = {
        // C e C++ Standard
        ".c", ".cc", ".cpp", ".cxx", ".c++",

        // Objective-C / Objective-C++
        ".m", ".mm",

        // Assembly files
        ".s", ".S", ".asm",

        // Windows Resources
        ".res"
    };


    auto getFingerprintPath(const std::string& cache_path, const std::string& id) {
        auto fingerprint_file = std::format("{}{}", id, FORGE_FINGERPRINT_FILE);
        auto fingerprint_path = stdext::fs::join_path(cache_path, fingerprint_file);

        return fingerprint_path;
    }

    auto loadFingerprintFile(const std::string& filepath) {
        auto context = stdext::hash_map<std::string, std::string>();

        if (!stdext::fs::is_regular_file(filepath)) {
            return context;
        }

        try {
            for (const auto& text_line : stdext::fs::read_lines(filepath)) {
                if (text_line.empty()) {
                    continue;
                }

                auto entries = stdext::string::split(text_line, '#');

                if (entries.size() != 2) {
                    throw SourceError(std::format("malformed fingerprint line: '{}'", text_line));
                }

                auto source_hash = entries.at(0);
                auto dependencies_hash = entries.at(1);

                if (source_hash.empty()) {
                    throw SourceError(std::format(": '{}'", text_line));
                }

                if (dependencies_hash.empty()) {
                    throw SourceError(std::format(" '{}'", source_hash));
                }

                context.insert(source_hash, dependencies_hash);
            }

            return context;
        }
        catch (const std::runtime_error& error) {
            throw SourceError(std::format("SourceError: {}", error.what()));
        }
    }

    auto computeSourceHash(const Project::Context& project_context, const std::string& package_id, const std::string& source_file) {
        auto dependency_file = stdext::fs::replace_extension(source_file, FORGE_DEPENDENCY_FILE);
        auto dependency_path = stdext::fs::join_path(project_context.build_path, package_id, dependency_file);

        if (stdext::fs::is_regular_file(dependency_path)) {
            auto dependencies = DependencyFile::ParseDependencies(dependency_path);
            auto global_hash = stdext::fs::hash_files(dependencies);

            return std::format("{}", global_hash);
        }

        return std::string("0");
    }

    auto computePathHash(const std::string& filepath) {
        return std::format("{}", stdext::crypto::hash::fnv(filepath.c_str(), filepath.length()));
    }

    auto processPackage(const Project::Context& project_context, const Project::Package& project_package, Context& context) {
        auto fingerprint_path = getFingerprintPath(project_context.cache_path, project_package.name);
        auto fingerprint_kv = loadFingerprintFile(fingerprint_path);
        auto package_sources = Source::Package();

        for (const auto& source_name : project_package.sources) {
            auto source_dir = stdext::fs::join_path(project_package.path, source_name);

            if (!stdext::fs::is_directory(source_dir)) {
                throw SourceError(std::format("Source directory not found: {}", source_dir));
            }

            for (const auto& entry : stdext::fs::directory_iterator(source_dir)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                auto source_ext = stdext::fs::extension(entry);

                if (!kSourceExtensions.contains(source_ext)) {
                    continue;
                }

                auto source_path = stdext::fs::sanitize_posix(stdext::fs::relative(entry));
                auto output_file = stdext::fs::replace_extension(source_path, FORGE_OBJECT_FILE);
                auto source_info = Source::SourceInfo(source_path, output_file);
                auto path_hash = computePathHash(source_path);

                source_info.current = computeSourceHash(project_context, project_package.name, source_path);

                if (fingerprint_kv.contains(path_hash)) {
                    source_info.previous = fingerprint_kv.at(path_hash);
                }

                package_sources.insert(source_path, source_info);
            }
        }

        context.insert(project_package.name, package_sources);
    }

    Context InitContext(const Project::Context& project_context) {
        auto context = Context();

        for (const auto& package : project_context.packages) {
            processPackage(project_context, package.second, context);
        }

        return context;
    }

    void StoreContext(const Context& context, const std::string& cache_path) {
        try {
            for (const auto& [package_id, cache_package] : context) {
                auto lines = std::vector<std::string>();

                for (const auto& [source_path, source_hash] : cache_package) {
                    lines.push_back(std::format("{}#{}", computePathHash(source_path), source_hash.current));
                }

                stdext::fs::write_file(getFingerprintPath(cache_path, package_id), stdext::string::join(lines, "\n"));
            }
        }
        catch (const std::runtime_error& error) {
            throw SourceError(std::format("SourceError: {}", error.what()));
        }
    }
}
