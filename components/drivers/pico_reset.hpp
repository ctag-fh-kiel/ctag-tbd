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

namespace CTAG::DRIVERS {

/// GPIO4 (RP2350 RESET) + GPIO53 (RP2350 BOOTSEL3/GP23) control.
class PicoReset {
public:
    /// Configure GPIO4 and GPIO53 as outputs.
    static void Init();

    /// Assert RESET (GPIO4 LOW).
    static void ResetAssert();

    /// Release RESET (GPIO4 HIGH).
    static void ResetRelease();

    /// Assert BOOTSEL3 (GPIO53 LOW) — tells picoboot3 to stay in bootloader.
    static void Bootsel3Assert();

    /// Release BOOTSEL3 (GPIO53 HIGH / float) — picoboot3 jumps to app.
    static void Bootsel3Release();

    /// Combined sequence: enter picoboot3 mode.
    /// BOOTSEL3 LOW → RESET LOW 10ms → RESET HIGH → wait 50ms → BOOTSEL3 stays LOW.
    /// Call Bootsel3Release() + ResetAssert/Release after flashing to reboot into app.
    static void EnterPicoboot3Mode();

    /// Reboot Pico into normal app mode (release BOOTSEL3, pulse RESET).
    static void RebootIntoApp();
};

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
