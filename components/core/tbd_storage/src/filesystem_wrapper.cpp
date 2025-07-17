#include <tbd/storage/filesystem_wrapper.hpp>

#ifndef TBD_FILE_SYSTEM_USE_WRAPPER
    #error "file system wrapper not available in config"
#endif

#if TBD_FILE_SYSTEM_USE_WRAPPER

namespace tbd::storage::filesystem {

const directory_iterator directory_iterator::_end_token;

}

#endif
