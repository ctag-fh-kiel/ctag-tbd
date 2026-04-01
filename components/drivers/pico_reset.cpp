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

#include "pico_reset.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PicoReset";

#define PICO_RESET_GPIO   GPIO_NUM_4    // RP2350 RESET line (active LOW)
#define PICO_BOOTSEL3_GPIO GPIO_NUM_53  // RP2350 GP23 / picoboot3 BOOTSEL3

namespace CTAG::DRIVERS {

void PicoReset::Init() {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = BIT64(PICO_RESET_GPIO) | BIT64(PICO_BOOTSEL3_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Default state: RESET released (HIGH), BOOTSEL3 released (HIGH = run app)
    gpio_set_level(PICO_RESET_GPIO, 1);
    gpio_set_level(PICO_BOOTSEL3_GPIO, 1);
    ESP_LOGI(TAG, "Init: GPIO%d (RESET), GPIO%d (BOOTSEL3)", PICO_RESET_GPIO, PICO_BOOTSEL3_GPIO);
}

void PicoReset::ResetAssert() {
    gpio_set_level(PICO_RESET_GPIO, 0);
}

void PicoReset::ResetRelease() {
    gpio_set_level(PICO_RESET_GPIO, 1);
}

void PicoReset::Bootsel3Assert() {
    gpio_set_level(PICO_BOOTSEL3_GPIO, 0);
}

void PicoReset::Bootsel3Release() {
    gpio_set_level(PICO_BOOTSEL3_GPIO, 1);
}

void PicoReset::EnterPicoboot3Mode() {
    ESP_LOGI(TAG, "Entering picoboot3 mode...");
    // 1. Assert BOOTSEL3 (LOW) — picoboot3 will see this on boot
    Bootsel3Assert();
    vTaskDelay(pdMS_TO_TICKS(5));

    // 2. Pulse RESET: assert LOW for 10ms, then release
    ResetAssert();
    vTaskDelay(pdMS_TO_TICKS(10));
    ResetRelease();

    // 3. Wait for picoboot3 to initialize (reads BOOTSEL3 after 5ms debounce + SPI init)
    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_LOGI(TAG, "Pico should now be in picoboot3 bootloader mode");
}

void PicoReset::RebootIntoApp() {
    ESP_LOGI(TAG, "Rebooting Pico into app mode...");
    // Release BOOTSEL3 (HIGH) so picoboot3 jumps to app
    Bootsel3Release();
    vTaskDelay(pdMS_TO_TICKS(5));

    // Pulse RESET
    ResetAssert();
    vTaskDelay(pdMS_TO_TICKS(10));
    ResetRelease();

    // Wait for app to boot
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "Pico should now be running application");
}

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
