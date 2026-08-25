#include "stdext/library.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void* stdext::load_library(const std::string& filepath) {
    #if _WIN32
    return LoadLibraryA(filepath.c_str());
    #else
    return dlopen(filepath.c_str(), RTLD_NOW);
    #endif
}

void* stdext::get_symbol(void* handle, const std::string& name) {
    #if _WIN32
    return (void*)GetProcAddress((HMODULE)handle, name.c_str());
    #else
    return dlsym(handle, name.c_str());
    #endif
}

void stdext::unload_library(void* handle) {
    #if _WIN32
    FreeLibrary((HMODULE)handle);
    #else
    dlclose(handle);
    #endif
}
