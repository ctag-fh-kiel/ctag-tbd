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

#pragma once

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include "esp_err.h"
#include <cstdint>

namespace CTAG::DRIVERS {

/// SPI master driver implementing the picoboot3 protocol.
/// Uses SPI3_HOST as master (same pins as command API, but before slave init).
class Picoboot3Master {
public:
    /// Initialize SPI3_HOST as master at 10 MHz, Mode 3.
    static esp_err_t Init();

    /// Free SPI master resources.
    static void Deinit();

    /// Send ACTIVATE (0xA5), expect "pbt3" response.
    /// If out_resp is non-null, copies the 4-byte response into it (for diagnostics).
    static esp_err_t Activate(uint8_t *out_resp = nullptr);

    /// Query bootloader version.
    static esp_err_t GetVersion(uint8_t &major, uint8_t &minor, uint8_t &patch);

    /// Query total flash size in bytes.
    static esp_err_t GetFlashSize(uint32_t &size);

    /// Poll ready/busy status. Returns true if ready.
    static esp_err_t IsReady(bool &ready);

    /// Poll until ready or timeout. Returns ESP_ERR_TIMEOUT on timeout.
    static esp_err_t WaitReady(uint32_t timeout_ms);

    /// Erase a 4KB sector by sector number (sector 8 = address 0x8000).
    static esp_err_t EraseSector(uint16_t sector);

    /// Program up to 4KB at the given flash address. Address and len must be 256-byte aligned.
    static esp_err_t ProgramPage(uint32_t addr, const uint8_t *data, uint16_t len);

    /// Read flash contents at the given address.
    static esp_err_t ReadFlash(uint32_t addr, uint8_t *data, uint16_t len);

    /// Jump to application code. Picoboot3 will not return.
    static esp_err_t GoToApp();
};

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
