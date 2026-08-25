#pragma once

#include "stdext_md.h"

#ifndef STDEXT_LIB_API
#ifdef STDEXT_LIB_IMPL
#define STDEXT_LIB_API      STDEXT_EXPORT
#else
#define STDEXT_LIB_API      STDEXT_IMPORT
#endif
#define STDEXT_LIB_HIDDEN   STDEXT_HIDDEN
#endif
