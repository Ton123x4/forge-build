#pragma once

#ifdef DEBUG
#define FORGE_PRODUCT_TYPE "Debug"
#define FORGE_IS_DEBUG   1
#define FORGE_IS_RELEASE 0
#define FORGE_IS_PREVIEW 0
#else
#ifdef RELEASE
#define FORGE_PRODUCT_TYPE "Release"
#define FORGE_IS_RELEASE 1
#define FORGE_IS_PREVIEW 0
#else
#define FORGE_PRODUCT_TYPE "Preview"
#define FORGE_IS_RELEASE 0
#define FORGE_IS_PREVIEW 1
#endif
#define FORGE_IS_DEBUG   0
#endif
