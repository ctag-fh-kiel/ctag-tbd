/**
 *  Enhanced Simple Tank Reverb
 *
 *  Copyright (C) 2006-2018 Teru Kamogashira
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  aint32_t with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "strev.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

// Fs = 29.761 kHz
const int32_t FV3_(strev)::allpCo[] = {142, 107, 379, 277, 672, 908, 1800, 2656,};
const int32_t FV3_(strev)::delayCo[] = {4453, 3720, 4217, 3163,};
const int32_t FV3_(strev)::idxLCo[] = {266, 2974, 1913, 1996, 1990, 187, 1066,};
const int32_t FV3_(strev)::idxRCo[] = {353, 3627, 1228, 2673, 2111, 335, 121,};
const int32_t FV3_(strev)::allpM_EXCURSION = 32;

FV3_(strev)::~FV3_(strev)(){
    if(totalBuf1 != NULL) heap_caps_free(totalBuf1);
    if(totalBuf2 != NULL) heap_caps_free(totalBuf2);
    totalBuf1 = NULL;
    totalBuf2 = NULL;
}

FV3_(strev)::FV3_(strev)() {
    //setSampleRate(44100.f);
    //setOSFactor(1);
    //setRSFactor(1.f);
    totalBuf1 = NULL;
    totalBuf2 = NULL;
    isMono = false;
    setrt60(1);
    setdccutfreq(10);
    setidiffusion1(0.750);
    setidiffusion2(0.625);
    setdiffusion1(0.7);
    setdiffusion2(0.5);
    setinputdamp(10000);
    setdamp(4000);
    setoutputdamp(10000);
    setspin(1);
    setspindiff(0.1);
    setspinlimit(10);
    setwander(.1);
    setmodulationnoise1(0.05);
    setmodulationnoise2(0.03);
    setAutoDiff(true);
}

void FV3_(strev)::mute() {
    FV3_(revbase)::mute();
    for (int32_t i = 0; i < FV3_STREV_NUM_ALLPASS_4; i++) allpassC[i].mute();
    allpassC_31_33.mute();
    allpassC_55_59.mute();
    allpassM_23_24.mute();
    allpassM_46_48.mute();
    delayC_30.mute();
    delayC_39.mute();
    delayC_54.mute();
    delayC_63.mute();
    dccut1.mute();
    lpf_in.mute();
    lpfC_30.mute();
    lpfC_54.mute();
    lfo1_lpf.mute();
    lfo2_lpf.mute();
    out1_lpf.mute();
    out2_lpf.mute();
    lfo1.mute();
    lfo2.mute();
    //noise1L.mute();
}

void FV3_(strev)::processreplace(fv3_float_t *samples, int32_t numsamples){
    //if (numsamples <= 0) return;
    //int32_t count = numsamples * getOSFactor();
    //growWave(numsamples);

    fv3_float_t outL, outR, input;
    for(uint32_t i = 0;i<numsamples; i++) {
        if(isMono){
            input = samples[i*2];
        }else{
            input = (samples[i*2] + samples[i*2 + 1]) / 2.f;
        }

        // DC-cut HPF + input LPF
        input = lpf_in(dccut1(input));

        // Feed through allpasses in series
        for (int32_t j = 0; j < FV3_STREV_NUM_ALLPASS_4; j++)
            input = allpassC[j]._process(input);

        // Split into L/R part
        outL = input + delayC_63._getlast() * decay;
        outR = input + delayC_39._getlast() * decay;

        // Tank
        fv3_float_t noise1 = noise1L.Process();//noise1L();
        fv3_float_t lfo1q = lfo1_lpf((lfo1() + modnoise1 * noise1) * wander);
        fv3_float_t lfo2q = lfo2_lpf((lfo2() - modnoise1 * noise1) * wander);
        delayC_39._process(allpassC_31_33._process(
                decay * lpfC_30(delayC_30._process(allpassM_23_24._process(outL, lfo1q, modnoise2 * noise1)))));
        delayC_63._process(allpassC_55_59._process(
                decay * lpfC_54(delayC_54._process(allpassM_46_48._process(outR, lfo2q, -1. * modnoise2 * noise1)))));

        outL = delayC_54._get_z(iLC[0]) + delayC_54._get_z(iLC[1]) - allpassC_55_59._get_z(iLC[2]) +
               delayC_63._get_z(iLC[3])
               - delayC_30._get_z(iLC[4]) - allpassC_31_33._get_z(iLC[5]) - delayC_39._get_z(iLC[6]);
        outR = delayC_30._get_z(iRC[0]) + delayC_30._get_z(iRC[1]) - allpassC_31_33._get_z(iRC[2]) +
               delayC_39._get_z(iRC[3])
               - delayC_54._get_z(iRC[4]) - allpassC_55_59._get_z(iRC[5]) - delayC_63._get_z(iRC[6]);


        fv3_float_t fpL = delayWL(out1_lpf(outL));
        fv3_float_t fpR = delayWR(out2_lpf(outR));
        samples[i*2] = fpL * wet1 + fpR * wet2 + delayL(samples[i*2]) * dry;
        if(isMono){
            samples[i*2 + 1]= fpR * wet1 + fpL * wet2 + delayR(samples[i*2]) * dry;
        }else{
            samples[i*2 + 1]= fpR * wet1 + fpL * wet2 + delayR(samples[i*2 + 1]) * dry;
        }

        UNDENORMAL(samples[i*2]);
        UNDENORMAL(samples[i*2 + 1]);

    }
    //SRC.dsrc(overO.L, overO.R, origOutL, origOutR, numsamples);

}

/*
  The following reverb algorithm and the allpass modulation filter's algorithm
  originate from the following article and was originally developed by David Griesinger.
  
  J. Dattorro, Effect Design Part 1: Reverberator and Other Filters (1997)
  J. Audio Eng. Soc., vol. 45, pp. 660-684, Sept. 1997.
  https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf

  J. Dattorro, Effect Design Part 2: Delay-Line Modulation and Chorus (1997)
  J. Audio Eng. Soc., vol. 45, pp. 764-788, Oct. 1997.
  http://ccrma.stanford.edu/~dattorro/EffectDesignPart2.pdf

  J. Dattorro, Effect Design: Part 3 Oscillators: Sinusoidal and Pseudonoise (2002)
  J. Audio Eng. Soc., vol. 50, pp. 115-146, Mar. 2002.
  http://ccrma.stanford.edu/~dattorro/EffectDesignPart3.pdf
*/

