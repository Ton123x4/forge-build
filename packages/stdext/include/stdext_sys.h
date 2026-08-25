#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#define STDEXT_PLATFORM_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define STDEXT_PLATFORM_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define STDEXT_PLATFORM_NAME "arm64"
#elif defined(__arm__) || defined(_M_ARM)
#define STDEXT_PLATFORM_NAME "arm"
#elif defined(__riscv)
#if __riscv_xlen == 64
#define STDEXT_PLATFORM_NAME "riscv64"
#else
#define STDEXT_PLATFORM_NAME "riscv32"
#endif
#else
#define STDEXT_PLATFORM_NAME "unknown"
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define STDEXT_VENDOR_NAME "apple"
#elif defined(__MINGW64__)
#define STDEXT_VENDOR_NAME "w64"
#elif defined(_WIN32) || defined(__CYGWIN__)
#define STDEXT_VENDOR_NAME "pc"
#elif defined(__linux__)
#define STDEXT_VENDOR_NAME "pc"
#else
#define STDEXT_VENDOR_NAME "unknown"
#endif

#if defined(_WIN32)
#define STDEXT_SYSTEM_NAME "windows"
#elif defined(__linux__)
#define STDEXT_SYSTEM_NAME "linux"
#elif defined(__APPLE__) && defined(__MACH__)
#define STDEXT_SYSTEM_NAME "darwin"
#elif defined(__FreeBSD__)
#define STDEXT_SYSTEM_NAME "freebsd"
#else
#define STDEXT_SYSTEM_NAME "unknown"
#endif

#if defined(__MINGW64__)
#define STDEXT_ABI_NAME "mingw64"
#define STDEXT_MINGW_TOOLCHAIN
#elif defined(__MINGW32__)
#define STDEXT_ABI_NAME "mingw32"
#define STDEXT_MINGW_TOOLCHAIN
#elif defined(_MSC_VER)
#define STDEXT_ABI_NAME "msvc"
#elif defined(__ANDROID__)
#define STDEXT_ABI_NAME "android"
#elif defined(__clang__)
#define STDEXT_ABI_NAME "gnu"
#elif defined(__GNUC__)
#define STDEXT_ABI_NAME "gnu"
#else
#define STDEXT_ABI_NAME "unknown"
#endif

#ifdef _WIN32
#define STDEXT_PLATFORM_POSIX 0
#else
#define STDEXT_PLATFORM_POSIX 1
#endif

#if defined(_WIN32)
#define STDEXT_EXECUTABLE_EXT ".exe"
#define STDEXT_SHARED_LIBRARY_EXT ".dll"
#if defined(__MINGW32__) || defined(__MINGW64__)
#define STDEXT_STATIC_LIBRARY_EXT ".a"
#define STDEXT_IMPORT_LIBRARY_EXT ".dll"
#else
#define STDEXT_STATIC_LIBRARY_EXT ".lib"
#define STDEXT_IMPORT_LIBRARY_EXT ".lib"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#define STDEXT_EXECUTABLE_EXT ""
#define STDEXT_SHARED_LIBRARY_EXT ".dylib"
#define STDEXT_STATIC_LIBRARY_EXT ".a"
#define STDEXT_IMPORT_LIBRARY_EXT ".dylib"
#else
#define STDEXT_EXECUTABLE_EXT ""
#define STDEXT_SHARED_LIBRARY_EXT ".so"
#define STDEXT_STATIC_LIBRARY_EXT ".a"
#define STDEXT_IMPORT_LIBRARY_EXT ".so"
#endif

#define STDEXT_TARGET_NAME STDEXT_SYSTEM_NAME "/" STDEXT_PLATFORM_NAME
#define STDEXT_TARGET_TRIPLE STDEXT_PLATFORM_NAME "-" STDEXT_SYSTEM_NAME "-" STDEXT_ABI_NAME
#define STDEXT_TARGET_TRIPLE_COMPLETE STDEXT_PLATFORM_NAME "-" STDEXT_VENDOR_NAME "-" STDEXT_SYSTEM_NAME "-" STDEXT_ABI_NAME
