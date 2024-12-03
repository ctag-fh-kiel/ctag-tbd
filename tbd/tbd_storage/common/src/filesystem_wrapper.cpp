#include <tbd/storage/common/filesystem_wrapper.hpp>

#if !TBD_FILE_SYSTEM_USE_WRAPPER
    #error "file system wrapper not available in config"
#endif

namespace tbd::storage::filesystem {

const directory_iterator directory_iterator::_end_token;

}
