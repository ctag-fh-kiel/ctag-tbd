/***************
TBD-16 — Pico Firmware Update System

(c) 2024-2026 Johannes Elias Lohbihler for dadamachines

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include "pico_firmware_update.hpp"
#include "pico_reset.hpp"
#include "picoboot3_master.hpp"
#include "led_rgb_bba.hpp"
#include <string>
#include "version.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sys/stat.h>

static const char *TAG = "PicoFwUpdate";

// SD card paths for firmware files
#define FW_BIN_PATH          "/sdcard/system/firmware/pico-firmware.bin"
#define FW_VERSION_PATH      "/sdcard/system/firmware/pico-firmware-version.txt"
#define FW_INSTALLED_PATH    "/sdcard/system/firmware/pico-installed-version.txt"
#define FW_DIR_PATH          "/sdcard/system/firmware"

#define PICOBOOT3_APP_OFFSET  0x8000  // 32KB bootloader reserved
#define FLASH_SECTOR_SIZE     4096
#define FLASH_PAGE_SIZE       256
#define ACTIVATE_RETRIES      3
#define ERASE_TIMEOUT_MS      5000
#define PROGRAM_TIMEOUT_MS    1000

/// Read a small text file into a buffer. Returns length or -1 on error.
static int read_text_file(const char *path, char *buf, size_t buf_size) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t n = fread(buf, 1, buf_size - 1, f);
    fclose(f);
    buf[n] = '\0';
    // Trim trailing whitespace
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r' || buf[n-1] == ' ')) {
        buf[--n] = '\0';
    }
    return (int)n;
}

/// Write a string to a text file.
static bool write_text_file(const char *path, const char *text) {
    FILE *f = fopen(path, "w");
    if (!f) return false;
    fputs(text, f);
    fclose(f);
    return true;
}

#define FW_LOG_PATH "/sdcard/system/firmware/pico-update-log.txt"

/// Append a line to the update log file on SD card.
static void log_to_sd(const char *fmt, ...) {
    FILE *f = fopen(FW_LOG_PATH, "a");
    if (!f) return;
    // Timestamp from FreeRTOS tick
    fprintf(f, "[%lu] ", (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS));
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
    fclose(f);
}

namespace CTAG::DRIVERS {

bool PicoFirmwareUpdate::ConsumePicoUpdateFlag() {
    struct stat st;
    if (stat(PICO_UPDATE_FLAG_PATH, &st) == 0) {
        remove(PICO_UPDATE_FLAG_PATH);
        return true;
    }
    return false;
}

bool PicoFirmwareUpdate::ConsumeP4UpdateFlag() {
    struct stat st;
    if (stat(P4_UPDATE_FLAG_PATH, &st) == 0) {
        remove(P4_UPDATE_FLAG_PATH);
        return true;
    }
    return false;
}

void PicoFirmwareUpdate::CheckP4VersionChange() {
    // Read previously stored P4 firmware version
    char installed[64] = {0};
    read_text_file(P4_INSTALLED_PATH, installed, sizeof(installed));

    const char *current = TBD_FW_VERSION.c_str();

    if (strcmp(installed, current) == 0) {
        ESP_LOGI(TAG, "P4 version unchanged: %s", installed);
        return;
    }

    ESP_LOGI(TAG, "P4 version changed: '%s' -> '%s'", installed, current);

    // Write flag file (consumed once by 0xAE handler after Pico reads it)
    write_text_file(P4_UPDATE_FLAG_PATH, "1");

    // Update installed version for next boot comparison
    write_text_file(P4_INSTALLED_PATH, current);
}

bool PicoFirmwareUpdate::CheckAndFlash() {
    // Clear previous log
    remove(FW_LOG_PATH);
    log_to_sd("=== Pico firmware update check ===");

    // 1. Check if firmware binary exists
    struct stat st;
    if (stat(FW_BIN_PATH, &st) != 0 || st.st_size == 0) {
        ESP_LOGI(TAG, "No firmware file at %s — skipping", FW_BIN_PATH);
        log_to_sd("SKIP: no firmware file at %s", FW_BIN_PATH);
        return false;
    }
    uint32_t fw_size = st.st_size;

    // 2. Read target version
    char target_version[64] = {0};
    if (read_text_file(FW_VERSION_PATH, target_version, sizeof(target_version)) <= 0) {
        ESP_LOGI(TAG, "No version file at %s — skipping", FW_VERSION_PATH);
        log_to_sd("SKIP: no version file at %s", FW_VERSION_PATH);
        return false;
    }

    // 3. Read installed version (may not exist)
    char installed_version[64] = {0};
    read_text_file(FW_INSTALLED_PATH, installed_version, sizeof(installed_version));

    // 4. Compare versions
    if (strcmp(target_version, installed_version) == 0) {
        ESP_LOGI(TAG, "Version %s already installed — skipping", target_version);
        log_to_sd("SKIP: version %s already installed", target_version);
        return false;
    }

    log_to_sd("UPDATE: %s -> %s (%lu bytes)",
             installed_version[0] ? installed_version : "(none)",
             target_version, fw_size);
    ESP_LOGI(TAG, "Firmware update: %s → %s (%lu bytes)",
             installed_version[0] ? installed_version : "(none)",
             target_version, fw_size);

    // 5. LED: orange = update in progress
    LedRGB::SetLedRGB(255, 100, 0);

    // 6. Initialize GPIO and enter picoboot3 mode
    PicoReset::Init();
    PicoReset::EnterPicoboot3Mode();

    // 7. Initialize SPI master
    esp_err_t ret = Picoboot3Master::Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI master init failed: %s", esp_err_to_name(ret));
        log_to_sd("FAIL: SPI master init: %s", esp_err_to_name(ret));
        goto fail;
    }
    log_to_sd("SPI master initialized");

    // 8. Activate picoboot3 with retries
    for (int attempt = 0; attempt < ACTIVATE_RETRIES; attempt++) {
        uint8_t activate_resp[4] = {0};
        ret = Picoboot3Master::Activate(activate_resp);
        log_to_sd("activate attempt %d: ret=%d resp=%02x %02x %02x %02x",
                 attempt + 1, ret,
                 activate_resp[0], activate_resp[1],
                 activate_resp[2], activate_resp[3]);
        if (ret == ESP_OK) break;
        ESP_LOGW(TAG, "Activate attempt %d failed, retrying...", attempt + 1);
        // Re-enter picoboot3 mode and wait longer
        Picoboot3Master::Deinit();
        PicoReset::EnterPicoboot3Mode();
        vTaskDelay(pdMS_TO_TICKS(200));
        Picoboot3Master::Init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to activate picoboot3 after %d attempts", ACTIVATE_RETRIES);
        log_to_sd("FAIL: activate picoboot3 after %d attempts", ACTIVATE_RETRIES);
        goto fail_deinit;
    }
    log_to_sd("picoboot3 activated OK");

    // 9. Verify flash size
    {
        uint32_t flash_size = 0;
        ret = Picoboot3Master::GetFlashSize(flash_size);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "GetFlashSize failed");
            goto fail_deinit;
        }
        if (fw_size + PICOBOOT3_APP_OFFSET > flash_size) {
            ESP_LOGE(TAG, "Firmware too large: %lu + %d > %lu", fw_size, PICOBOOT3_APP_OFFSET, flash_size);
            goto fail_deinit;
        }

        uint8_t major, minor, patch;
        Picoboot3Master::GetVersion(major, minor, patch);
    }

    // 10. Open firmware file and flash
    {
        FILE *fw_file = fopen(FW_BIN_PATH, "rb");
        if (!fw_file) {
            ESP_LOGE(TAG, "Cannot open %s", FW_BIN_PATH);
            goto fail_deinit;
        }

        // Calculate sectors needed (starting from sector 8 = offset 0x8000)
        uint16_t first_sector = PICOBOOT3_APP_OFFSET / FLASH_SECTOR_SIZE;
        uint16_t num_sectors = (fw_size + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;
        uint8_t page_buf[FLASH_PAGE_SIZE];

        ESP_LOGI(TAG, "Flashing %lu bytes: sectors %d–%d", fw_size, first_sector, first_sector + num_sectors - 1);
        log_to_sd("Flashing %lu bytes: sectors %d-%d", fw_size, first_sector, first_sector + num_sectors - 1);

        for (uint16_t s = 0; s < num_sectors; s++) {
            uint16_t sector = first_sector + s;

            // Erase sector
            ret = Picoboot3Master::WaitReady(ERASE_TIMEOUT_MS);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "WaitReady before erase sector %d failed", sector);
                fclose(fw_file);
                goto fail_deinit;
            }

            ret = Picoboot3Master::EraseSector(sector);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "EraseSector %d failed", sector);
                fclose(fw_file);
                goto fail_deinit;
            }

            // Program pages within this sector
            uint32_t sector_base_addr = (uint32_t)sector * FLASH_SECTOR_SIZE;
            uint16_t pages_per_sector = FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE;

            for (uint16_t p = 0; p < pages_per_sector; p++) {
                size_t n = fread(page_buf, 1, FLASH_PAGE_SIZE, fw_file);
                if (n == 0) break;  // EOF

                // Pad to page boundary if partial
                if (n < FLASH_PAGE_SIZE) {
                    memset(page_buf + n, 0xFF, FLASH_PAGE_SIZE - n);
                }

                uint32_t page_addr = sector_base_addr + p * FLASH_PAGE_SIZE;

                ret = Picoboot3Master::WaitReady(PROGRAM_TIMEOUT_MS);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "WaitReady before program 0x%08lX failed", page_addr);
                    fclose(fw_file);
                    goto fail_deinit;
                }

                ret = Picoboot3Master::ProgramPage(page_addr, page_buf, FLASH_PAGE_SIZE);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "ProgramPage 0x%08lX failed", page_addr);
                    fclose(fw_file);
                    goto fail_deinit;
                }

                if (feof(fw_file)) break;
            }

            // Progress log every 10 sectors (~40KB)
            if (s % 10 == 0 || s == num_sectors - 1) {
                ESP_LOGI(TAG, "Progress: %d/%d sectors", s + 1, num_sectors);
                log_to_sd("Progress: %d/%d sectors", s + 1, num_sectors);
            }
        }

        fclose(fw_file);
    }

    // 11. Wait for last operation to complete
    ret = Picoboot3Master::WaitReady(ERASE_TIMEOUT_MS);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Final WaitReady timed out — continuing anyway");
    }

    // 12. Send GO_TO_APP — picoboot3 jumps to application
    Picoboot3Master::GoToApp();

    // 13. Deinit SPI master (before command API slave re-initializes it)
    Picoboot3Master::Deinit();

    // 14. Release BOOTSEL3 for next boot
    PicoReset::Bootsel3Release();

    // Wait for Pico app to boot
    vTaskDelay(pdMS_TO_TICKS(200));

    // 15. Record installed version
    write_text_file(FW_INSTALLED_PATH, target_version);

    ESP_LOGI(TAG, "Firmware update complete: %s", target_version);
    log_to_sd("SUCCESS: firmware update complete, version %s", target_version);

    // LED: green flash for success
    LedRGB::SetLedRGB(0, 255, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    LedRGB::SetLedRGB(0, 0, 255);  // back to blue (normal boot)

    // Write flag file so the Pico can query update status after reboot
    write_text_file(PICO_UPDATE_FLAG_PATH, "1");
    return true;

fail_deinit:
    Picoboot3Master::Deinit();
fail:
    // Try to reboot Pico into normal app mode
    PicoReset::Bootsel3Release();
    PicoReset::RebootIntoApp();

    ESP_LOGE(TAG, "Firmware update FAILED — continuing with normal boot");
    log_to_sd("FAIL: firmware update failed, continuing normal boot");

    // LED: red flash for error, then back to blue
    LedRGB::SetLedRGB(255, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    LedRGB::SetLedRGB(0, 0, 255);

    return false;
}

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
