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
#include "esp_heap_caps.h"
#include "zlib.h"

using namespace CTAG::DRIVERS;

#define BUF_SZ (64*1024)

static sdmmc_card_t* card = nullptr;

static bool MountSDCard(const char *mnt_pt = "/sdcard"){
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

/*
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

    char* buffer = (char*)heap_caps_malloc(BUFSIZ, MALLOC_CAP_SPIRAM);
    assert(buffer);

    while (in.read(buffer, BUF_SZ))
        out.write(buffer, in.gcount());
    out.write(buffer, in.gcount());  // write remaining bytes

    heap_caps_free(buffer);

    return true;
}
*/

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

    // Allocate path buffer in SPIRAM
    char* full_path = (char*)heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    if (!full_path) {
        ESP_LOGE("FS", "Failed to allocate path buffer");
        closedir(dir);
        return false;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        const char* name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        snprintf(full_path, 512, "%s/%s", path.c_str(), name);

        struct stat st{};
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                delete_dir_recursive(full_path);
            } else {
                unlink(full_path);
            }
        }
    }

    heap_caps_free(full_path);
    closedir(dir);
    rmdir(path.c_str());
    return true;
}

static bool extract_zip_to_sd(const std::string& zip_path, const std::string& dest_dir) {
    ESP_LOGI("FS", "Extracting %s to %s", zip_path.c_str(), dest_dir.c_str());

    FILE* zip_file = fopen(zip_path.c_str(), "rb");
    if (!zip_file) {
        ESP_LOGE("FS", "Failed to open zip file");
        return false;
    }

    // Find End of Central Directory (EOCD) signature
    // Search from end of file backwards (typically last 22 bytes + comment)
    fseek(zip_file, 0, SEEK_END);
    long file_size = ftell(zip_file);

    // Search last 64KB for EOCD
    long search_start = (file_size > 65536) ? (file_size - 65536) : 0;
    fseek(zip_file, search_start, SEEK_SET);

    uint8_t* search_buf = (uint8_t*)heap_caps_malloc(file_size - search_start, MALLOC_CAP_SPIRAM);
    if (!search_buf) {
        ESP_LOGE("FS", "Failed to allocate search buffer");
        fclose(zip_file);
        return false;
    }

    fread(search_buf, 1, file_size - search_start, zip_file);

    // Find EOCD signature (0x06054b50)
    long eocd_offset = -1;
    for (long i = file_size - search_start - 22; i >= 0; i--) {
        if (search_buf[i] == 0x50 && search_buf[i+1] == 0x4b &&
            search_buf[i+2] == 0x05 && search_buf[i+3] == 0x06) {
            eocd_offset = search_start + i;
            break;
        }
    }

    if (eocd_offset < 0) {
        ESP_LOGE("FS", "No EOCD found - not a valid ZIP file");
        heap_caps_free(search_buf);
        fclose(zip_file);
        return false;
    }

    // Parse EOCD to get central directory location
    fseek(zip_file, eocd_offset + 16, SEEK_SET);
    uint32_t cd_offset, cd_size;
    uint16_t num_entries;
    fread(&num_entries, 2, 1, zip_file);
    fseek(zip_file, eocd_offset + 10, SEEK_SET);
    fread(&num_entries, 2, 1, zip_file); // Total entries
    fseek(zip_file, eocd_offset + 12, SEEK_SET);
    fread(&cd_size, 4, 1, zip_file);
    fread(&cd_offset, 4, 1, zip_file);

    heap_caps_free(search_buf);

    ESP_LOGI("FS", "ZIP: %d files, central dir at offset %u", num_entries, cd_offset);

    // Read central directory
    fseek(zip_file, cd_offset, SEEK_SET);
    uint8_t* cd_buf = (uint8_t*)heap_caps_malloc(cd_size, MALLOC_CAP_SPIRAM);
    if (!cd_buf) {
        ESP_LOGE("FS", "Failed to allocate central directory buffer");
        fclose(zip_file);
        return false;
    }
    fread(cd_buf, 1, cd_size, zip_file);

    // Allocate decompression buffers
    const size_t chunk_size = 16384; // 16KB chunks
    uint8_t* in_buf = (uint8_t*)heap_caps_malloc(chunk_size, MALLOC_CAP_SPIRAM);
    uint8_t* out_buf = (uint8_t*)heap_caps_malloc(chunk_size, MALLOC_CAP_SPIRAM);

    if (!in_buf || !out_buf) {
        ESP_LOGE("FS", "Failed to allocate decompression buffers");
        heap_caps_free(cd_buf);
        heap_caps_free(in_buf);
        heap_caps_free(out_buf);
        fclose(zip_file);
        return false;
    }

    // Process each file from central directory
    uint32_t cd_pos = 0;
    int files_extracted = 0;
    int files_processed = 0;
    uint32_t last_log_time = esp_log_timestamp();

    while (cd_pos < cd_size) {
        // Check central directory signature
        if (cd_buf[cd_pos] != 0x50 || cd_buf[cd_pos+1] != 0x4b ||
            cd_buf[cd_pos+2] != 0x01 || cd_buf[cd_pos+3] != 0x02) {
            break; // End of central directory entries
        }

        uint16_t comp_method = *(uint16_t*)(cd_buf + cd_pos + 10);
        uint32_t comp_size = *(uint32_t*)(cd_buf + cd_pos + 20);
        uint32_t uncomp_size = *(uint32_t*)(cd_buf + cd_pos + 24);
        uint16_t name_len = *(uint16_t*)(cd_buf + cd_pos + 28);
        uint16_t extra_len = *(uint16_t*)(cd_buf + cd_pos + 30);
        uint16_t comment_len = *(uint16_t*)(cd_buf + cd_pos + 32);
        uint32_t local_header_offset = *(uint32_t*)(cd_buf + cd_pos + 42);

        std::string filename((char*)(cd_buf + cd_pos + 46), name_len);
        std::string full_path = dest_dir + "/" + filename;

        files_processed++;

        // Log progress every 10 files or every 2 seconds
        uint32_t now = esp_log_timestamp();
        if ((files_processed % 10 == 0) || (now - last_log_time > 2000)) {
            ESP_LOGI("FS", "Progress: %d/%d files (%.1f%%) - Current: %s",
                     files_processed, num_entries,
                     (files_processed * 100.0f) / num_entries,
                     filename.c_str());
            last_log_time = now;
        }

        ESP_LOGD("FS", "File: %s (%u bytes, method=%d)", filename.c_str(), uncomp_size, comp_method);

        // Handle directories
        if (filename.back() == '/') {
            ESP_LOGD("FS", "Creating directory: %s", full_path.c_str());
            mkdir(full_path.c_str(), 0755);
        } else {
            // Create parent directories
            size_t pos = full_path.find_last_of('/');
            if (pos != std::string::npos) {
                std::string dir_path = full_path.substr(0, pos);
                size_t start = dest_dir.length() + 1;
                while ((pos = dir_path.find('/', start)) != std::string::npos) {
                    mkdir(dir_path.substr(0, pos).c_str(), 0755);
                    start = pos + 1;
                }
                mkdir(dir_path.c_str(), 0755);
            }

            // Read local file header to get actual data offset
            fseek(zip_file, local_header_offset, SEEK_SET);
            uint8_t local_header[30];
            fread(local_header, 1, 30, zip_file);

            uint16_t local_name_len = *(uint16_t*)(local_header + 26);
            uint16_t local_extra_len = *(uint16_t*)(local_header + 28);
            uint32_t data_offset = local_header_offset + 30 + local_name_len + local_extra_len;

            fseek(zip_file, data_offset, SEEK_SET);

            // Open output file
            FILE* out_file = fopen(full_path.c_str(), "wb");
            if (!out_file) {
                ESP_LOGE("FS", "Failed to create: %s", full_path.c_str());
                cd_pos += 46 + name_len + extra_len + comment_len;
                continue;
            }

            if (comp_method == 0) {
                // Stored (no compression)
                uint32_t remaining = uncomp_size;
                while (remaining > 0) {
                    uint32_t to_read = (remaining < chunk_size) ? remaining : chunk_size;
                    fread(in_buf, 1, to_read, zip_file);
                    fwrite(in_buf, 1, to_read, out_file);
                    remaining -= to_read;
                }
            } else if (comp_method == 8) {
                // Deflate compression - use zlib
                z_stream stream;
                memset(&stream, 0, sizeof(stream));

                // Use raw deflate (negative windowBits)
                if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
                    ESP_LOGE("FS", "inflateInit failed");
                    fclose(out_file);
                    cd_pos += 46 + name_len + extra_len + comment_len;
                    continue;
                }

                uint32_t total_in = 0;
                int ret;

                do {
                    // Read compressed data
                    uint32_t to_read = ((comp_size - total_in) < chunk_size) ?
                                      (comp_size - total_in) : chunk_size;
                    stream.avail_in = fread(in_buf, 1, to_read, zip_file);
                    total_in += stream.avail_in;

                    if (stream.avail_in == 0) break;

                    stream.next_in = in_buf;

                    // Decompress
                    do {
                        stream.avail_out = chunk_size;
                        stream.next_out = out_buf;

                        ret = inflate(&stream, Z_NO_FLUSH);

                        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                            ESP_LOGE("FS", "inflate error: %d", ret);
                            break;
                        }

                        uint32_t have = chunk_size - stream.avail_out;
                        fwrite(out_buf, 1, have, out_file);

                    } while (stream.avail_out == 0);

                } while (ret != Z_STREAM_END && total_in < comp_size);

                inflateEnd(&stream);
            }

            fclose(out_file);
            files_extracted++;
        }

        cd_pos += 46 + name_len + extra_len + comment_len;
    }

    heap_caps_free(cd_buf);
    heap_caps_free(in_buf);
    heap_caps_free(out_buf);
    fclose(zip_file);

    ESP_LOGI("FS", "Extraction complete: %d/%d files extracted successfully (%.1f%%)",
             files_extracted, files_processed,
             (files_extracted * 100.0f) / files_processed);
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

    // Backup is pre-created in the zip archive at /dbup
    ESP_LOGI("FS", "Note: Backup of /data is included in archive as /dbup");

    // Create .version file to mark successful extraction
    std::ofstream version_out(version_file);
    if (version_out) {
        version_out << current_hash;
        version_out.close();
        ESP_LOGI("FS", "Created .version file with hash: %s", current_hash.c_str());
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
    check_and_update_sd_content("/sdcard");
}