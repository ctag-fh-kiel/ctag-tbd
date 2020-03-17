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


#include "CalibrationModel.hpp"
#include <cmath>

using namespace std;

CTAG::CAL::CalibrationModel::CalibrationModel() {
    loadJSON(m, MODELJSONFN);
}

CTAG::CAL::CalibrationModel::~CalibrationModel() {

}

void CTAG::CAL::CalibrationModel::PrintSelf() {
    printJSON(m);
}

void CTAG::CAL::CalibrationModel::CreateMatrix() {
    matrix.SetArray();
    matrix.Clear();
}

void CTAG::CAL::CalibrationModel::PushRow(const vector<uint32_t> data) {
    Value matrixRow(kArrayType);
    for(auto &i : data){
        matrixRow.PushBack(i, matrix.GetAllocator());
    }
    matrix.PushBack(matrixRow, matrix.GetAllocator());
}

void CTAG::CAL::CalibrationModel::StoreMatrix(const string &id) {
    if(m.HasMember(id)) m.RemoveMember(id);
    Value sid(id, m.GetAllocator());
    m.GetObject().AddMember(sid, matrix.Move(), m.GetAllocator());
    storeJSON(m, MODELJSONFN);
}

vector<vector<uint32_t >> CTAG::CAL::CalibrationModel::GetMatrix(const string &id) {
    vector<vector<uint32_t>> mat;
    for(auto &i: m[id].GetArray()){
        vector<uint32_t > v;
        for(auto &j: i.GetArray()){
            v.push_back(j.GetInt());
        }
        mat.push_back(v);
    }
    return mat;
}

void CTAG::CAL::CalibrationModel::StoreMatrix(const string &id, const vector<vector<float>> mat) {
    if(m.HasMember(id)) m.RemoveMember(id);

    Value rows(kArrayType);
    Value sid(id, m.GetAllocator());

    for(auto i:mat){
        Value col(kArrayType);
        for(auto j: i){
            ESP_LOGD("CM", "Storing %f", j);
            Value f(kNumberType);
            if(isinf(j) || isnan(j)) f.SetFloat(0);
            else f.SetFloat(j);
            col.PushBack(f, m.GetAllocator());
        }
        rows.PushBack(col, m.GetAllocator());
    }

    m.GetObject().AddMember(sid, rows.Move(), m.GetAllocator());
    storeJSON(m, MODELJSONFN);
}

void CTAG::CAL::CalibrationModel::LoadMatrix(const string &id, float *data) {
    if(!m.HasMember(id)) return;
    for(auto &i: m[id].GetArray()){
        for(auto &j: i.GetArray()){
            *data = j.GetFloat();
            ESP_LOGD("Cal", "Value %f", *data);
            data++;
        }
    }
}
