#pragma once

#include <tbd/system/common/header_utils.hpp>


TBD_C_BEGIN

#ifndef TBD_LOG_LEVEL
#define TBD_LOG_LEVEL TBD_LOG_INFO
#endif

typedef enum {
    TBD_LOG_SILENT   = 0,   
    TBD_LOG_ERROR    = 1,   
    TBD_LOG_WARN     = 2,   
    TBD_LOG_INFO     = 3,  
    TBD_LOG_DEBUG    = 4,   
    TBD_LOG_VERBOSE  = 5,    
    TBD_LOG_ALL      = 6,   
} tbd_log_levels_t;

TBD_C_END