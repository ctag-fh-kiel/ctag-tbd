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


#include "esp_log.h"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "led_rgb.hpp"
#include "adc.hpp"

using namespace CTAG;
using namespace CTAG::CAL;

TaskHandle_t Calibration::ledTaskHandle;
TaskHandle_t Calibration::btnTaskHandle;
std::atomic_int32_t Calibration::taskControl;
QueueHandle_t Calibration::evQueue;
unique_ptr<CalibrationModel> Calibration::model;
float Calibration::aCoeffs05V[4 * 2];
float Calibration::bCoeffs05V[4 * 2];
float Calibration::aCoeffs10V[4 * 2];
float Calibration::bCoeffs10V[4 * 2];
CVConfig Calibration::configCV[4];


void Calibration::Init() {
    model = std::make_unique<CalibrationModel>();
    ConfigCVChannels(CVConfig::CVUnipolar, CVConfig::CVUnipolar, CVConfig::CVUnipolar, CVConfig::CVUnipolar);
    DRIVERS::ADC::SetCVINUnipolar(0);
    DRIVERS::ADC::SetCVINUnipolar(1);
    ESP_LOGI("CAL", "Check calibration request");
    if (model->GetCalibrateOnReboot()) {
        ESP_LOGI("CAL", "Starting calibration");
        DRIVERS::LedRGB::SetLedRGB(0, 0, 0);
        doCalibration();
        ESP_LOGI("CAL", "Calibration completed");
        DRIVERS::LedRGB::SetLedRGB(0, 0, 0);
        model->SetCalibrateOnReboot(false);
    }
    model->LoadMatrix("aCalCalibration_CV_05V", aCoeffs05V);
    model->LoadMatrix("bCalCalibration_CV_05V", bCoeffs05V);
    model->LoadMatrix("aCalCalibration_CV_10V", aCoeffs10V);
    model->LoadMatrix("bCalCalibration_CV_10V", bCoeffs10V);
}

void Calibration::doCalibration() {
    // start tasks for ui
    taskControl.store(1);
    evQueue = xQueueCreate(10, sizeof(Event));
    xTaskCreate(&Calibration::ledTask, "led_task", 4096, nullptr, 5, &ledTaskHandle);
    xTaskCreate(&Calibration::btnTask, "btn_task", 4096, nullptr, 5, &btnTaskHandle);

    std::vector<uint32_t> data;
    model->CreateMatrix();
    DRIVERS::ADC::SetCVINUnipolar(0);
    DRIVERS::ADC::SetCVINUnipolar(1);
    // get mins
    ESP_LOGW("CAL", "Adjust CV INs to min value (clock left resp. 0V CV in)");
    acquireData(data);
    model->PushRow(data);
    // get maxs
    taskControl.store(2);
    ESP_LOGW("CAL", "Adjust CV INs to middle / +2.5V");
    acquireData(data);
    model->PushRow(data);
    taskControl.store(3);
    // get mid CV in
    ESP_LOGW("CAL", "Adjust CV INs to max value (clock right resp. +5V CV in)");
    acquireData(data);
    model->PushRow(data);
    model->StoreMatrix("Calibration_CV_05V");
    taskControl.store(4);
    DRIVERS::ADC::SetCVINBipolar(0);
    DRIVERS::ADC::SetCVINBipolar(1);
    model->CreateMatrix();
    // get mins
    ESP_LOGW("CAL", "Adjust CV INs to min value (clock left resp. -5V CV in)");
    acquireData(data);
    model->PushRow(data);
    // get maxs
    taskControl.store(5);
    ESP_LOGW("CAL", "Adjust CV INs to 0V CV in");
    acquireData(data);
    model->PushRow(data);
    taskControl.store(6);
    // get mid CV in
    ESP_LOGW("CAL", "Adjust CV INs to max value (clock right resp. +5V CV in)");
    acquireData(data);
    model->PushRow(data);
    model->StoreMatrix("Calibration_CV_10V");
    model->PrintSelf();

    DRIVERS::ADC::SetCVINUnipolar(0);
    DRIVERS::ADC::SetCVINUnipolar(1);

    calcPiecewiseLinearCoeffs("Calibration_CV_05V", CVConfig::CVUnipolar);
    calcPiecewiseLinearCoeffs("Calibration_CV_10V", CVConfig::CVBipolar);

    taskControl.store(0);
    vTaskDelete(ledTaskHandle);
    vTaskDelete(btnTaskHandle);
    vQueueDelete(evQueue);
}

