#include "core/utils.h"

#include "core/constants.h"
#include "core/project.h"
#include "stdext/filesystem.hpp"
#include "stdext/handler.hpp"

#include <string>

namespace Utils {
    std::string GetOutputDirectory(const std::string& output_file) {
        return stdext::pipe(output_file, stdext::fs::parent_path, stdext::fs::sanitize_posix);
    }

    std::string GetObjectFile(const Project::Context& context, const std::string& package_id, const std::string& output_file) {
        return stdext::fs::sanitize_posix(stdext::fs::join_path(context.build_path, package_id, output_file));
    }

    std::string GetOutputFile(const Project::Context& context, const std::string& output_file) {
        return stdext::fs::sanitize_posix(stdext::fs::join_path(context.output_path, output_file));
    }

    std::string GetOutputFile(const Project::Context& context, const Project::Package& package) {
        return GetOutputFile(context, package.output);
    }

    std::string GetDependencyDirectory(const Project::Context& context, const std::string& dependency_id) {
        return stdext::fs::sanitize_posix(stdext::fs::join_path(context.base_path, FORGE_DEPENDENCIES_PATH, dependency_id));
    }

    std::string GetLibraryFile(const Project::Context& context, std::string output_file) {
        if (stdext::fs::extension(output_file) == FORGE_WINDOWS_SHARED_LIBRARY_EXT) {
            output_file = stdext::fs::replace_extension(output_file, FORGE_WINDOWS_STATIC_LIBRARY_EXT);
        }

        return GetOutputFile(context, output_file);
    }
}
