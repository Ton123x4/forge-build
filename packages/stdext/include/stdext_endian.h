#pragma once

#define STDEXT_BSWAP16(x) __builtin_bswap16(x)
#define STDEXT_BSWAP32(x) __builtin_bswap32(x)
#define STDEXT_BSWAP64(x) __builtin_bswap64(x)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define STDEXT_LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define STDEXT_BIG_ENDIAN
#endif
