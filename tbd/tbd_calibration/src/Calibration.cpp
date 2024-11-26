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
#include <memory>
#include <atomic>
#include <tbd/calibration.hpp>
#include <tbd/ram.hpp>
#include <tbd/logging.hpp>
#include <tbd/system/tasks.hpp>
#include <tbd/system/queues.hpp>
#include <tbd/drivers/adc.hpp>
#include <tbd/drivers/indicator.hpp>
#include <tbd/drivers/gpio.hpp>
#include <tbd/calibration/calibration_model.hpp>



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"

namespace {

enum class Event : uint32_t {
    NONE, BTN_PRESS
};

TaskHandle_t ledTaskHandle;
TaskHandle_t btnTaskHandle;
std::atomic<int32_t> taskControl;

using EventQueue = tbd::system::Queue<Event>;
std::unique_ptr<EventQueue> event_queue;

tbd::calibration::CalibrationModel  model;
TBD_DRAM float aCoeffs05V[4 * 2];
TBD_DRAM float bCoeffs05V[4 * 2];
TBD_DRAM float aCoeffs10V[4 * 2];
TBD_DRAM float bCoeffs10V[4 * 2];
TBD_DRAM tbd::calibration::CVConfig configCV[4];

}

namespace tbd {
void Calibration::Init() {
    ConfigCVChannels(calibration::CVConfig::CVUnipolar, calibration::CVConfig::CVUnipolar, calibration::CVConfig::CVUnipolar, calibration::CVConfig::CVUnipolar);
    drivers::ADC::SetCVINUnipolar(0);
    drivers::ADC::SetCVINUnipolar(1);
    TBD_LOGI("CAL", "Check calibration request");
    if (model.GetCalibrateOnReboot()) {
        TBD_LOGI("CAL", "Starting calibration");
        drivers::Indicator::SetLedRGB(0, 0, 0);
        doCalibration();
        TBD_LOGI("CAL", "Calibration completed");
        drivers::Indicator::SetLedRGB(0, 0, 0);
        model.SetCalibrateOnReboot(false);
    }
    model.LoadMatrix("aCalCalibration_CV_05V", aCoeffs05V);
    model.LoadMatrix("bCalCalibration_CV_05V", bCoeffs05V);
    model.LoadMatrix("aCalCalibration_CV_10V", aCoeffs10V);
    model.LoadMatrix("bCalCalibration_CV_10V", bCoeffs10V);
}

void Calibration::doCalibration() {
    // start tasks for ui
    taskControl.store(1);
    event_queue = std::make_unique<EventQueue>();
    xTaskCreate(&Calibration::ledTask, "led_task", 4096, nullptr, 5, &ledTaskHandle);
    xTaskCreate(&Calibration::btnTask, "btn_task", 4096, nullptr, 5, &btnTaskHandle);

    std::vector<uint32_t> data;
    model.CreateMatrix();
    drivers::ADC::SetCVINUnipolar(0);
    drivers::ADC::SetCVINUnipolar(1);
    // get mins
    TBD_LOGW("CAL", "Adjust CV INs to min value (clock left resp. 0V CV in)");
    acquireData(data);
    model.PushRow(data);
    // get maxs
    taskControl.store(2);
    TBD_LOGW("CAL", "Adjust CV INs to middle / +2.5V");
    acquireData(data);
    model.PushRow(data);
    taskControl.store(3);
    // get mid CV in
    TBD_LOGW("CAL", "Adjust CV INs to max value (clock right resp. +5V CV in)");
    acquireData(data);
    model.PushRow(data);
    model.StoreMatrix("Calibration_CV_05V");
    taskControl.store(4);
    drivers::ADC::SetCVINBipolar(0);
    drivers::ADC::SetCVINBipolar(1);
    model.CreateMatrix();
    // get mins
    TBD_LOGW("CAL", "Adjust CV INs to min value (clock left resp. -5V CV in)");
    acquireData(data);
    model.PushRow(data);
    // get maxs
    taskControl.store(5);
    TBD_LOGW("CAL", "Adjust CV INs to 0V CV in");
    acquireData(data);
    model.PushRow(data);
    taskControl.store(6);
    // get mid CV in
    TBD_LOGW("CAL", "Adjust CV INs to max value (clock right resp. +5V CV in)");
    acquireData(data);
    model.PushRow(data);
    model.StoreMatrix("Calibration_CV_10V");
    model.PrintSelf();

    drivers::ADC::SetCVINUnipolar(0);
    drivers::ADC::SetCVINUnipolar(1);

    calcPiecewiseLinearCoeffs("Calibration_CV_05V", calibration::CVConfig::CVUnipolar);
    calcPiecewiseLinearCoeffs("Calibration_CV_10V", calibration::CVConfig::CVBipolar);

    taskControl.store(0);
    vTaskDelete(ledTaskHandle);
    vTaskDelete(btnTaskHandle);
    event_queue.reset();
}

void Calibration::ledTask(void *params) {
    while (taskControl.load()) {
        //TBD_LOGW("CAL", "Calibration LED %d", ledTaskControl.load());
        int32_t blinks = taskControl;
        for (int32_t i = 0; i < blinks; i++) {
            drivers::Indicator::SetLedR(255);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (!taskControl.load()) break;
            drivers::Indicator::SetLedR(0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (!taskControl.load()) break;
        }
        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}

void Calibration::btnTask(void *params) {
    Event event = Event::BTN_PRESS;
    while (taskControl.load()) {
        while (drivers::GPIO::GetTrig0() == 0) {
            system::Task::sleep(50);
        }
        while (drivers::GPIO::GetTrig0() == 1) {
            system::Task::sleep(50);
        }
        while (drivers::GPIO::GetTrig0() == 0) {
            system::Task::sleep(50);
        }
        event_queue->push(event);
    }
}

void Calibration::acquireData(std::vector<uint32_t> &d) {
    //drivers::LedRGB::SetLedG(0);
    uint16_t data[4];
    int cnt = 0;
    uint32_t avgdata[4] = {0, 0, 0, 0};
    while (event_queue->try_pop()) {
        drivers::ADC::Update();
        drivers::ADC::GetChannelVals(data);
        for (int i = 0; i < 4; i++) {
            avgdata[i] += data[i];
        }
        if (cnt == 128) {
            cnt = 0;
            d.clear();
            for (int i = 0; i < 4; i++) {
                avgdata[i] >>= 7;
                d.push_back(avgdata[i]);
            }
            //drivers::LedRGB::SetLedG(8);
            //TBD_LOGE("CAL", "Average values %d, %d, %d, %d", avgdata[0], avgdata[1], avgdata[2], avgdata[3]);
            vTaskDelay(50 / portTICK_PERIOD_MS); // satisfy idle task
        }
        cnt++;
    }
}

void Calibration::calcPiecewiseLinearCoeffs(
    const std::string &dataID, calibration::CVConfig cvType)
{
    std::vector<std::vector<uint32_t >> xMat;
    std::vector<std::vector<float>> aCalMat;
    std::vector<std::vector<float>> bCalMat;
    xMat = model.GetMatrix(dataID);
    TBD_LOGD("CM", "Matrix content:");
    for (auto &i: xMat) {
        for (auto &j: i) {
            printf("%li\t", j);
        }
        printf("\n");
    }
    for (int i = 0; i < xMat.size() - 1; i++) {
        float y1 = 0.f, y2 = 0.f;
        if (cvType == calibration::CVConfig::CVBipolar) {
            y1 = (float) (i - 1);//(float)(i * 4096/2 - 1);
            y2 = (float) i;// ((i+1) * 4096/2 - 1);
        } else if (cvType == calibration::CVConfig::CVUnipolar) {
            y1 = (float) i * 0.5f;//(float)(i * 4096/2 - 1);
            y2 = (float) (i + 1) * 0.5f;// ((i+1) * 4096/2 - 1);
        }

        std::vector<float> aRow, bRow;
        for (int j = 0; j < xMat[i].size(); j++) {
            float x1 = xMat[i][j];
            float x2 = xMat[i + 1][j];
            // y = ax + b
            float a = (y2 - y1) / (x2 - x1);
            float b = (x2 * y1 - x1 * y2) / (x2 - x1);
            TBD_LOGD("CAL", "y2 %f, y1 %f, x2 %f, y2 %f, a %f, b %f", y2, y1, x2, x1, a, b);
            aRow.push_back(a);
            bRow.push_back(b);
        }
        aCalMat.push_back(aRow);
        bCalMat.push_back(bRow);
    }
    model.StoreMatrix("aCal" + dataID, aCalMat);
    model.StoreMatrix("bCal" + dataID, bCalMat);
}

void IRAM_ATTR Calibration::MapCVData(const uint16_t *adcInPtr, float *mapOutPtr) {
#if defined(CONFIG_TBD_PLATFORM_STR)
    // -1.5V .. 5.5V unipolar
    mapOutPtr[0] = adcInPtr[0] * 0.00034188034188f;
    mapOutPtr[1] = adcInPtr[1] * 0.00034188034188f;
    // -5V .. 5V bipolar
    mapOutPtr[2] = adcInPtr[2] * 0.0004884004884f - 1.f;
    mapOutPtr[3] = adcInPtr[3] * 0.0004884004884f - 1.f;
    // 0 .. 5V unipolar
    mapOutPtr[4] = adcInPtr[4] * 0.0002442002442f;
    mapOutPtr[5] = adcInPtr[5] * 0.0002442002442f;
    mapOutPtr[6] = adcInPtr[6] * 0.0002442002442f;
    mapOutPtr[7] = adcInPtr[7] * 0.0002442002442f;
#elif defined(CONFIG_TBD_PLATFORM_MK2)
#else
    // real-cv ins 0-1, pots 2-3
    for (int i = 0; i < N_CVS; i++) {
        if (*adcInPtr < 2048) { // which linear piece are we using
            if (configCV[i] == calibration::CVConfig::CVUnipolar)
                *mapOutPtr = aCoeffs05V[i] * (float) *adcInPtr + bCoeffs05V[i];
            else
                *mapOutPtr = aCoeffs10V[i] * (float) *adcInPtr + bCoeffs10V[i];
        } else {
            if (configCV[i] == calibration::CVConfig::CVUnipolar) {
                *mapOutPtr = aCoeffs05V[i + 4] * (float) *adcInPtr + bCoeffs05V[i + 4];
            } else
                *mapOutPtr = aCoeffs10V[i + 4] * (float) *adcInPtr + bCoeffs10V[i + 4];
        }
        // constrain ranges
        if (configCV[i] == calibration::CVConfig::CVUnipolar) {
            if (*mapOutPtr < 0.002f) *mapOutPtr = 0.f;
        } else {
            if (*mapOutPtr < -0.998f) *mapOutPtr = -1.f;
        }
        if (*mapOutPtr > 0.998f) *mapOutPtr = 1.f;
        adcInPtr++;
        mapOutPtr++;
    }
#endif
}

void Calibration::ConfigCVChannels(
    calibration::CVConfig ch0,
    calibration::CVConfig ch1,
    calibration::CVConfig ch2,
    calibration::CVConfig ch3)
{
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    if (ch0 == calibration::CVConfig::CVUnipolar)
        drivers::ADC::SetCVINUnipolar(0);
    else
        drivers::ADC::SetCVINBipolar(0);
    if (ch1 == calibration::CVConfig::CVUnipolar)
        drivers::ADC::SetCVINUnipolar(1);
    else
        drivers::ADC::SetCVINBipolar(1);

    configCV[0] = ch0;
    configCV[1] = ch1;
    configCV[2] = ch2;
    configCV[3] = ch3;
#endif
}

void Calibration::RequestCalibrationOnReboot() {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    model.SetCalibrateOnReboot(true);
#endif
}

const char* Calibration::GetCStrJSONCalibration() {
    return model.GetCStrJSONCalibration();
}

void Calibration::SetJSONCalibration(const std::string &calData) {
    model.SetJSONCalibration(calData);
    /* this is NOT thread safe !!! */
    model.LoadMatrix("aCalCalibration_CV_05V", aCoeffs05V);
    model.LoadMatrix("bCalCalibration_CV_05V", bCoeffs05V);
    model.LoadMatrix("aCalCalibration_CV_10V", aCoeffs10V);
    model.LoadMatrix("bCalCalibration_CV_10V", bCoeffs10V);
    /* NOT THREAD SAFE END */
}

}

#pragma GCC diagnostic pop