#pragma once

#include <tbd/esp32_only.hpp>

#include <esp_heap_caps.h>

#include <tbd/system/common/heaps_pre.hpp>


#if (MALLOC_CAP_8BIT != TBD_HEAPS_8BIT \
    || MALLOC_CAP_SPIRAM != TBD_HEAPS_SPIRAM \
    || MALLOC_CAP_INTERNAL != TBD_HEAPS_INTERNAL \
    || MALLOC_CAP_DEFAULT != TBD_HEAPS_DEFAULT)
#error "idf heap cap interface diverges from TBD heaps"
#endif


TBD_C_BEGIN

inline void* tbd_heaps_malloc(unsigned int size, unsigned int caps) {
    return heap_caps_malloc(size, caps);
}

inline void* tbd_heaps_malloc_prefer(unsigned int size, unsigned int num, ...) {
    va_list pref_list;
    return heap_caps_malloc_prefer(size, num, pref_list);
}

inline void tbd_heaps_free(void* ptr) {
    heap_caps_free(ptr);
}

inline void* tbd_heaps_calloc(unsigned int n, unsigned int size, unsigned int caps) {
    return heap_caps_calloc(n, size, caps);
}

inline void* tbd_heaps_realloc(void* ptr, unsigned int size, unsigned int caps) {
    return heap_caps_realloc(ptr, size, caps);
}

inline int tbd_heaps_get_free_size(unsigned int caps) {
    return heap_caps_get_free_size(caps);
}

inline int tbd_heaps_get_largest_free_block(unsigned int caps) {
    return heap_caps_get_largest_free_block(caps);
}

TBD_C_END
