#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define STDEXT_LIKELY(x)   (__builtin_expect(!!(x), 1))
#define STDEXT_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define STDEXT_LIKELY(x)   (x)
#define STDEXT_UNLIKELY(x) (x)
#endif

#ifdef STDEXT_C23_EXTENSIONS
#ifndef likely
#define likely [[likely]]
#endif

#ifndef unlikely
#define unlikely [[unlikely]]
#endif
#endif
