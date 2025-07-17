#pragma once

#include <tbd/host_only.hpp>

#include <stdlib.h>
#include <tbd/system/common/heaps_pre.hpp>

TBD_C_BEGIN

inline void* tbd_heaps_malloc(unsigned int size, unsigned int caps) {
    return malloc(size);
}

inline void* tbd_heaps_malloc_prefer(unsigned int size, unsigned int num, ...) {
    return tbd_heaps_malloc(size, TBD_HEAPS_DEFAULT);
}

inline void tbd_heaps_free(void* ptr) {
    free(ptr);
}

inline void* tbd_heaps_calloc(unsigned int n, unsigned int size, unsigned int caps) {
    return calloc(n, size);
}

inline void* tbd_heaps_realloc(void* ptr, unsigned int size, unsigned int caps) {
    return realloc(ptr, size);
}

inline int tbd_heaps_get_free_size(unsigned int caps) {
    return 4*1024*1024;
}

inline int tbd_heaps_get_largest_free_block(unsigned int caps) {
    return 3*1024*1024; // 3 Megs
}

TBD_C_END
