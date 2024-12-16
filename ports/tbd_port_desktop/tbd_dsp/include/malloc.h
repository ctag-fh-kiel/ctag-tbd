// compat_malloc.h
#ifndef COMPAT_MALLOC_H
#define COMPAT_MALLOC_H

#ifdef __clang__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#endif // COMPAT_MALLOC_H