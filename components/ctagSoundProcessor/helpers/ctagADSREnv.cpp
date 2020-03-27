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
// Created by Robert Manzke on 09.02.20.
//

#include "ctagADSREnv.hpp"
#include "esp_log.h"
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

#define LOG0001 -9.210340371976182f // -80dB


float CTAG::SP::HELPERS::ctagADSREnv::Process() {
    float val;
    if(envMode == EnvModeType::MODE_LIN){
        switch (envState){
            case EnvStateType::STATE_ATTACK:
                val = envAccum + attack;
                if(val >= 1.f){
                    envAccum = 1.f;
                    //ESP_LOGW("ADSR", "Switching from attack to decay");
                    envState = EnvStateType::STATE_DECAY;
                }else{
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_DECAY:
                val = envAccum - decay;
                if(val <= sustain){
                    envAccum = sustain;
                    //ESP_LOGW("ADSR", "Switching from decy to sus");
                    envState = EnvStateType::STATE_SUSTAIN;
                }else{
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_RELEASE:
                val = envAccum - release;
                if(val <= 0.0001f){
                    envAccum = 0.f;
                    //ESP_LOGW("ADSR", "Switching from sus to rel");
                    envState = EnvStateType::STATE_IDLE;
                }else{
                    envAccum = val;
                }
            default:
                break;
        }
    }else{
        switch (envState){
            case EnvStateType::STATE_ATTACK:
                val = envAccum * attack;
                if(val >= 1.f){
                    envAccum = 1.f;
                    //ESP_LOGW("ADSR", "Switching from attack to decay");
                    envState = EnvStateType::STATE_DECAY;
                }else{
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_DECAY:
                val = envAccum * decay;
                if(val <= sustain){
                    envAccum = sustain;
                    //ESP_LOGW("ADSR", "Switching from decy to sus");
                    envState = EnvStateType::STATE_SUSTAIN;
                }else{
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_RELEASE:
                val = envAccum * release;
                if(val <= 0.0001f){
                    envAccum = 0.f;
                    //ESP_LOGW("ADSR", "Switching from sus to rel");
                    envState = EnvStateType::STATE_IDLE;
                }else{
                    envAccum = val;
                }
            default:
                break;
        }
    }
    if(envAccum > 1.f) envAccum = 1.f;
    if(envAccum < 0.f) envAccum = 0.f;
    return envAccum;
}

void CTAG::SP::HELPERS::ctagADSREnv::SetAttack(float a_s) {
    if(envMode == EnvModeType::MODE_LIN)
        attack = 1.f / (a_s * fSample);
    else
        attack = 1.f - LOG0001 / (a_s * fSample);
}

void CTAG::SP::HELPERS::ctagADSREnv::SetDecay(float d_s) {
    if(envMode == EnvModeType::MODE_LIN)
        decay = 1.f / (d_s * fSample);
    else
        decay = 1.f + LOG0001 / (d_s * fSample);
}

void CTAG::SP::HELPERS::ctagADSREnv::Gate(bool isGate) {
    if(isGate != preGate){
        preGate = isGate;
        if(preGate == true){
            envAccum = 0.0001f;
            envState = EnvStateType::STATE_ATTACK;
            //ESP_LOGW("ADSR", "trig -> attack");
        }else{
            envState = EnvStateType::STATE_RELEASE;
            //ESP_LOGW("ADSR", "trig -> release");
        }
    }
}

void ctagADSREnv::SetModeLin() {
    envMode = EnvModeType::MODE_LIN;
}

void ctagADSREnv::SetModeExp() {
    envMode = EnvModeType::MODE_LOG;
}

void ctagADSREnv::SetSampleRate(float fs_hz) {
    fSample = fs_hz;
    tSample = 1.f / fSample;
}

void ctagADSREnv::SetSustain(float s_s) {
    sustain = s_s;
}

void ctagADSREnv::SetRelease(float r_s) {
    if(envMode == EnvModeType::MODE_LIN)
        release = 1.f / (r_s * fSample);
    else
        release = 1.f + LOG0001 / (r_s * fSample);
}