void FV3_(strev)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR,
                                 int32_t numsamples) {
    if (numsamples <= 0) return;
    int32_t count = numsamples * getOSFactor();
    //growWave(count);

    fv3_float_t outL, outR, input, *origOutL = outputL, *origOutR = outputR;
    //SRC.usrc(inputL, inputR, over.L, over.R, numsamples);
    /*
    inputL = over.L;
    inputR = over.R;
    outputL = overO.L;
    outputR = overO.R;
     */

    while (count-- > 0) {
        outL = outR = 0.0;
        input = (*inputL + *inputR) / 2;

        // DC-cut HPF + input LPF
        input = lpf_in(dccut1(input));

        // Feed through allpasses in series
        for (int32_t i = 0; i < FV3_STREV_NUM_ALLPASS_4; i++)
            input = allpassC[i]._process(input);

        // Split into L/R part
        outL = input + delayC_63._getlast() * decay;
        outR = input + delayC_39._getlast() * decay;

        // Tank
        fv3_float_t noise1 = noise1L.Process();//noise1L();
        fv3_float_t lfo1q = lfo1_lpf((lfo1() + modnoise1 * noise1) * wander);
        fv3_float_t lfo2q = lfo2_lpf((lfo2() - modnoise1 * noise1) * wander);
        delayC_39._process(allpassC_31_33._process(
                decay * lpfC_30(delayC_30._process(allpassM_23_24._process(outL, lfo1q, modnoise2 * noise1)))));
        delayC_63._process(allpassC_55_59._process(
                decay * lpfC_54(delayC_54._process(allpassM_46_48._process(outR, lfo2q, -1. * modnoise2 * noise1)))));

        outL = delayC_54._get_z(iLC[0]) + delayC_54._get_z(iLC[1]) - allpassC_55_59._get_z(iLC[2]) +
               delayC_63._get_z(iLC[3])
               - delayC_30._get_z(iLC[4]) - allpassC_31_33._get_z(iLC[5]) - delayC_39._get_z(iLC[6]);
        outR = delayC_30._get_z(iRC[0]) + delayC_30._get_z(iRC[1]) - allpassC_31_33._get_z(iRC[2]) +
               delayC_39._get_z(iRC[3])
               - delayC_54._get_z(iRC[4]) - allpassC_55_59._get_z(iRC[5]) - delayC_63._get_z(iRC[6]);

        fv3_float_t fpL = delayWL(out1_lpf(outL));
        fv3_float_t fpR = delayWR(out2_lpf(outR));
        *outputL = fpL * wet1 + fpR * wet2 + delayL(*inputL) * dry;
        *outputR = fpR * wet1 + fpL * wet2 + delayR(*inputR) * dry;
        UNDENORMAL(*outputL);
        UNDENORMAL(*outputR);
        inputL++;
        inputR++;
        outputL++;
        outputR++;
    }
    //SRC.dsrc(overO.L, overO.R, origOutL, origOutR, numsamples);
}

