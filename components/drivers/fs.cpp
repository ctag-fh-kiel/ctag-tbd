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
#include "fs.hpp"

#include <sd_pwr_ctrl_interface.h>
#include <ctime>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <fcntl.h>

#include "sd_pwr_ctrl_by_on_chip_ldo.h"

using namespace CTAG::DRIVERS;

static sdmmc_card_t* card = nullptr;
#define BUF_SZ (64*1024)
static char* buffer = nullptr;

static bool MountSDCard(const char *mnt_pt = "/spiffs"){
    // configure gpio 45 as output and toggle to reset the sd card
    gpio_config_t io_conf{};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_45);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    // toggle power of SD card to reset
    gpio_set_level(GPIO_NUM_45, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_45, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = SDMMC_FREQ_DDR50;
    host.flags |= SDMMC_HOST_FLAG_DDR;
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };

    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
    esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK){
        ESP_LOGE("FS", "Failed to create a new on-chip LDO power control driver");
        return false;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.d4 = GPIO_NUM_NC;
    slot_config.d5 = GPIO_NUM_NC;
    slot_config.d6 = GPIO_NUM_NC;
    slot_config.d7 = GPIO_NUM_NC;
    slot_config.width = 4;
    slot_config.flags |= SDMMC_SLOT_FLAG_UHS1;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    mount_config.format_if_mount_failed = true;

    ESP_LOGI("FS", "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mnt_pt, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK){
        ESP_LOGE("FS", "Failed to mount sd-card");
        return false;
    }
    ESP_LOGI("FS", "sd-card mounted");
    sdmmc_card_print_info(stdout, card);
    return true;
}


static bool copy_file(const std::string& src, const std::string& dst) {
    ESP_LOGI("FS", "Copying file %s to %s", src.c_str(), dst.c_str());
    std::ifstream in(src, std::ios::binary);
    if (!in) {
        ESP_LOGE("FS", "Failed to open source file: %s", src.c_str());
        return false;
    }

    std::ofstream out(dst, std::ios::binary);
    if (!out) {
        ESP_LOGE("FS", "Failed to open destination file: %s", src.c_str());
        return false;
    }


    while (in.read(buffer, BUF_SZ))
        out.write(buffer, in.gcount());
    out.write(buffer, in.gcount());  // write remaining bytes

    return true;
}

static bool copy_dir(const std::string& src, const std::string& dst) {
    ESP_LOGI("FS", "Copying dir %s to %s", src.c_str(), dst.c_str());
    struct stat st{};
    if (stat(dst.c_str(), &st) != 0) {
        if (mkdir(dst.c_str(), 0755) != 0) {
            ESP_LOGE("FS", "Failed to create directory %s", dst.c_str());
            return false;
        }
    }

    DIR* dir = opendir(src.c_str());
    if (!dir) {
        ESP_LOGE("FS", "Failed to open directory %s", src.c_str());
        return false;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string src_path = src + "/" + name;
        std::string dst_path = dst + "/" + name;

        if (stat(src_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (!copy_dir(src_path, dst_path)) {
                    closedir(dir);
                    return false;
                }
            } else if (S_ISREG(st.st_mode)) {
                if (!copy_file(src_path, dst_path)) {
                    closedir(dir);
                    return false;
                }
            }
        }
    }

    closedir(dir);
    return true;
}


bool FileSystem::SDMounted() {
    return card != nullptr;
}

void FileSystem::InitFS(){
    // try to mount the SD card first
    auto sd_mounted = MountSDCard();
    assert(sd_mounted);
}