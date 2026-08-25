#pragma once

#include <string>
#include <vector>

namespace DependencyFile {
    std::vector<std::string> ParseDependencies(const std::string& filepath);
}
