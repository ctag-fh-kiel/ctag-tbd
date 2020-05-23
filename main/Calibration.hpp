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

#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "CalibrationModel.hpp"

namespace CTAG{
    namespace CAL{
        enum class CVConfig : int {
            CVUnipolar, CVBipolar
        };
        class Calibration{
        public:
            static void Init();
            static void IRAM_ATTR MapCVData(const uint16_t *adcIn, float *mapOut);
            static void ConfigCVChannels(CVConfig ch0, CVConfig ch1, CVConfig ch2, CVConfig ch3);
            static void RequestCalibrationOnReboot();
            static const char *GetCStrJSONCalibration() {
                return model->GetCStrJSONCalibration();
            }
            static void SetJSONCalibration(const string &calData){
                model->SetJSONCalibration(calData);
                /* this is NOT thread safe !!! */
                model->LoadMatrix("aCalCalibration_CV_05V", aCoeffs05V);
                model->LoadMatrix("bCalCalibration_CV_05V", bCoeffs05V);
                model->LoadMatrix("aCalCalibration_CV_10V", aCoeffs10V);
                model->LoadMatrix("bCalCalibration_CV_10V", bCoeffs10V);
                /* NOT THREAD SAFE END */
            }
        private:
            static void doCalibration();
            static void ledTask(void * params);
            static void btnTask(void * params);
            static void acquireData(std::vector<uint32_t> &d);
            static void calcPiecewiseLinearCoeffs(const string &dataID, CVConfig cvType);
            static TaskHandle_t ledTaskHandle, btnTaskHandle;
            static std::atomic_int32_t taskControl;
            static QueueHandle_t evQueue;
            enum class Event : uint32_t {NONE, BTN_PRESS};
            static unique_ptr<CalibrationModel> model;
            static DRAM_ATTR float aCoeffs05V[4*2];
            static DRAM_ATTR float bCoeffs05V[4*2];
            static DRAM_ATTR float aCoeffs10V[4*2];
            static DRAM_ATTR float bCoeffs10V[4*2];
            static DRAM_ATTR CVConfig configCV[4];
        };
    }
}
