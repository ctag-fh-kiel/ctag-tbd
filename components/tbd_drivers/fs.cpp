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


#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_littlefs.h"
#include "fs.hpp"

using namespace CTAG::DRIVERS;

void FileSystem::InitFS() {
    ESP_LOGI("fs", "Initializing LITTLEFS");

    esp_vfs_littlefs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = "storage",
            .partition = NULL,
            .format_if_mount_failed = true,
            .read_only = false,
            .dont_mount = false,
            .grow_on_mount = 1
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("fs", "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("fs", "Failed to find LITTLEFS partition");
        } else {
            ESP_LOGE("fs", "Failed to initialize LITTLEFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info("storage", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE("fs", "Failed to get LITTLEFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI("fs", "Partition size: total: %d, used: %d", total, used);
    }
}