void FV3_(strev)::setrt60(fv3_float_t value) {
    rt60 = value;
    fv3_float_t back = rt60 * getTotalSampleRate();
    UNDENORMAL(back);
    if (back > 0) decay = std::pow((fv3_float_t) 10.0, -3 * (fv3_float_t) tankDelay / back);
    else decay = 0;
    if (autoDiff) {
        fv3_float_t adiff = decay + 0.15;
        if (adiff < 0.25) adiff = 0.25;
        if (adiff > 0.5) adiff = 0.5;
        setdiffusion2(adiff);
    }
}

fv3_float_t FV3_(strev)::getrt60() {
    return rt60;
}

void FV3_(strev)::setdccutfreq(fv3_float_t value) {
    dccutfq = limFs2(value);
    dccut1.setCutOnFreq(dccutfq, getTotalSampleRate());
}

fv3_float_t FV3_(strev)::getdccutfreq() {
    return dccutfq;
}

void FV3_(strev)::setidiffusion1(fv3_float_t value) {
    idiff1 = value;
    allpassC[0].setfeedback(-1 * idiff1);
    allpassC[1].setfeedback(-1 * idiff1);
}

fv3_float_t FV3_(strev)::getidiffusion1() {
    return idiff1;
}

void FV3_(strev)::setidiffusion2(fv3_float_t value) {
    idiff2 = value;
    allpassC[2].setfeedback(-1 * idiff2);
    allpassC[3].setfeedback(-1 * idiff2);
}

fv3_float_t FV3_(strev)::getidiffusion2() {
    return idiff2;
}

void FV3_(strev)::setdiffusion1(fv3_float_t value) {
    diff1 = value;
    allpassC_31_33.setfeedback(-1 * diff1);
    allpassC_55_59.setfeedback(-1 * diff1);
}

fv3_float_t FV3_(strev)::getdiffusion1() {
    return diff1;
}

void FV3_(strev)::setdiffusion2(fv3_float_t value) {
    diff2 = value;
    allpassM_23_24.setfeedback(diff2);
    allpassM_46_48.setfeedback(diff2);
}

fv3_float_t FV3_(strev)::getdiffusion2() {
    return diff2;
}

void FV3_(strev)::setspin(fv3_float_t value) {
    if (spin > 0) {
        spin = limFs2(value);
        lfo1.setFreq(spin, getTotalSampleRate());
        lfo2.setFreq(spin + spindiff, getTotalSampleRate());
    } else {
        spin = 0;
        lfo1.setFreq(0);
        lfo2.setFreq(0);
    }
}

fv3_float_t FV3_(strev)::getspin() {
    return spin;
}


void FV3_(strev)::setspindiff(fv3_float_t value) {
    spindiff = limFs2(value);
    setspin(getspin());
}

fv3_float_t FV3_(strev)::getspindiff() {
    return spindiff;
}

void FV3_(strev)::setspinlimit(fv3_float_t value) {
    spinlimit = limFs2(value);
    lfo1_lpf.setLPF_BW(spinlimit, getTotalSampleRate());
    lfo2_lpf.setLPF_BW(spinlimit, getTotalSampleRate());
}

