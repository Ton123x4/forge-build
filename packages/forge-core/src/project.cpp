#include "core/project.h"

#include <format>
#include <string>

#include "core/constants.h"
#include "stdext/filesystem.hpp"
#include "stdext/forge.hpp"
#include "stdext/hash_map.hpp"
#include "stdext/json.hpp"
#include "stdext/string.hpp"
#include "stdext_sys.h"

namespace Project {
    constexpr auto kManifestFilename = "forge.json";
    constexpr auto kVersionFilename = "version.txt";
    constexpr auto kManifestVariables = "storage";
    constexpr auto kManifestDefines = "defines";
    constexpr auto kManifestIncludes = "includes";
    constexpr auto kManifestPackages = "packages";
    constexpr auto kManifestDependencies = "dependencies";
    constexpr auto kManifestResources = "resources";
    constexpr auto kManifestTests = "tests";
    constexpr auto kDefaultKey = "default";

    const char* kBuildDirectories[] = {
        FORGE_INT_DEBUG_PATH,
        FORGE_INT_PREVIEW_PATH,
        FORGE_INT_RELEASE_PATH
    };

    const char* kOutputDirectories[] = {
        FORGE_DEBUG_PATH,
        FORGE_PREVIEW_PATH,
        FORGE_RELEASE_PATH
    };

    const auto kPackageTypes = stdext::hash_map<std::string, PackageType>{
        { "executable", PackageType::Executable },
        { "library", PackageType::StaticLibrary },
        { "shared", PackageType::SharedLibrary }
    };

    const auto kOutputExtensions = stdext::hash_map<PackageType, std::string>{
        { PackageType::Executable, STDEXT_EXECUTABLE_EXT },
        { PackageType::StaticLibrary, STDEXT_STATIC_LIBRARY_EXT },
        { PackageType::SharedLibrary, STDEXT_SHARED_LIBRARY_EXT }
    };

    struct InternalContext {
        Context& project;
        const StringArray& environment;
        Dictionary<std::string> storage;
    };

    auto resolveManifestPath(std::string filepath) {
        if (stdext::fs::is_regular_file(filepath)) {
            return filepath;
        }

        return stdext::fs::join_path(filepath, kManifestFilename);
    }

    auto resolvePackagePath(const InternalContext& context, const std::string& filepath) {
        return stdext::fs::join_path(context.project.base_path, filepath);
    }

    auto resolveOutputName(PackageType type, const std::string& filepath) {
        return std::format("{}{}", filepath, kOutputExtensions.at(type));
    }

    /// Internal

    auto parseStringValue(const InternalContext& context, std::string string_value) {
        for (const auto& [name, value] : context.storage) {
            string_value = stdext::string::replace_all(string_value, "$(" + name + ")", value);
        }

        return string_value;
    }

    auto parseStringNode(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto string_node = stdext::json_string::take_dynamic(node);

        if (string_node == nullptr) {
            throw ProjectLoaderError("Expected a string value");
        }

        return parseStringValue(context, string_node->value);
    }

    std::string parseSelectString(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto object_node = stdext::json_object::take_dynamic(node);

        if (object_node == nullptr) {
            throw ProjectLoaderError("Expected an object value");
        }

        for (const auto& [name, value] : object_node->values) {
            if (kDefaultKey == name) {
                continue;
            }

            if (context.environment.contains(name)) {
                if (value->type == stdext::json_type::object) {
                    return parseSelectString(context, value);
                }

                return parseStringNode(context, value);
            }
        }

        if (object_node->values.contains(kDefaultKey)) {
            return parseStringNode(context, object_node->values.at(kDefaultKey));
        }

        return {};
    }

    auto parseArrayNode(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto array_node = stdext::json_array::take_dynamic(node);

        if (array_node == nullptr) {
            throw ProjectLoaderError("Expected a string array value");
        }

        auto string_array = StringArray();

        for (auto& value : array_node->values) {
            if (value->type == stdext::json_type::object) {
                string_array.insert(parseSelectString(context, value));
            }
            else {
                string_array.insert(parseStringNode(context, value));
            }
        }

        return string_array;
    }

    auto parseSelectArray(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto object_node = stdext::json_object::take_dynamic(node);

        if (object_node == nullptr) {
            throw ProjectLoaderError("Expected an object value");
        }

        for (const auto& [name, value] : object_node->values) {
            if (kDefaultKey == name) {
                continue;
            }

            if (context.environment.contains(name)) {
                return parseArrayNode(context, value);
            }
        }

        if (object_node->values.contains(kDefaultKey)) {
            return parseArrayNode(context, object_node->values.at(kDefaultKey));
        }

        return StringArray();
    }

    auto parseRequiredString(const InternalContext& context, stdext::json_object::safe_ptr& object, const std::string& key) {
        auto value = object->values.find(key);

        if (value == object->values.end()) {
            throw ProjectLoaderError(std::format("Missing required key '{}'", key));
        }

        return parseStringNode(context, value->second);
    }

