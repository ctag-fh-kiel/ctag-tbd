#pragma once

#include <tbd/system/port/heaps.hpp>

#ifndef TBD_HEAPS_8BIT
    #error "port has to provide TBD_HEAPS_SPIRAM flag"
#endif

#ifndef TBD_HEAPS_SPIRAM
    #error "port has to provide TBD_HEAPS_SPIRAM flag"
#endif

#ifndef TBD_HEAPS_INTERNAL
    #error "port has to provide TBD_HEAPS_INTERNAL flag"
#endif

#ifndef TBD_HEAPS_DEFAULT
    #error "port has to provide TBD_HEAPS_DEFAULT flag"
#endif
