#pragma once

/// Build output directories

#ifdef DEBUG
#define FORGE_DIRECTORY             ".forge_debug"
#else
#define FORGE_DIRECTORY             ".forge"
#endif

#define FORGE_BUILD_PATH            FORGE_DIRECTORY "/build"
#define FORGE_DEBUG_PATH            FORGE_DIRECTORY "/debug"
#define FORGE_PREVIEW_PATH          FORGE_DIRECTORY "/preview"
#define FORGE_RELEASE_PATH          FORGE_DIRECTORY "/release"

#define FORGE_INT_DEBUG_PATH        FORGE_BUILD_PATH "/debug"
#define FORGE_INT_PREVIEW_PATH      FORGE_BUILD_PATH "/preview"
#define FORGE_INT_RELEASE_PATH      FORGE_BUILD_PATH "/release"

#define FORGE_DEPENDENCIES_PATH     FORGE_DIRECTORY "/.dependencies"
#define FORGE_DEBUG_FILE            FORGE_DIRECTORY "/build_debug.log"
#define FORGE_COMMANDS_FILE         FORGE_DIRECTORY "/compile_commands.json"

/// Build artifacts

#define FORGE_CACHE_DIR             ".cache"
#define FORGE_LIBRARIES_DIR         ".libraries"
#define FORGE_FINGERPRINT_FILE      ".fingerprint"

#define FORGE_OBJECT_FILE           ".o"
#define FORGE_DEPENDENCY_FILE       ".d"

/// Project files

#define FORGE_CLANGD_FILE           ".clangd"
#define FORGE_PROJECT_FILE          "forge.json"
#define FORGE_VERSION_FILE          "version.txt"

/// Platform profile

#define FORGE_APPLE_TARGET      "darwin"
#define FORGE_WINDOWS_TARGET    "windows"

#define FORGE_POSIX_FAMILY      "posix"
#define FORGE_DARWIN_FAMILY     "darwin"

#define FORGE_WINDOWS_EXECUTABLE_EXT      ".exe"
#define FORGE_WINDOWS_STATIC_LIBRARY_EXT  ".lib"
#define FORGE_WINDOWS_SHARED_LIBRARY_EXT  ".dll"

#define FORGE_POSIX_EXECUTABLE_EXT        ""
#define FORGE_POSIX_STATIC_LIBRARY_EXT    ".a"
#define FORGE_POSIX_SHARED_LIBRARY_EXT    ".so"
