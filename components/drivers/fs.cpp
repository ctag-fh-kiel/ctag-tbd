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
#include "miniz.h"

using namespace CTAG::DRIVERS;

#define BUF_SZ (64*1024)

static sdmmc_card_t* card = nullptr;

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

    char* buffer = heap_caps_malloc(BUFSIZ, MALLOC_CAP_SPIRAM);
    assert(buffer);

    while (in.read(buffer, BUF_SZ))
        out.write(buffer, in.gcount());
    out.write(buffer, in.gcount());  // write remaining bytes

    heap_caps_free(buffer);

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

static bool read_hash_file(const std::string& path, std::string& hash) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    std::getline(file, hash);
    // Trim whitespace
    hash.erase(0, hash.find_first_not_of(" \t\r\n"));
    hash.erase(hash.find_last_not_of(" \t\r\n") + 1);
    return !hash.empty();
}

static bool delete_dir_recursive(const std::string& path) {
    ESP_LOGI("FS", "Deleting directory: %s", path.c_str());

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string full_path = path + "/" + name;
        struct stat st{};

        if (stat(full_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                delete_dir_recursive(full_path);
            } else {
                unlink(full_path.c_str());
            }
        }
    }

    closedir(dir);
    rmdir(path.c_str());
    return true;
}

static bool extract_zip_to_sd(const std::string& zip_path, const std::string& dest_dir) {
    ESP_LOGI("FS", "Extracting %s to %s", zip_path.c_str(), dest_dir.c_str());

    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);

    if (!mz_zip_reader_init_file(&zip, zip_path.c_str(), 0)) {
        ESP_LOGE("FS", "Failed to open zip file: %s", zip_path.c_str());
        return false;
    }

    int num_files = (int)mz_zip_reader_get_num_files(&zip);
    ESP_LOGI("FS", "Zip contains %d files", num_files);

    for (int i = 0; i < num_files; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            ESP_LOGE("FS", "Failed to get file stat for index %d", i);
            continue;
        }

        std::string filename = file_stat.m_filename;
        std::string full_path = dest_dir + "/" + filename;

        // Skip if it's a directory entry
        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            ESP_LOGD("FS", "Creating directory: %s", full_path.c_str());
            mkdir(full_path.c_str(), 0755);
            continue;
        }

        // Create parent directories if needed
        size_t pos = full_path.find_last_of('/');
        if (pos != std::string::npos) {
            std::string dir_path = full_path.substr(0, pos);

            // Create directories recursively
            size_t start = dest_dir.length() + 1;
            while ((pos = dir_path.find('/', start)) != std::string::npos) {
                std::string partial_dir = dir_path.substr(0, pos);
                mkdir(partial_dir.c_str(), 0755);
                start = pos + 1;
            }
            mkdir(dir_path.c_str(), 0755);
        }

        // Extract file
        ESP_LOGD("FS", "Extracting: %s", filename.c_str());
        if (!mz_zip_reader_extract_to_file(&zip, i, full_path.c_str(), 0)) {
            ESP_LOGE("FS", "Failed to extract file: %s", filename.c_str());
        }
    }

    mz_zip_reader_end(&zip);
    ESP_LOGI("FS", "Zip extraction completed");
    return true;
}

static void check_and_update_sd_content(const std::string& base_path) {
    const std::string version_file = base_path + "/.version";
    const std::string hash_file = base_path + "/tbd-sd-card-hash.txt";
    const std::string zip_file = base_path + "/tbd-sd-card.zip";

    std::string current_hash;
    std::string stored_hash;

    bool version_exists = read_hash_file(version_file, stored_hash);
    bool hash_exists = read_hash_file(hash_file, current_hash);

    if (!hash_exists) {
        ESP_LOGW("FS", "No tbd-sd-card-hash.txt found, skipping update check");
        return;
    }

    ESP_LOGI("FS", "Current hash from tbd-sd-card-hash.txt: %s", current_hash.c_str());

    if (version_exists) {
        ESP_LOGI("FS", "Stored hash from .version: %s", stored_hash.c_str());

        if (current_hash == stored_hash) {
            ESP_LOGI("FS", "Content is up to date");
            return;
        }

        ESP_LOGI("FS", "New version detected, updating content...");
    } else {
        ESP_LOGI("FS", "No .version file found, performing initial extraction");
    }

    // Check if zip file exists
    struct stat st{};
    if (stat(zip_file.c_str(), &st) != 0) {
        ESP_LOGE("FS", "Zip file not found: %s", zip_file.c_str());
        return;
    }

    // Delete old content directories if they exist
    delete_dir_recursive(base_path + "/data");
    delete_dir_recursive(base_path + "/www");
    delete_dir_recursive(base_path + "/tbdsamples");
    unlink(version_file.c_str()); // Delete old .version file

    // Extract new content
    if (!extract_zip_to_sd(zip_file, base_path)) {
        ESP_LOGE("FS", "Failed to extract zip file");
        return;
    }

    ESP_LOGI("FS", "Content updated successfully");

    // Create backup of data directory to dbup for safety
    std::string data_dir = base_path + "/data";
    std::string backup_dir = base_path + "/dbup";

    // Delete old backup if it exists
    delete_dir_recursive(backup_dir);

    // Create backup
    ESP_LOGI("FS", "Creating backup of /data to /dbup for safety...");
    if (!copy_dir(data_dir, backup_dir)) {
        ESP_LOGW("FS", "Failed to create backup of /data to /dbup");
    } else {
        ESP_LOGI("FS", "Backup created successfully");
    }
}


bool FileSystem::SDMounted() {
    return card != nullptr;
}

void FileSystem::InitFS(){
    // try to mount the SD card first
    auto sd_mounted = MountSDCard();
    assert(sd_mounted);

    // Check and update SD card content from zip if needed
    check_and_update_sd_content("/spiffs");
}