fv3_float_t FV3_(strev)::getspinlimit() {
    return spinlimit;
}

void FV3_(strev)::setwander(fv3_float_t value) {
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    wander = value;
}

fv3_float_t FV3_(strev)::getwander() {
    return wander;
}

void FV3_(strev)::setmodulationnoise1(fv3_float_t value) {
    modnoise1 = value;
}

fv3_float_t FV3_(strev)::getmodulationnoise1() {
    return modnoise1;
}

void FV3_(strev)::setmodulationnoise2(fv3_float_t value) {
    modnoise2 = value;
}

fv3_float_t FV3_(strev)::getmodulationnoise2() {
    return modnoise2;
}

void FV3_(strev)::setinputdamp(fv3_float_t value) {
    inputdamp = limFs2(value);
    lpf_in.setLPF_BW(inputdamp, getTotalSampleRate());
}

fv3_float_t FV3_(strev)::getinputdamp() {
    return inputdamp;
}

void FV3_(strev)::setdamp(fv3_float_t value) {
    damp = limFs2(value);
    lpfC_30.setLPF_BW(damp, getTotalSampleRate());
    lpfC_54.setLPF_BW(damp, getTotalSampleRate());
}

fv3_float_t FV3_(strev)::getdamp() {
    return damp;
}

void FV3_(strev)::setoutputdamp(fv3_float_t value) {
    outputdamp = limFs2(value);
    out1_lpf.setLPF_BW(outputdamp, getTotalSampleRate());
    out2_lpf.setLPF_BW(outputdamp, getTotalSampleRate());
}

fv3_float_t FV3_(strev)::getoutputdamp() {
    return outputdamp;
}

void FV3_(strev)::setAutoDiff(bool value) {
    autoDiff = value;
}

bool FV3_(strev)::getAutoDiff() {
    return autoDiff;
}

void FV3_(strev)::setFsFactors() {
    FV3_(revbase)::setFsFactors();

    fv3_float_t totalFactor = getTotalFactorFs() / (fv3_float_t) FV3_STREV_DEFAULT_FS;
    fv3_float_t excurFactor = getTotalSampleRate() / (fv3_float_t) FV3_STREV_DEFAULT_FS;

    uint32_t totalMem1 = 0;
    for (int32_t i = 0; i < FV3_STREV_NUM_ALLPASS_4; i++) totalMem1 += f_(allpCo[i], totalFactor);
    totalMem1 += f_(allpCo[6], totalFactor);
    totalMem1 += f_(allpCo[7], totalFactor);

    uint32_t totalMem2 = 0;
    totalMem2 += f_(allpCo[4], totalFactor) + f_(allpM_EXCURSION, excurFactor);
    totalMem2 += f_(allpCo[5], totalFactor) + f_(allpM_EXCURSION, excurFactor);
    totalMem2 += f_(delayCo[0], totalFactor);
    totalMem2 += f_(delayCo[1], totalFactor);
    totalMem2 += f_(delayCo[2], totalFactor);
    totalMem2 += f_(delayCo[3], totalFactor);

#ifndef TBD_SIM
    if(totalMem2 >= totalMem1){
        ESP_LOGE("STREV", "Trying mem alloc!");
        totalBuf2 = (float*) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if(totalBuf2 == NULL){
            ESP_LOGE("STREV", "Cannot alloc memm trying SPIRAM!");
            totalBuf2 = (float*) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_SPIRAM);
            if(totalBuf2 == NULL) {
                ESP_LOGE("STREV", "Cannot alloc mem on SPIRAM!");
                totalBuf2 = (float *) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_SPIRAM);
                return;
            }
        }else{
            ESP_LOGE("STREV", "Mem alloc success requested size %d, freesize %d, largest block %d!",
                     (int)(totalMem2 * sizeof(float)), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
        }
        totalBuf1 = (float*) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if(totalBuf1 == NULL){
            ESP_LOGE("STREV", "Cannot alloc memm trying SPIRAM!");
            totalBuf1 = (float*) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_SPIRAM);
            if(totalBuf1 == NULL) {
                ESP_LOGE("STREV", "Cannot alloc mem on SPIRAM!");
                totalBuf1 = (float *) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_SPIRAM);
                return;
            }
        }else{
            ESP_LOGE("STREV", "Mem alloc success requested size %d, freesize %d, largest block %d!",
                     (int)(totalMem1 * sizeof(float)), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
        }
    }else{
        ESP_LOGE("STREV", "Trying mem alloc!");
        totalBuf1 = (float*) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if(totalBuf1 == NULL){
            ESP_LOGE("STREV", "Cannot alloc memm trying SPIRAM!");
            totalBuf1 = (float*) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_SPIRAM);
            if(totalBuf1 == NULL) {
                ESP_LOGE("STREV", "Cannot alloc mem on SPIRAM!");
                totalBuf1 = (float *) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_SPIRAM);
                return;
            }
        }else{
            ESP_LOGE("STREV", "Mem alloc success requested size %d, freesize %d, largest block %d!",
                     (int)(totalMem1 * sizeof(float)), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
        }
        totalBuf2 = (float*) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if(totalBuf2 == NULL){
            ESP_LOGE("STREV", "Cannot alloc memm trying SPIRAM!");
            totalBuf2 = (float*) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_SPIRAM);
            if(totalBuf2 == NULL) {
                ESP_LOGE("STREV", "Cannot alloc mem on SPIRAM!");
                totalBuf2 = (float *) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_SPIRAM);
                return;
            }
        }else{
            ESP_LOGE("STREV", "Mem alloc success requested size %d, freesize %d, largest block %d!",
                     (int)(totalMem2 * sizeof(float)), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
        }
    }
