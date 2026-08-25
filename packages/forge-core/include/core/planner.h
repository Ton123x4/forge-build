#pragma once

#include "project.h"
#include "source.h"
#include <vector>
#include <optional>

namespace Planner {
    using DirtyPackages = std::vector<struct DirtyPackage>;

    struct DirtyPackage {
        std::string id;
        std::vector<std::string> objects;
        std::vector<Source::SourceInfo> sources;
    };

    DirtyPackages GetDirtyPackages(const Project::Context& project, std::optional<std::string> package, bool force_build);
}
