#pragma once

#include "project.h"
#include "stdext/hash_map.hpp"

#include <stdexcept>
#include <string>

namespace Source {
    using Package = stdext::hash_map<std::string, struct SourceInfo>;
    using Context = stdext::hash_map<std::string, Package>;

    struct SourceInfo {
        std::string path;
        std::string output;
        std::string current;
        std::string previous;
    };

    struct SourceError : public std::runtime_error {
        explicit SourceError(const std::string& message)
            : std::runtime_error(message) {}
    };

    Context InitContext(const Project::Context& project_context);

    void StoreContext(const Context& context, const std::string& build_directory);
}
