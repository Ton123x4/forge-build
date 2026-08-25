#pragma once

#include "core/project.h"

#include <string>

namespace Utils {
    std::string GetOutputDirectory(const std::string& output_file);

    std::string GetObjectFile(const Project::Context& context, const std::string& package_id, const std::string& output_file);

    std::string GetOutputFile(const Project::Context& context, const std::string& output_file);

    std::string GetOutputFile(const Project::Context& context, const Project::Package& package);

    std::string GetDependencyDirectory(const Project::Context& context, const std::string& dependency_id);

    std::string GetLibraryFile(const Project::Context& context, std::string output_file);
}
