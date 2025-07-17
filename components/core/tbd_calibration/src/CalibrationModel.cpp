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
#include "tbd/calibration/calibration_model.hpp"

#include <cmath>
#include "rapidjson/filereadstream.h"
#include <tbd/logging.hpp>

#include "rapidjson/writer.h"


namespace rj = rapidjson;

namespace tbd::calibration {
CalibrationModel::CalibrationModel() {
    loadJSON(m, MODELJSONFN);
}

void CalibrationModel::PrintSelf() {
    printJSON(m);
}

void CalibrationModel::CreateMatrix() {
    matrix.SetArray();
    matrix.Clear();
}

void CalibrationModel::PushRow(const std::vector<uint32_t> data) {
    rj::Value matrixRow(rj::kArrayType);
    for (auto &i : data) {
        matrixRow.PushBack(static_cast<unsigned int>(i), matrix.GetAllocator());
    }
    matrix.PushBack(matrixRow, matrix.GetAllocator());
}

void CalibrationModel::StoreMatrix(const std::string &id) {
    if (m.HasMember(id)) m.RemoveMember(id);
    rj::Value sid(id, m.GetAllocator());
    m.GetObject().AddMember(sid, matrix.Move(), m.GetAllocator());
    storeJSON(m, MODELJSONFN);
}

std::vector<std::vector<uint32_t >> CalibrationModel::GetMatrix(const std::string &id) {
    std::vector<std::vector<uint32_t>> mat;
    for (auto &i: m[id].GetArray()) {
        std::vector<uint32_t> v;
        for (auto &j: i.GetArray()) {
            v.push_back(j.GetInt());
        }
        mat.push_back(v);
    }
    return mat;
}

void CalibrationModel::StoreMatrix(const std::string &id, const std::vector<std::vector<float>> mat) {
    if (m.HasMember(id)) m.RemoveMember(id);

    rj::Value rows(rj::kArrayType);
    rj::Value sid(id, m.GetAllocator());

    for (auto i:mat) {
        rj::Value col(rj::kArrayType);
        for (auto j: i) {
            TBD_LOGD("CM", "Storing %f", j);
            rj::Value f(rj::kNumberType);
            if (std::isinf(j) || std::isnan(j)) f.SetFloat(0);
            else f.SetFloat(j);
            col.PushBack(f, m.GetAllocator());
        }
        rows.PushBack(col, m.GetAllocator());
    }

    m.GetObject().AddMember(sid, rows.Move(), m.GetAllocator());
    storeJSON(m, MODELJSONFN);
}

void CalibrationModel::LoadMatrix(const std::string &id, float *data) {
    if (!m.HasMember(id)) return;
    for (auto &i: m[id].GetArray()) {
        for (auto &j: i.GetArray()) {
            *data = j.GetFloat();
            TBD_LOGD("Cal", "Value %f", *data);
            data++;
        }
    }
}

bool CalibrationModel::GetCalibrateOnReboot() {
    if (!m.HasMember("CalibrationOnReboot")) return false;
    if (!m["CalibrationOnReboot"].IsBool()) return false;
    return m["CalibrationOnReboot"].GetBool() == true;
}

void CalibrationModel::SetCalibrateOnReboot(bool val) {
    if (!m.HasMember("CalibrationOnReboot")) {
        rj::Value b(val);
        m.AddMember(b, b.Move(), m.GetAllocator());
    } else {
        m["CalibrationOnReboot"].SetBool(val);
    }
    storeJSON(m, MODELJSONFN);
}

const char *CalibrationModel::GetCStrJSONCalibration() {
#if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_AEM)
    json.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(json);
    m.Accept(writer);
    return json.GetString();
#else
    static const char *json = "{}";
    return json;
#endif
}

void CalibrationModel::SetJSONCalibration(const std::string &calData) {
    rj::Document d;
    d.Parse(calData);
    if (!d.IsObject()) {
        TBD_LOGE("CALMODEL", "Could not parse json string!");
        return;
    }
    storeJSON(d, MODELJSONFN);
    loadJSON(m, MODELJSONFN);
}

}
