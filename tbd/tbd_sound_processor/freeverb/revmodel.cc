// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include <cassert>
#include "revmodel.hpp"

/*
// Buffers for the combs
float	DRAM_ATTR bufcombL1[combtuningL1];
float	DRAM_ATTR bufcombR1[combtuningR1];
float	DRAM_ATTR bufcombL2[combtuningL2];
float	DRAM_ATTR bufcombR2[combtuningR2];
float	DRAM_ATTR bufcombL3[combtuningL3];
float	DRAM_ATTR bufcombR3[combtuningR3];
float	DRAM_ATTR bufcombL4[combtuningL4];
float	DRAM_ATTR bufcombR4[combtuningR4];
float	DRAM_ATTR bufcombL5[combtuningL5];
float	DRAM_ATTR bufcombR5[combtuningR5];
float	DRAM_ATTR bufcombL6[combtuningL6];
float	DRAM_ATTR bufcombR6[combtuningR6];
float	DRAM_ATTR bufcombL7[combtuningL7];
float	DRAM_ATTR bufcombR7[combtuningR7];
float	DRAM_ATTR bufcombL8[combtuningL8];
float	DRAM_ATTR bufcombR8[combtuningR8];

// Buffers for the allpasses
float	DRAM_ATTR bufallpassL1[allpasstuningL1];
float	DRAM_ATTR bufallpassR1[allpasstuningR1];
float	DRAM_ATTR bufallpassL2[allpasstuningL2];
float	DRAM_ATTR bufallpassR2[allpasstuningR2];
float	DRAM_ATTR bufallpassL3[allpasstuningL3];
float	DRAM_ATTR bufallpassR3[allpasstuningR3];
float	DRAM_ATTR bufallpassL4[allpasstuningL4];
float	DRAM_ATTR bufallpassR4[allpasstuningR4];
 */

revmodel::revmodel() {
}

void revmodel::mute() {
    if (getmode() >= freezemode)
        return;

    for (int i = 0; i < numcombs; i++) {
        combL[i].mute();
        combR[i].mute();
    }
    for (int i = 0; i < numallpasses; i++) {
        allpassL[i].mute();
        allpassR[i].mute();
    }
}

void revmodel::processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip) {
    float outL, outR, input;

    while (numsamples-- > 0) {
        outL = outR = 0;
        input = (*inputL + *inputR) * gain;

        // Accumulate comb filters in parallel
        for (int i = 0; i < numcombs; i++) {
            outL += combL[i].process(input);
            outR += combR[i].process(input);
        }

        // Feed through allpasses in series
        for (int i = 0; i < numallpasses; i++) {
            outL = allpassL[i].process(outL);
            outR = allpassR[i].process(outR);
        }

        // Calculate output REPLACING anything already there
        *outputL = outL * wet1 + outR * wet2 + *inputL * dry;
        *outputR = outR * wet1 + outL * wet2 + *inputR * dry;

        // Increment sample pointers, allowing for interleave (if any)
        inputL += skip;
        inputR += skip;
        outputL += skip;
        outputR += skip;
    }
}

void revmodel::processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip) {
    float outL, outR, input;

    while (numsamples-- > 0) {
        outL = outR = 0;
        input = (*inputL + *inputR) * gain;

        // Accumulate comb filters in parallel
        for (int i = 0; i < numcombs; i++) {
            outL += combL[i].process(input);
            outR += combR[i].process(input);
        }

        // Feed through allpasses in series
        for (int i = 0; i < numallpasses; i++) {
            outL = allpassL[i].process(outL);
            outR = allpassR[i].process(outR);
        }

        // Calculate output MIXING with anything already there
        *outputL += outL * wet1 + outR * wet2 + *inputL * dry;
        *outputR += outR * wet1 + outL * wet2 + *inputR * dry;

        // Increment sample pointers, allowing for interleave (if any)
        inputL += skip;
        inputR += skip;
        outputL += skip;
        outputR += skip;
    }
}

void revmodel::update() {
// Recalculate internal values after parameter change

    int i;

    wet1 = wet * (width / 2 + 0.5f);
    wet2 = wet * ((1 - width) / 2);

    if (mode >= freezemode) {
        roomsize1 = 1.f;
        damp1 = 0.f;
        //gain = muted;
        gain = fixedgain;
    } else {
        roomsize1 = roomsize;
        damp1 = damp;
        gain = fixedgain;
    }

    for (i = 0; i < numcombs; i++) {
        combL[i].setfeedback(roomsize1);
        combR[i].setfeedback(roomsize1);
    }

    for (i = 0; i < numcombs; i++) {
        combL[i].setdamp(damp1);
        combR[i].setdamp(damp1);
    }
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value) {
    roomsize = (value * scaleroom) + offsetroom;
    update();
}

float revmodel::getroomsize() {
    return (roomsize - offsetroom) / scaleroom;
}

void revmodel::setdamp(float value) {
    damp = value * scaledamp;
    update();
}

float revmodel::getdamp() {
    return damp / scaledamp;
}

void revmodel::setwet(float value) {
    wet = value * scalewet;
    update();
}

float revmodel::getwet() {
    return wet / scalewet;
}

void revmodel::setdry(float value) {
    dry = value * scaledry;
}

float revmodel::getdry() {
    return dry / scaledry;
}

void revmodel::setwidth(float value) {
    width = value;
    update();
}

