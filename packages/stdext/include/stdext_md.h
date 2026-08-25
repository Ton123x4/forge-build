#pragma once

#ifdef _WIN32
#define STDEXT_EXPORT __declspec(dllexport)
#define STDEXT_IMPORT __declspec(dllimport)
#define STDEXT_HIDDEN
#else
#define STDEXT_EXPORT __attribute__((visibility("default")))
#define STDEXT_IMPORT
#define STDEXT_HIDDEN __attribute__((visibility("hidden")))
#endif

#ifdef __cplusplus
#define STDEXT_LIB_START    extern "C" {
#define STDEXT_LIB_END      }
#else
#define STDEXT_LIB_START
#define STDEXT_LIB_END
#endif
