#include <tbd/system/port/ram.hpp>


#ifndef TBD_DRAM
    #error "system port needs to provide TBD_DRAM define"
#endif

#ifndef TBD_IRAM
    #error "system port needs to provide TBD_IRAM define"
#endif

#ifndef TBD_DMA
    #error "system port needs to provide TBD_DMA define"
#endif