float revmodel::getwidth() {
    return width;
}

void revmodel::setmode(float value) {
    mode = value;
    update();
}

float revmodel::getmode() {
    if (mode >= freezemode)
        return 1;
    else
        return 0;
}

void  revmodel::process(float *data, uint32_t numsamples) {
    float outL, outR, input;

    for (uint32_t i = 0; i < numsamples; i++) {
        outL = outR = 0.f;
        if (isMono)
            input = data[i * 2] * gain;
        else
            input = (data[i * 2] + data[i * 2 + 1]) * gain;


        // Accumulate comb filters in parallel
        for (int j = 0; j < numcombs; j++) {
            outL += combL[j].process(input);
            outR += combR[j].process(input);
        }

        // Feed through allpasses in series
        for (int j = 0; j < numallpasses; j++) {
            outL = allpassL[j].process(outL);
            outR = allpassR[j].process(outR);
        }

        // Calculate output MIXING with anything already there
        data[i * 2] = outL * wet1 + outR * wet2 + data[i * 2] * dry;
        if (isMono) {
            data[i * 2 + 1] = outR * wet1 + outL * wet2 + data[i * 2] * dry;
        } else {
            data[i * 2 + 1] = outR * wet1 + outL * wet2 + data[i * 2 + 1] * dry;
        }

    }
}

revmodel::~revmodel() {
}

void revmodel::init(int memSize, void *memPtr) {
    uint32_t sz = combtuningL1 + combtuningR1 +
                  combtuningL2 + combtuningR2 +
                  combtuningL3 + combtuningR3 +
                  combtuningL4 + combtuningR4 +
                  combtuningL5 + combtuningR5 +
                  combtuningL6 + combtuningR6 +
                  combtuningL7 + combtuningR7 +
                  combtuningL8 + combtuningR8 +
                  allpasstuningL1 + allpasstuningR1 +
                  allpasstuningL2 + allpasstuningR2 +
                  allpasstuningL3 + allpasstuningR3 +
                  allpasstuningL4 + allpasstuningR4;

    assert(memSize >= sz * sizeof(float));
    allBuf = (float *) memPtr;

    float *fPtr = allBuf;
    // Tie the components to their buffers
    combL[0].setbuffer(fPtr, combtuningL1);
    fPtr += combtuningL1;
    combR[0].setbuffer(fPtr, combtuningR1);
    fPtr += combtuningR1;
    combL[1].setbuffer(fPtr, combtuningL2);
    fPtr += combtuningL2;
    combR[1].setbuffer(fPtr, combtuningR2);
    fPtr += combtuningR2;
    combL[2].setbuffer(fPtr, combtuningL3);
    fPtr += combtuningL3;
    combR[2].setbuffer(fPtr, combtuningR3);
    fPtr += combtuningR3;
    combL[3].setbuffer(fPtr, combtuningL4);
    fPtr += combtuningL4;
    combR[3].setbuffer(fPtr, combtuningR4);
    fPtr += combtuningR4;
    combL[4].setbuffer(fPtr, combtuningL5);
    fPtr += combtuningL5;
    combR[4].setbuffer(fPtr, combtuningR5);
    fPtr += combtuningR5;
    combL[5].setbuffer(fPtr, combtuningL6);
    fPtr += combtuningL6;
    combR[5].setbuffer(fPtr, combtuningR6);
    fPtr += combtuningR6;
    combL[6].setbuffer(fPtr, combtuningL7);
    fPtr += combtuningL7;
    combR[6].setbuffer(fPtr, combtuningR7);
    fPtr += combtuningR7;
    combL[7].setbuffer(fPtr, combtuningL8);
    fPtr += combtuningL8;
    combR[7].setbuffer(fPtr, combtuningR8);
    fPtr += combtuningR8;
    allpassL[0].setbuffer(fPtr, allpasstuningL1);
    fPtr += allpasstuningL1;
    allpassR[0].setbuffer(fPtr, allpasstuningR1);
    fPtr += allpasstuningR1;
    allpassL[1].setbuffer(fPtr, allpasstuningL2);
    fPtr += allpasstuningL2;
    allpassR[1].setbuffer(fPtr, allpasstuningR2);
    fPtr += allpasstuningR2;
    allpassL[2].setbuffer(fPtr, allpasstuningL3);
    fPtr += allpasstuningL3;
    allpassR[2].setbuffer(fPtr, allpasstuningR3);
    fPtr += allpasstuningR3;
    allpassL[3].setbuffer(fPtr, allpasstuningL4);
    fPtr += allpasstuningL4;
    allpassR[3].setbuffer(fPtr, allpasstuningR4);

    // Set default values
    allpassL[0].setfeedback(0.5f);
    allpassR[0].setfeedback(0.5f);
    allpassL[1].setfeedback(0.5f);
    allpassR[1].setfeedback(0.5f);
    allpassL[2].setfeedback(0.5f);
    allpassR[2].setfeedback(0.5f);
    allpassL[3].setfeedback(0.5f);
    allpassR[3].setfeedback(0.5f);
    setwet(initialwet);
    setroomsize(initialroom);
    setdry(initialdry);
    setdamp(initialdamp);
    setwidth(initialwidth);
    setmode(initialmode);

    // Buffer will be full of rubbish - so we MUST mute them
    mute();
}

//ends
