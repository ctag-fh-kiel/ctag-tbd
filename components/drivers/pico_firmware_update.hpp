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

namespace CTAG::DRIVERS {

/// Boot-time check for Pico firmware updates.
/// Compares version files on SD card and flashes via picoboot3 if needed.
class PicoFirmwareUpdate {
public:
    /// Check if a Pico firmware update is pending and flash if needed.
    /// Call from app_main() after InitFS() but before StartSoundProcessor().
    /// Returns true if an update was performed, false if skipped.
    static bool CheckAndFlash();
};

}  // namespace CTAG::DRIVERS

#endif // CONFIG_TBD_USE_RP2350
