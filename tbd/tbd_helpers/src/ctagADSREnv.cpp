//  Created by Nigel Redmon on 12/18/12.
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the ADSR envelope generator and code,
//  read the series of articles by the author, starting here:
//  http://www.earlevel.com/main/2013/06/01/envelope-generators/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//
//  1.01  2016-01-02  njr   added calcCoef to SetTargetRatio functions that were in the ADSR widget but missing in this code
//  1.02  2017-01-04  njr   in calcCoef, checked for rate 0, to support non-IEEE compliant compilers
//  1.03  2020-04-08  njr   changed float to double; large target ratio and rate resulted in exp returning 1 in calcCoef
//

#include "helpers/ctagADSREnv.hpp"
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

ctagADSREnv::ctagADSREnv(void) {
    fs = 44100.f;
    Reset();
    SetAttack(0.f);
    SetDecay(0.f);
    SetRelease(0.f);
    SetSustain(1.0);
    SetTargetRatioA(0.3);
    SetTargetRatioDR(0.0001);
}

ctagADSREnv::~ctagADSREnv(void) {
}

void ctagADSREnv::SetAttack(float rate) {
    attackRate = rate * fs;
    attackCoef = calcCoef(attackRate, targetRatioA);
    attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
}

void ctagADSREnv::SetDecay(float rate) {
    decayRate = rate * fs;
    decayCoef = calcCoef(decayRate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
}

void ctagADSREnv::SetRelease(float rate) {
    releaseRate = rate * fs;
    releaseCoef = calcCoef(releaseRate, targetRatioDR);
    releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

float ctagADSREnv::calcCoef(float rate, float targetRatio) {
    float exponent = -logf_fast((1.0f + targetRatio) / targetRatio) / rate;
    //float a = (rate <= 0) ? 0.0f : expf(exponent);
    float b = (rate <= 0) ? 0.0f : exp5(exponent);
    return b;
}

void ctagADSREnv::SetSustain(float level) {
    sustainLevel = level;
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
}

void ctagADSREnv::SetTargetRatioA(float targetRatio) {
    if (targetRatio < 0.000000001f)
        targetRatio = 0.000000001f;  // -180 dB
    targetRatioA = targetRatio;
    SetAttack(attackRate / fs);
}

void ctagADSREnv::SetTargetRatioDR(float targetRatio) {
    if (targetRatio < 0.000000001f)
        targetRatio = 0.000000001f;  // -180 dB
    targetRatioDR = targetRatio;
    SetDecay(decayRate / fs);
    SetRelease(releaseRate / fs);
}

void ctagADSREnv::SetSampleRate(float fs_hz) {
    fs = fs_hz;
}

bool ctagADSREnv::IsIdle() {
    return state == env_idle;
}

void ctagADSREnv::SetModeLin() {
    SetTargetRatioA(10.f);
    SetTargetRatioDR(10.f);
}

void ctagADSREnv::SetModeExp() {
    SetTargetRatioA(1);
    SetTargetRatioDR(0.001);
}

void ctagADSREnv::Hold() {
    if (state == env_release) {
        state = env_sustain;
    }
}

float ctagADSREnv::Process() {
    switch (state) {
        case env_idle:
            break;
        case env_attack:
            output = attackBase + output * attackCoef;
            if (output >= 1.0) {
                output = 1.0;
                state = env_decay;
            }
            break;
        case env_decay:
            output = decayBase + output * decayCoef;
            if (output <= sustainLevel) {
                output = sustainLevel;
                state = env_sustain;
            }
            break;
        case env_sustain:
            output = sustainLevel;
            break;
        case env_release:
            output = releaseBase + output * releaseCoef;
            if (output <= 0.0) {
                output = 0.0;
                state = env_idle;
            }
    }
    return output;
}

void ctagADSREnv::Gate(bool gate) {
    if (gate){
        if(state == env_idle || state == env_release){
            state = env_attack;
        }
    }else if (state != env_idle){
        state = env_release;
    }
}

int ctagADSREnv::GetState() {
    return state;
}

void ctagADSREnv::Reset() {
    state = env_idle;
    output = 0.0;
}

float ctagADSREnv::GetOutput() {
    return output;
}