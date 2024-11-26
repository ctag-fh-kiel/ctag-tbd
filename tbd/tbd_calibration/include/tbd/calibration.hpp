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
#pragma once

#include <tbd/calibration/calibration_model.hpp>

#include <tbd/ram.hpp>

namespace tbd::calibration {

enum class CVConfig : int {
    CVUnipolar, CVBipolar
};

}

namespace tbd {

struct Calibration final {

    Calibration() = delete;
    static void Init();

    static TBD_IRAM void MapCVData(const uint16_t *adcIn, float *mapOut);

    static void ConfigCVChannels(
        calibration::CVConfig ch0,
        calibration::CVConfig ch1,
        calibration::CVConfig ch2,
        calibration::CVConfig ch3);

    static void RequestCalibrationOnReboot();

    static const char *GetCStrJSONCalibration();
    static void SetJSONCalibration(const std::string &calData);
private:
    static void doCalibration();

    static void ledTask(void *params);

    static void btnTask(void *params);

    static void acquireData(std::vector<uint32_t> &d);

    static void calcPiecewiseLinearCoeffs(
        const std::string &dataID,
        calibration::CVConfig cvType);
};

}

