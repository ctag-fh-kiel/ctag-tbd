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
// Created by Robert Manzke on 06.03.20.
//

#include "ctagEnvFollow.hpp"
#include "helpers/ctagFastMath.hpp"

CTAG::SP::HELPERS::ctagEnvFollow::ctagEnvFollow() :
    _fs(44100.f), _a(0.001f), _d(0.001f), envOut(0.f)
{

}

void CTAG::SP::HELPERS::ctagEnvFollow::SetSampleRate(float fs) {
    _fs = fs;
}

float CTAG::SP::HELPERS::ctagEnvFollow::Process(float envIn) {
    envIn = fastfabs(envIn);
    if(envOut < envIn)
        envOut = envIn + _a * (envOut - envIn);
    else
        envOut = envIn + _d * (envOut - envIn);
    return envOut;
}

void CTAG::SP::HELPERS::ctagEnvFollow::SetAttack(float t_in_s) {
    _a = fasterexp(-1.f / (_fs * t_in_s));
}

void CTAG::SP::HELPERS::ctagEnvFollow::SetDecay(float t_in_s) {
    _d = fasterexp(-1.f / (_fs * t_in_s));
}

float CTAG::SP::HELPERS::ctagEnvFollow::GetDecayComp() {
    return _d;
}

float CTAG::SP::HELPERS::ctagEnvFollow::GetAttackComp() {
    return _a;
}

void CTAG::SP::HELPERS::ctagEnvFollow::SetDecayComp(float val) {
    _d = val;
}

void CTAG::SP::HELPERS::ctagEnvFollow::SetAttackComp(float val) {
    _a = val;
}
