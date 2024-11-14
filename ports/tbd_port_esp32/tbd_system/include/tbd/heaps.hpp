/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/
#pragma once

#include <esp_heap_caps.h>

#include <tbd/private/heaps_caps.hpp>



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
    va_list argp;
    return heap_caps_malloc_prefer(size, num, argp);
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

#include <tbd/private/heaps_cpp.hpp>