void Calibration::ledTask(void *params) {
    while (taskControl.load()) {
        //ESP_LOGW("CAL", "Calibration LED %d", ledTaskControl.load());
        int32_t blinks = taskControl;
        for (int32_t i = 0; i < blinks; i++) {
            DRIVERS::LedRGB::SetLedR(255);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (!taskControl.load()) break;
            DRIVERS::LedRGB::SetLedR(0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (!taskControl.load()) break;
        }
        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}

void Calibration::btnTask(void *params) {
    Event ev = Event::BTN_PRESS;
    while (taskControl.load()) {
        while (DRIVERS::GPIO::GetTrig0() == 0) vTaskDelay(50 / portTICK_PERIOD_MS);
        while (DRIVERS::GPIO::GetTrig0() == 1) vTaskDelay(50 / portTICK_PERIOD_MS);
        while (DRIVERS::GPIO::GetTrig0() == 0) vTaskDelay(50 / portTICK_PERIOD_MS);
        xQueueSend(evQueue, &ev, portMAX_DELAY);
    }
}

void Calibration::acquireData(std::vector<uint32_t> &d) {
    Event ev = Event::NONE;
    //DRIVERS::LedRGB::SetLedG(0);
    uint16_t data[4];
    int cnt = 0;
    uint32_t avgdata[4] = {0, 0, 0, 0};
    while (!xQueueReceive(evQueue, &ev, 0)) {
        DRIVERS::ADC::Update();
        DRIVERS::ADC::GetChannelVals(data);
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
            //DRIVERS::LedRGB::SetLedG(8);
            //ESP_LOGE("CAL", "Average values %d, %d, %d, %d", avgdata[0], avgdata[1], avgdata[2], avgdata[3]);
            vTaskDelay(50 / portTICK_PERIOD_MS); // satisfy idle task
        }
        cnt++;
    }
}

void Calibration::calcPiecewiseLinearCoeffs(const string &dataID, CVConfig cvType) {
    vector<vector<uint32_t >> xMat;
    vector<vector<float>> aCalMat;
    vector<vector<float>> bCalMat;
    xMat = model->GetMatrix(dataID);
    ESP_LOGD("CM", "Matrix content:");
    for (auto &i: xMat) {
        for (auto &j: i) {
            printf("%li\t", j);
        }
        printf("\n");
    }
    for (int i = 0; i < xMat.size() - 1; i++) {
        float y1 = 0.f, y2 = 0.f;
        if (cvType == CVConfig::CVBipolar) {
            y1 = (float) (i - 1);//(float)(i * 4096/2 - 1);
            y2 = (float) i;// ((i+1) * 4096/2 - 1);
        } else if (cvType == CVConfig::CVUnipolar) {
            y1 = (float) i * 0.5f;//(float)(i * 4096/2 - 1);
            y2 = (float) (i + 1) * 0.5f;// ((i+1) * 4096/2 - 1);
        }

        vector<float> aRow, bRow;
        for (int j = 0; j < xMat[i].size(); j++) {
            float x1 = xMat[i][j];
            float x2 = xMat[i + 1][j];
            // y = ax + b
            float a = (y2 - y1) / (x2 - x1);
            float b = (x2 * y1 - x1 * y2) / (x2 - x1);
            ESP_LOGD("CAL", "y2 %f, y1 %f, x2 %f, y2 %f, a %f, b %f", y2, y1, x2, x1, a, b);
            aRow.push_back(a);
            bRow.push_back(b);
        }
        aCalMat.push_back(aRow);
        bCalMat.push_back(bRow);
    }
    model->StoreMatrix("aCal" + dataID, aCalMat);
    model->StoreMatrix("bCal" + dataID, bCalMat);
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
            if (configCV[i] == CVConfig::CVUnipolar)
                *mapOutPtr = aCoeffs05V[i] * (float) *adcInPtr + bCoeffs05V[i];
            else
                *mapOutPtr = aCoeffs10V[i] * (float) *adcInPtr + bCoeffs10V[i];
        } else {
            if (configCV[i] == CVConfig::CVUnipolar) {
                *mapOutPtr = aCoeffs05V[i + 4] * (float) *adcInPtr + bCoeffs05V[i + 4];
            } else
                *mapOutPtr = aCoeffs10V[i + 4] * (float) *adcInPtr + bCoeffs10V[i + 4];
        }
        // constrain ranges
        if (configCV[i] == CVConfig::CVUnipolar) {
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

void Calibration::ConfigCVChannels(CVConfig ch0, CVConfig ch1, CVConfig ch2, CVConfig ch3) {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    if (ch0 == CVConfig::CVUnipolar)
        DRIVERS::ADC::SetCVINUnipolar(0);
    else
        DRIVERS::ADC::SetCVINBipolar(0);
    if (ch1 == CVConfig::CVUnipolar)
        DRIVERS::ADC::SetCVINUnipolar(1);
    else
        DRIVERS::ADC::SetCVINBipolar(1);

    configCV[0] = ch0;
    configCV[1] = ch1;
    configCV[2] = ch2;
    configCV[3] = ch3;
#endif
}

void Calibration::RequestCalibrationOnReboot() {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    model->SetCalibrateOnReboot(true);
#endif
}