    auto parseOptionalString(const InternalContext& context, stdext::json_object::safe_ptr& object, const std::string& key) {
        auto value = object->values.find(key);

        if (value == object->values.end()) {
            return std::string();
        }

        if (value->second->type == stdext::json_type::object) {
            return parseSelectString(context, value->second);
        }

        return parseStringNode(context, value->second);
    }

    auto parseStringArray(const InternalContext& context, stdext::json_object::safe_ptr& object, const std::string& key) {
        auto value = object->values.find(key);

        if (value == object->values.end()) {
            return StringArray();
        }

        if (value->second->type == stdext::json_type::object) {
            return parseSelectArray(context, value->second);
        }

        return parseArrayNode(context, value->second);
    }

    /// Standard Properties

    auto parseVariablesNode(InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto storage = stdext::json_object::take_dynamic(node);

        if (storage == nullptr) {
            throw ProjectLoaderError("Expected an object value for 'storage'");
        }

        for (const auto& [name, value] : storage->values) {
            if (value->type == stdext::json_type::object) {
                context.storage.insert(name, parseSelectString(context, value));
            }
            else {
                context.storage.insert(name, parseStringNode(context, value));
            }
        }
    }

    auto parseDependencyNode(InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto dependency_node = stdext::json_object::take_dynamic(node);

        if (dependency_node == nullptr) {
            throw ProjectLoaderError("Expected an object value for dependency");
        }

        auto dependency_config = Dependency();

        dependency_config.name = parseRequiredString(context, dependency_node, "name");
        dependency_config.defines = parseStringArray(context, dependency_node, "defines");
        dependency_config.includes = parseStringArray(context, dependency_node, "includes");
        dependency_config.libpaths = parseStringArray(context, dependency_node, "libpaths");
        dependency_config.libraries = parseStringArray(context, dependency_node, "libraries");
        dependency_config.dependencies = parseStringArray(context, dependency_node, "requires");

        context.project.dependencies.insert(dependency_config.name, dependency_config);
    }

    auto parseDependenciesNode(InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto dependencies = stdext::json_array::take_dynamic(node);

        if (dependencies == nullptr) {
            throw ProjectLoaderError("Expected an array value for 'dependencies'");
        }

        for (auto& dependency : dependencies->values) {
            parseDependencyNode(context, dependency);
        }
    }

    auto parsePackageNode(InternalContext& context, const std::string& base_path, stdext::json_node::safe_ptr& node) {
        auto package_node = stdext::json_object::take_dynamic(node);

        if (package_node == nullptr) {
            throw ProjectLoaderError("Expected an object value for package");
        }

        auto package_config = Package{ .type = PackageType::Executable, .path = base_path };
        auto package_type = parseRequiredString(context, package_node, "type");

        if (!package_type.empty()) {
            if (!kPackageTypes.contains(package_type)) {
                throw ProjectLoaderError(std::format("Unknown package type '{}'", package_type));
            }

            package_config.type = kPackageTypes.at(package_type);
        }

        auto package_name = parseRequiredString(context, package_node, "name");
        auto package_target = parseOptionalString(context, package_node, "target");
        auto package_output = parseOptionalString(context, package_node, "output");

        package_config.name = package_name;

        if (!package_output.empty()) {
            package_config.output = package_output;
        }
        else if (!package_target.empty()) {
            package_config.output = resolveOutputName(package_config.type, package_target);
        }
        else {
            package_config.output = resolveOutputName(package_config.type, package_name);
        }

        package_config.flags = parseStringArray(context, package_node, "flags");
        package_config.cflags = parseStringArray(context, package_node, "cflags");
        package_config.bflags = parseStringArray(context, package_node, "bflags");
        package_config.defines = parseStringArray(context, package_node, "defines");
        package_config.includes = parseStringArray(context, package_node, "includes");
        package_config.sources = parseStringArray(context, package_node, "sources");
        package_config.libpaths = parseStringArray(context, package_node, "libpaths");
        package_config.libraries = parseStringArray(context, package_node, "libraries");
        package_config.dependencies = parseStringArray(context, package_node, "requires");

        context.project.packages.insert(package_name, package_config);
    }

    auto parsePackagesNode(InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto packages = stdext::json_array::take_dynamic(node);

        if (packages == nullptr) {
            throw ProjectLoaderError("Expected an array value for 'packages'");
        }

        for (auto& package : packages->values) {
            if (package->type == stdext::json_type::object) {
                parsePackageNode(context, context.project.base_path, package);
                continue;
            }

            auto relative_path = parseStringNode(context, package);
            auto package_path = resolvePackagePath(context, relative_path);
            auto package_file = resolveManifestPath(package_path);
            auto package_dir = stdext::fs::parent_path(package_file);
            auto package_root = stdext::json::parse_file(package_file);

            parsePackageNode(context, package_dir, package_root);
        }
    }

    auto parseResourceNode(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        if (node->type != stdext::json_type::object) {
            auto path = parseStringNode(context, node);
            auto resource = Resource(path, path);

            return resource;
        }

        auto resource_node = stdext::json_object::take_dynamic(node);

        if (resource_node == nullptr) {
            throw ProjectLoaderError("Expected a string or an object value for resource");
        }

        auto resource_config = Resource();

        resource_config.from = parseRequiredString(context, resource_node, "from");
        resource_config.to = parseRequiredString(context, resource_node, "to");

        return resource_config;
    }

