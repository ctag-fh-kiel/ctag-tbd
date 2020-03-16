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

#include "ctagADEnv.hpp"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

float CTAG::SP::HELPERS::ctagADEnv::Process() {
    switch (envState){
        case EnvStateType::STATE_ATTACK:
            envAccum += attack;
            if(envAccum >= 1.f){
                envAccum = 1.f;
                envState = EnvStateType::STATE_DECAY;
            }
            break;
        case EnvStateType::STATE_DECAY:
            envAccum -= decay;
            if(envAccum <= 0.f){
                envAccum = 0.f;
                if(loop){
                    envState = EnvStateType::STATE_ATTACK;
                }else{
                    envState = EnvStateType::STATE_IDLE;
                }
            }
            break;
        default:
            break;
    }
    if(envMode == EnvModeType::MODE_LOG)
        return envAccum * envAccum; // approx very rough
    else
        return envAccum;
}

void CTAG::SP::HELPERS::ctagADEnv::SetAttack(float a_s) {
    if(a_s < tSample) a_s = tSample;
    attack = 1.f / (a_s * fSample);
}

void CTAG::SP::HELPERS::ctagADEnv::SetDecay(float d_s) {
    if(d_s < tSample) d_s = tSample;
    decay = 1.f / (d_s * fSample);
}

void CTAG::SP::HELPERS::ctagADEnv::Trigger() {
    envAccum = 0.f;
    envState = EnvStateType::STATE_ATTACK;
}

void ctagADEnv::SetModeLin() {
    envMode = EnvModeType::MODE_LIN;
}

void ctagADEnv::SetModeExp() {
    envMode = EnvModeType::MODE_LOG;
}

void ctagADEnv::SetLoop(bool l) {
    loop = l;
}

void ctagADEnv::SetSampleRate(float fs_hz) {
    fSample = fs_hz;
    tSample = 1.f / fSample;
}
