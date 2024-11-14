#pragma once

#include <tbd/header_utils.hpp>

#if TBD_IS_CPP

namespace tbd {
namespace heaps {

inline void* malloc(unsigned int size, unsigned int caps) {
    return heap_caps_malloc(size, caps);
}

inline void* malloc_prefer(unsigned int size, unsigned int num, ...) {
    va_list argp;
    return tbd_heaps_malloc_prefer(size, num, argp);
}

inline void free(void* ptr) {
    tbd_heaps_free(ptr);
}

inline void* calloc(unsigned int n, unsigned int size, unsigned int caps) {
    return tbd_heaps_calloc(n, size, caps);
}

inline void* realloc(void* ptr, unsigned int size, unsigned int caps) {
    return tbd_heaps_realloc(ptr, size, caps);
}

inline int get_free_size(unsigned int caps) {
    return tbd_heaps_get_free_size(caps);
}

inline int get_largest_free_block(unsigned int caps) {
    return tbd_heaps_get_largest_free_block(caps);
}

}
}

#endif
