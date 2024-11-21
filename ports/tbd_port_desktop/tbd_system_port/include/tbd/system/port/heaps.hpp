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
