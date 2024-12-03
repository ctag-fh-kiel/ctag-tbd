#pragma once

#if TBD_FILE_SYSTEM_USE_WRAPPER
    // support for a subset of std::filesystem interfaces wrapping posix fs API
    #include <tbd/storage/common/filesystem_wrapper.hpp>
#else

#include <filesystem>

namespace tbd::storage {

namespace filesystem = std::filesystem;

}

#endif
