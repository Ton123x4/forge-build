#pragma once

#include "stdext/time.hpp"
#include "stdext_forge.h"
#include "stdext_sys.h"

#include <cstdio>
#include <format>
#include <string>

namespace stdext {
    constexpr auto is_debug = bool(FORGE_IS_DEBUG);
    constexpr auto is_release = bool(FORGE_IS_RELEASE);
    constexpr auto is_preview = bool(FORGE_IS_PREVIEW);

    namespace forge {
        #ifdef FORGE_BUILD_TOOL
        constexpr auto build_tool = true;
        #else
        constexpr auto build_tool = false;
        #endif

        #ifdef FORGE_BUILD_TOOL
        constexpr auto product_type = FORGE_PRODUCT_TYPE;
        constexpr auto version_first = FORGE_VERSION_FIRST;
        constexpr auto version_second = FORGE_VERSION_SECOND;
        constexpr auto version_build = FORGE_VERSION_BUILD;
        constexpr auto version_timestamp = FORGE_VERSION_TIMESTAMP;
        #endif

        static inline void print_build_info(const std::string& name, bool show_timestamp = false) {
            #ifdef FORGE_BUILD_TOOL
            std::printf("%s %d.%d.%d (%s)\n", name.c_str(), version_first, version_second, version_build, product_type);

            if (show_timestamp) {
                auto local_time = time::to_zoned_time(FORGE_VERSION_TIMESTAMP);
                auto build_date = std::format("{:%d/%m/%Y %H:%M:%S}", local_time);

                std::printf("Built: %s\n", build_date.c_str());
            }
            #else
            std::printf("%s\n", name.c_str());
            #endif

            std::printf("Target: %s\n", STDEXT_TARGET_TRIPLE_COMPLETE);
        }
    }
}
