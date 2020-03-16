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


//
// Created by Robert Manzke on 03.03.20.
//

#include "ctagBiQuad.hpp"
#include "ctagFastMath.hpp"

using namespace CTAG::SP::HELPERS;

CTAG::SP::HELPERS::ctagBiQuad::ctagBiQuad() {
    Mute();
    for(int i=0;i<6;i++)coeff[i] = 0.f;
    type = BIQUAD_TYPE::LP;
    _fs = 44100.f;
    _cut = 1000.f;
    _q = 0.5f;
    calcCoeffs();
}

void CTAG::SP::HELPERS::ctagBiQuad::SetType(CTAG::SP::HELPERS::BIQUAD_TYPE t) {
    type = t;
}

void CTAG::SP::HELPERS::ctagBiQuad::Mute() {
    w[0] = w[1] = 0.f;
}

/*
void CTAG::SP::HELPERS::ctagBiQuad::Process(float *data, const uint32_t sz, const uint32_t inc) {

}

float CTAG::SP::HELPERS::ctagBiQuad::Process(float data) {
    return 0;
}
*/
void CTAG::SP::HELPERS::ctagBiQuad::SetCutoffHz(float cut) {
    _cut = cut;
    calcCoeffs();
}

void CTAG::SP::HELPERS::ctagBiQuad::SetQ(float q) {
    _q = q;
    calcCoeffs();
}

void CTAG::SP::HELPERS::ctagBiQuad::SetSampleRate(float fs) {
    _fs = fs;
    calcCoeffs();
}

void CTAG::SP::HELPERS::ctagBiQuad::calcCoeffs(){
    switch (type){
        case BIQUAD_TYPE ::LP:
            dsps_biquad_gen_lpf_f32(coeff, _cut/_fs, _q);
            break;
        case BIQUAD_TYPE ::BP:
            dsps_biquad_gen_bpf0db_f32(coeff, _cut / _fs, _q);
            break;
        case BIQUAD_TYPE ::HP:
            dsps_biquad_gen_hpf_f32(coeff, _cut / _fs, _q);
            break;
        default:
            break;
    }
}

void ctagBiQuad::Process(float *data, const uint32_t sz) {
    dsps_biquad_f32_ae32(data, data, sz, coeff, w);
}
