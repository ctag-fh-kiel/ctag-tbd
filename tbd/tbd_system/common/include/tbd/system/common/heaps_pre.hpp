#pragma once

#include <tbd/system/common/header_utils.hpp>

// heap types

#define TBD_HEAPS_8BIT     (1 << 2)
#define TBD_HEAPS_SPIRAM   (1 << 10)
#define TBD_HEAPS_INTERNAL (1 << 11)
#define TBD_HEAPS_DEFAULT  (1 << 12)

// exptected signatures

TBD_C_BEGIN

inline void* tbd_heaps_malloc(unsigned int size, unsigned int caps);
inline void* tbd_heaps_malloc_prefer(unsigned int size, unsigned int num, ...);
inline void  tbd_heaps_free(void* ptr);
inline void* tbd_heaps_calloc(unsigned int n, unsigned int size, unsigned int caps);
inline void* tbd_heaps_realloc(void* ptr, unsigned int size, unsigned int caps);
inline int   tbd_heaps_get_free_size(unsigned int caps);
inline int   tbd_heaps_get_largest_free_block(unsigned int caps);

TBD_C_END

// namespaced cpp versions

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
