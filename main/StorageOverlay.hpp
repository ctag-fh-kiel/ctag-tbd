/***************
TBD-16 — dadamachines Storage Overlay

Storage overlay resolution for factory/user directory structure.

The SD card is organized into zones:
  /sdcard/factory/  — immutable factory defaults (from SD card image)
  /sdcard/user/     — user-created/modified data (the backup target)
  /sdcard/system/   — runtime caches (ephemeral, rebuilt at boot)

Overlay resolution: check /user/ first, then /factory/.
Writes always go to /user/.

(c) 2026 Johannes Elias Lohbihler for dadamachines.
Licensed under the GNU Lesser General Public License (LGPL 3.0).
***************/

#pragma once

#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <vector>
#include "esp_log.h"
#include "ctagResources.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace CTAG {
namespace STORAGE {

static const char *OVERLAY_TAG = "StorageOverlay";

/**
 * Global storage mutex — protects SD card file write operations
 * from concurrent access by SPI task and HTTP handlers.
 * Acquire before any file write/delete/rename sequence.
 */
inline SemaphoreHandle_t &storageMutex() {
    static SemaphoreHandle_t mtx = xSemaphoreCreateMutex();
    return mtx;
}
inline void lockStorage()   { xSemaphoreTake(storageMutex(), portMAX_DELAY); }
inline void unlockStorage() { xSemaphoreGive(storageMutex()); }

// Zone path helpers — all relative to sdcardRoot (/sdcard)
inline std::string factoryPath() { return RESOURCES::sdcardRoot + "/factory"; }
inline std::string userPath()    { return RESOURCES::sdcardRoot + "/user"; }
inline std::string systemPath()  { return RESOURCES::sdcardRoot + "/system"; }

// Subdirectory names (shared between factory/ and user/ zones)
static constexpr const char *DIR_PRESETS       = "presets";      // was macrosoundpresets/
static constexpr const char *DIR_MACROS        = "macros";       // was macrodefinitions/
static constexpr const char *DIR_PLUGINS        = "plugins";       // was patches/, sp/
static constexpr const char *DIR_TRACKDEFAULTS = "trackdefaults";
static constexpr const char *DIR_PROJECTS      = "projects";
static constexpr const char *DIR_KITS          = "kits";
static constexpr const char *DIR_CONFIG        = "config";

/**
 * Check if a file or directory exists.
 */
inline bool fileExists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

/**
 * Check if path is a directory.
 */
inline bool isDirectory(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

/**
 * Resolve a resource file using overlay pattern.
 * Checks /user/{subdir}/{filename} first, then /factory/{subdir}/{filename}.
 * Returns the full resolved path, or empty string if not found in either zone.
 *
 * @param subdir  e.g. "presets", "macros", "plugins"
 * @param filename  e.g. "db-all-def.json"
 * @return Full path to the resolved file, or "" if not found
 */
inline std::string resolveFile(const char *subdir, const std::string &filename) {
    // Check user layer first
    std::string userFile = userPath() + "/" + subdir + "/" + filename;
    if (fileExists(userFile)) {
        return userFile;
    }
    // Fall back to factory layer
    std::string factoryFile = factoryPath() + "/" + subdir + "/" + filename;
    if (fileExists(factoryFile)) {
        return factoryFile;
    }
    return "";
}

/**
 * Resolve a resource directory using overlay pattern.
 * Returns the first existing directory path (user then factory).
 */
inline std::string resolveDir(const char *subdir) {
    std::string userDir = userPath() + "/" + subdir;
    if (isDirectory(userDir)) {
        return userDir;
    }
    std::string factoryDir = factoryPath() + "/" + subdir;
    if (isDirectory(factoryDir)) {
        return factoryDir;
    }
    return "";
}

/**
 * Get the user-writable path for a resource.
 * Always returns the /user/ path (for writes), regardless of whether it exists yet.
 */
inline std::string userFilePath(const char *subdir, const std::string &filename) {
    return userPath() + "/" + subdir + "/" + filename;
}

/**
 * Get the factory (read-only) path for a resource.
 */
inline std::string factoryFilePath(const char *subdir, const std::string &filename) {
    return factoryPath() + "/" + subdir + "/" + filename;
}

/**
 * List files in a subdirectory, merging user and factory layers.
 * User files shadow factory files with the same name.
 * Returns filenames only (no path prefix).
 */
inline std::vector<std::string> listMergedDir(const char *subdir) {
    std::vector<std::string> result;

    // Collect factory files first
    std::string factoryDir = factoryPath() + "/" + subdir;
    DIR *d = opendir(factoryDir.c_str());
    if (d) {
        struct dirent *entry;
        while ((entry = readdir(d)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            result.push_back(entry->d_name);
        }
        closedir(d);
    }

    // Add user files, replacing factory duplicates
    std::string userDir = userPath() + "/" + subdir;
    d = opendir(userDir.c_str());
    if (d) {
        struct dirent *entry;
        while ((entry = readdir(d)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            std::string name = entry->d_name;
            // Remove factory duplicate if present
            for (auto it = result.begin(); it != result.end(); ++it) {
                if (*it == name) {
                    result.erase(it);
                    break;
                }
            }
            result.push_back(name);
        }
        closedir(d);
    }

    return result;
}

/**
 * Check if the SD card has the new overlay layout (/factory/ + /user/).
 */
inline bool hasOverlayLayout() {
    return isDirectory(factoryPath()) && isDirectory(userPath());
}

/**
 * Create the directory structure for the overlay layout.
 */
inline void ensureOverlayDirs() {
    const char *dirs[] = {
        "/factory",
        "/factory/presets",
        "/factory/macros",
        "/factory/plugins",
        "/factory/trackdefaults",
        "/factory/kits",
        "/factory/config",
        "/user",
        "/user/projects",
        "/user/presets",
        "/user/macros",
        "/user/plugins",
        "/user/trackdefaults",
        "/user/kits",
        "/user/config",
        "/system",
        "/system/cache",
        nullptr
    };
    for (int i = 0; dirs[i]; i++) {
        std::string path = RESOURCES::sdcardRoot + dirs[i];
        if (!isDirectory(path)) {
            mkdir(path.c_str(), 0755);
            ESP_LOGI(OVERLAY_TAG, "Created directory: %s", path.c_str());
        }
    }
}

/**
 * Copy a single file from src to dst.
 * Returns true on success.
 */
inline bool copyFile(const std::string &src, const std::string &dst) {
    FILE *fin = fopen(src.c_str(), "rb");
    if (!fin) return false;

    FILE *fout = fopen(dst.c_str(), "wb");
    if (!fout) {
        fclose(fin);
        return false;
    }

    char buf[4096];
    size_t n;
    bool ok = true;
    while ((n = fread(buf, 1, sizeof(buf), fin)) > 0) {
        if (fwrite(buf, 1, n, fout) != n) {
            ok = false;
            break;
        }
    }

    fclose(fin);
    fclose(fout);
    if (!ok) {
        remove(dst.c_str());
    }
    return ok;
}

/**
 * Initialize the storage overlay system.
 * Call once at boot, after SD card is mounted.
 * Ensures all required directories exist.
 */
inline void initOverlay() {
    ensureOverlayDirs();
}

} // namespace STORAGE
} // namespace CTAG