#else
    totalBuf1 = (float*) heap_caps_malloc(totalMem1 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    totalBuf2 = (float*) heap_caps_malloc(totalMem2 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#endif


    float *fPtr = totalBuf1;
    for (int32_t i = 0; i < FV3_STREV_NUM_ALLPASS_4; i++){
        allpassC[i].setsize(fPtr, f_(allpCo[i], totalFactor));
        fPtr += f_(allpCo[i], totalFactor);
    }
    allpassC_31_33.setsize(fPtr, f_(allpCo[6], totalFactor));
    fPtr += f_(allpCo[6], totalFactor);
    allpassC_55_59.setsize(fPtr, f_(allpCo[7], totalFactor));

    fPtr = totalBuf2;
    allpassM_23_24.setsize(fPtr, f_(allpCo[4], totalFactor), f_(allpM_EXCURSION, excurFactor));
    fPtr += f_(allpCo[4], totalFactor) + f_(allpM_EXCURSION, excurFactor);
    allpassM_46_48.setsize(fPtr, f_(allpCo[5], totalFactor), f_(allpM_EXCURSION, excurFactor));
    fPtr += f_(allpCo[5], totalFactor) + f_(allpM_EXCURSION, excurFactor);

    delayC_30.setsize(fPtr, f_(delayCo[0], totalFactor));
    fPtr += f_(delayCo[0], totalFactor);
    delayC_39.setsize(fPtr, f_(delayCo[1], totalFactor));
    fPtr += f_(delayCo[1], totalFactor);
    delayC_54.setsize(fPtr, f_(delayCo[2], totalFactor));
    fPtr += f_(delayCo[2], totalFactor);
    delayC_63.setsize(fPtr, f_(delayCo[3], totalFactor));
    for (int32_t i = 0; i < FV3_STREV_NUM_INDEX; i++) {
        iLC[i] = f_(idxLCo[i], totalFactor);
        iRC[i] = f_(idxRCo[i], totalFactor);
    }
    tankDelay = (delayC_30.getsize() + delayC_39.getsize() + delayC_54.getsize() + delayC_63.getsize()) / 4;
    setrt60(getrt60());
    setdccutfreq(getdccutfreq());
    setidiffusion1(getidiffusion1());
    setidiffusion2(getidiffusion2());
    setdiffusion1(getdiffusion1());
    setdiffusion2(getdiffusion2());
    setinputdamp(getinputdamp());
    setdamp(getdamp());
    setoutputdamp(getoutputdamp());
    setspin(getspin());
    setspindiff(getspindiff());
    setspinlimit(getspinlimit());
    setwander(getwander());
}

#include "fv3_ns_end.h"