    auto parseResourcesNode(const InternalContext& context, stdext::json_node::safe_ptr& node) {
        auto resources = stdext::json_array::take_dynamic(node);

        if (resources == nullptr) {
            throw ProjectLoaderError("Expected an array value for 'resources'");
        }

        auto resource_list = std::vector<Resource>();

        for (auto& resource : resources->values) {
            resource_list.push_back(parseResourceNode(context, resource));
        }

        return resource_list;
    }

    void initProjectVersion(Context& context) {
        auto version_path = stdext::fs::join_path(context.base_path, kVersionFilename);

        if (!stdext::fs::is_regular_file(version_path)) {
            return;
        }

        context.version = stdext::load_version_file(version_path);
    }

    void initBuildConfig(Context& context) {
#ifdef __linux__
        context.build_config.compiler = "gcc";
        context.build_config.archiver = "ar";
#else
        context.build_config.compiler = "clang";
        context.build_config.archiver = "llvm-ar";
#endif
    }

    void initProjectPaths(Context& context, Profile mode) {
        auto dir_index = static_cast<int>(mode);

        if (mode == Profile::Complete) {
            if constexpr (stdext::is_debug) {
                throw ProjectLoaderError("Unknwon profile type");
            }

            throw ProjectLoaderError("Internal project error");
        }

        context.build_path = stdext::fs::join_path(context.base_path, kBuildDirectories[dir_index]);
        context.output_path = stdext::fs::join_path(context.base_path, kOutputDirectories[dir_index]);
        context.cache_path = stdext::fs::join_path(context.build_path, FORGE_CACHE_DIR);
    }

    void updateBuildConfig(const InternalContext& context) {
        if (context.storage.contains("compiler")) {
            context.project.build_config.compiler = context.storage.at("compiler");
        }

        if (context.storage.contains("archiver")) {
            context.project.build_config.archiver = context.storage.at("archiver");
        }
    }

    void checkPackageDependencies(const InternalContext& context) {
        for (const auto& [id, package] : context.project.dependencies) {
            for (const auto& dependency_id : package.dependencies) {
                if (!context.project.dependencies.contains(dependency_id)) {
                    throw ProjectLoaderError(std::format("External package '{}' depends on unknown package '{}'", id, dependency_id));
                }
            }
        }

        for (const auto& [id, package] : context.project.packages) {
            for (const auto& dependency_id : package.dependencies) {
                if (!context.project.packages.contains(dependency_id) && !context.project.dependencies.contains(dependency_id)) {
                    throw ProjectLoaderError(std::format("Package '{}' depends on unknown package '{}'", id, dependency_id));
                }
            }
        }
    }

    Context InitContext(Profile mode, const StringArray& env, const StringArray& defines, const std::string& project_path) {
        auto project_file = resolveManifestPath(project_path);
        auto project_dir = stdext::fs::parent_path(project_file);
        auto project_node = stdext::json::parse_file(project_file);
        auto project_root = stdext::json_object::dynamic(project_node);

        if (project_root == nullptr) {
            throw ProjectLoaderError("Expected an object value at project root");
        }

        auto project_context = Context(mode, stdext::fs::sanitize_posix(project_dir));
        auto internal_context = InternalContext(project_context, env);

        initProjectVersion(project_context);
        initBuildConfig(project_context);
        initProjectPaths(project_context, mode);

        if (project_root->values.contains(kManifestVariables)) {
            parseVariablesNode(internal_context, project_root->values.at(kManifestVariables));
        }

        updateBuildConfig(internal_context);

        project_context.global_defines = defines;

        if (!project_context.base_path.empty()) {
            project_context.global_includes.insert(project_context.base_path);
        }

        if (project_root->values.contains(kManifestDefines)) {
            for (const auto& entry : parseArrayNode(internal_context, project_root->values.at(kManifestDefines))) {
                internal_context.project.global_defines.insert(entry);
            }
        }

        if (project_root->values.contains(kManifestIncludes)) {
            for (const auto& entry : parseArrayNode(internal_context, project_root->values.at(kManifestIncludes))) {
                internal_context.project.global_includes.insert(entry);
            }
        }

        if (project_root->values.contains(kManifestDependencies)) {
            parseDependenciesNode(internal_context, project_root->values.at(kManifestDependencies));
        }

        if (project_root->values.contains(kManifestPackages)) {
            parsePackagesNode(internal_context, project_root->values.at(kManifestPackages));
        }

        if (project_root->values.contains(kManifestResources)) {
            internal_context.project.resources = parseResourcesNode(internal_context, project_root->values.at(kManifestResources));
        }

        checkPackageDependencies(internal_context);

        if (project_root->values.contains(kManifestTests)) {
            internal_context.project.tests = parseArrayNode(internal_context, project_root->values.at(kManifestTests));
        }

        return project_context;
    }
}
