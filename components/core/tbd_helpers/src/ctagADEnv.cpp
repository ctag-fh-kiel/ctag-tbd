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

#include "tbd/helpers/ctagADEnv.hpp"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

#define LOG0001 -9.210340371976182f // -80dB

float CTAG::SP::HELPERS::ctagADEnv::Process() {
    float val;
    if (envMode == EnvModeType::MODE_LIN) {
        switch (envState) {
            case EnvStateType::STATE_ATTACK:
                val = envAccum + attack;
                if (val >= 1.f) {
                    envAccum = 1.f;
                    envState = EnvStateType::STATE_DECAY;
                } else {
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_DECAY:
                val = envAccum - decay;
                if (envAccum <= 0.f) {
                    envAccum = 0.f;
                    if (loop) {
                        envState = EnvStateType::STATE_ATTACK;
                    } else {
                        envState = EnvStateType::STATE_IDLE;
                    }
                } else {
                    envAccum = val;
                }
                break;
            default:
                break;
        }
    } else if (envMode == EnvModeType::MODE_LOG) {
        switch (envState) {
            case EnvStateType::STATE_ATTACK:
                val = envAccum * attack;
                if (val >= 1.f) {
                    envAccum = 1.f;
                    envState = EnvStateType::STATE_DECAY;
                } else {
                    envAccum = val;
                }
                break;
            case EnvStateType::STATE_DECAY:
                val = envAccum * decay;
                if (envAccum <= 0.0001f) {
                    if (loop) {
                        envState = EnvStateType::STATE_ATTACK;
                        envAccum = 0.0001f;
                    } else {
                        envState = EnvStateType::STATE_IDLE;
                        envAccum = 0.f;
                    }
                } else {
                    envAccum = val;
                }
                break;
            default:
                break;
        }
    }
    if (envAccum > 1.f) envAccum = 1.f;
    if (envAccum < 0.f) envAccum = 0.f;
    return envAccum;
}

void CTAG::SP::HELPERS::ctagADEnv::SetAttack(float a_s) {
    if (envMode == EnvModeType::MODE_LIN)
        attack = 1.f / (a_s * fSample);
    else
        attack = 1.f - LOG0001 / (a_s * fSample);
}

void CTAG::SP::HELPERS::ctagADEnv::SetDecay(float d_s) {
    if (envMode == EnvModeType::MODE_LIN)
        decay = 1.f / (d_s * fSample);
    else
        decay = 1.f + LOG0001 / (d_s * fSample);
}

void CTAG::SP::HELPERS::ctagADEnv::Trigger() {
    if (envMode == EnvModeType::MODE_LIN) {
        envAccum = 0.f;
    } else {
        envAccum = 0.0001f;
    }
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

bool ctagADEnv::GetLoop() {
    return loop;
}

bool ctagADEnv::GetIsRunning() {
    return envState != EnvStateType::STATE_IDLE ? true : false;
}

void ctagADEnv::Reset() {
    envAccum = 0.f;
    envState = EnvStateType::STATE_IDLE;
}
