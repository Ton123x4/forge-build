#pragma once

#include <string>

namespace stdext {
    void* load_library(const std::string& filepath);

    void* get_symbol(void* handle, const std::string& name);

    void unload_library(void* handle);
}
