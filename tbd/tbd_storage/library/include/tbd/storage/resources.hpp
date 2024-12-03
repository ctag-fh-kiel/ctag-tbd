#pragma once

#include <tbd/storage/common/resources.hpp>
#include <tbd/storage/fileystem.hpp>


namespace tbd::storage {

inline std::optional<tbd::storage::filesystem::path> get_fs_path(const std::string& path) {
    namespace fs = tbd::storage::filesystem;

    auto root_path = fs::path(fs_root);
    // if (!fs::is_directory(root_path)) {
    //     TBD_LOGE("storage", "file system root '%s' does not exist", root_path.c_str());
    //     return {};
    // }

    auto full_path = root_path / fs::path(path).relative_path();
    if (!fs::exists(full_path)) {
        TBD_LOGE("storage", "path %s does not exist", full_path.c_str());
        return {};
    }
    return {full_path};
}

}
