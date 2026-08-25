#pragma once

#include "stdext/hash_set.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/version.hpp"
#include <stdexcept>
#include <string>

namespace Project {
    template<typename Type>
    using Dictionary = stdext::hash_map<std::string, Type>;
    using StringArray = stdext::hash_set<std::string>;

    enum struct Profile {
        Debug,
        Preview,
        Release,
        Complete
    };

    enum struct PackageType {
        Executable,
        StaticLibrary,
        SharedLibrary
    };

    struct Dependency {
        std::string id;
        StringArray defines;
        StringArray includes;
        StringArray libpaths;
        StringArray libraries;
        StringArray dependencies;
    };

    struct Package {
        std::string id;
        PackageType type;
        std::string path;
        std::string output;
        StringArray flags;
        StringArray cflags;
        StringArray bflags;
        StringArray defines;
        StringArray includes;
        StringArray sources;
        StringArray libpaths;
        StringArray libraries;
        StringArray dependencies;
    };

    struct Resource {
        std::string from;
        std::string to;
    };

    struct BuildConfig {
        std::string compiler;
        std::string archiver;
    };

    struct Context {
        Profile profile;
        std::string base_path;
        std::string build_path;
        std::string cache_path;
        std::string output_path;
        StringArray global_defines;
        StringArray global_includes;
        std::vector<Resource> resources;
        StringArray tests;
        BuildConfig build_config;
        stdext::version_info version;
        Dictionary<Package> packages;
        Dictionary<Dependency> dependencies;
    };

    struct ProjectLoaderError : public std::runtime_error {
        explicit ProjectLoaderError(const std::string& message)
            : std::runtime_error(message) {}
    };

    Context InitContext(Profile mode, const StringArray& env, const StringArray& defines, const std::string& project_path);
}
