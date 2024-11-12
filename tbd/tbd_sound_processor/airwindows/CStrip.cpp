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

/* based on CStrip by Chris Johnson, see LICENSE file */
/* ported to CTAG TBD by Christian Feyer */

#include "CStrip.hpp"
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP::HELPERS;


namespace airwindows {
    void CStrip::Process(float *buf, int bufSz) {

        float overallscale = 1.0f;
        overallscale /= 44100.0f;
        float compscale = overallscale;
        overallscale = 44100.0f;
        compscale = compscale * overallscale;

        //compscale is the one that's 1 or something like 2.2 for 96K rates
        float fpOld = 0.618033988749894848204586f; //golden ratio!
        float fpNew = 1.0f - fpOld;
        float inputSampleL;
        float inputSampleR;

        float highSampleL = 0.0f;
        float midSampleL = 0.0f;
        float bassSampleL = 0.0f;

        float highSampleR = 0.0f;
        float midSampleR = 0.0f;
        float bassSampleR = 0.0f;

        float densityA = (A*12.0f)-6.0f;
        float densityB = (B*12.0f)-6.0f;
        float densityC = (C*12.0f)-6.0f;
        bool engageEQ = true;
        if ( (0.0f == densityA) && (0.0f == densityB) && (0.0f == densityC) ) engageEQ = false;

        densityA = powf_fast_precise(10.0f,densityA/20.0f)-1.0f;
        densityB = powf_fast_precise(10.0f,densityB/20.0f)-1.0f;
        densityC = powf_fast_precise(10.0f,densityC/20.0f)-1.0f;
        //convert to 0 to X multiplier with 1.0 being O db
        //minus one gives nearly -1 to ? (should top out at 1)
        //calibrate so that X db roughly equals X db with maximum topping out at 1 internally

        float tripletIntensity = -densityA;

        float iirAmountC = (((D*D*15.0f)+1.0f)*0.0188f) + 0.7f;
        if (iirAmountC > 1.0f) iirAmountC = 1.0f;
        bool engageLowpass = false;
        if (((D*D*15.0f)+1.0f) < 15.99f) engageLowpass = true;

        float iirAmountA = (((E*E*15.0f)+1.0f)*1000.f)/overallscale;
        float iirAmountB = (((F*F*1570.0f)+30.0f)*10.f)/overallscale;
        float iirAmountD = (((G*G*1570.0f)+30.0f)*1.0f)/overallscale;
        bool engageHighpass = false;
        if (((G*G*1570.0f)+30.0f) > 30.01f) engageHighpass = true;
        //bypass the highpass and lowpass if set to extremes
        float bridgerectifier;
        float outA = fabsf(densityA);
        float outB = fabsf(densityB);
        float outC = fabsf(densityC);
        //end EQ
        //begin Gate
        float onthreshold = (powf_fast_precise(H,4.f)/3.f)+0.00018f;
        float offthreshold = onthreshold * 1.1f;
        bool engageGate = false;
        if (onthreshold > 0.00018f) engageGate = true;

        float release = 0.028331119964586f;
        float absmax = 220.9f;
        //speed to be compensated w.r.t sample rate
        //end Gate
        //begin Timing
        float offset = powf_fast_precise(K,5.f) * 700.f;
        int near = (int)floorf(fabsf(offset));
        float farLevel = fabsf(offset) - near;
        int far = near + 1;
        float nearLevel = 1.0f - farLevel;
        bool engageTiming = false;
        if (offset > 0.0f) engageTiming = true;
        //end Timing
        //begin ButterComp
        float inputpos = 0.f;
        float inputneg = 0.f;
        float calcpos = 0.f;
        float calcneg = 0.f;
        float outputpos = 0.f;
        float outputneg = 0.f;
        float totalmultiplier = 0.f;
        float inputgain = (powf_fast_precise(I,4.0f)*35.f)+1.0f;   //compressor value
        float compoutgain = inputgain;
        compoutgain -= 1.0f;
        compoutgain /= 1.2f;
        compoutgain += 1.0f;
        float divisor = (0.008f * powf_fast_precise(J,2.f))+0.0004f;
        //originally 0.012
        divisor /= compscale;
        float remainder = divisor;
        divisor = 1.0f - divisor;
        bool engageComp = false;
        if (inputgain > 1.0f) engageComp = true;
        //end ButterComp
        float outputgain = powf_fast_precise(10.0f,((L*36.0f)-18.0f)/20.0f);


        for(int i=0;i<bufSz;i++) {
            {
                inputSampleL = buf[i * 2];   //Channel 0
                inputSampleR = buf[i * 2 + 1];   //Channel 1
/*
        if (inputSampleL<1.2e-38f && -inputSampleL<1.2e-38f) {
            static int noisesource = 0;
            //this declares a variable before anything else is compiled. It won't keep assigning
            //it to 0 for every sample, it's as if the declaration doesn't exist in this context,
            //but it lets me add this denormalization fix in a single place rather than updating
            //it in three different locations. The variable isn't thread-safe but this is only
            //a random seed and we can share it with whatever.
            noisesource = noisesource % 1700021; noisesource++;
            int residue = noisesource * noisesource;
            residue = residue % 170003; residue *= residue;
            residue = residue % 17011; residue *= residue;
            residue = residue % 1709; residue *= residue;
            residue = residue % 173; residue *= residue;
            residue = residue % 17;
            float applyresidue = residue;
            applyresidue *= 0.00000001f;
            applyresidue *= 0.00000001f;
            inputSampleL = applyresidue;
        }
        if (inputSampleR<1.2e-38f && -inputSampleR<1.2e-38f) {
            static int noisesource = 0;
            noisesource = noisesource % 1700021; noisesource++;
            int residue = noisesource * noisesource;
            residue = residue % 170003; residue *= residue;
            residue = residue % 17011; residue *= residue;
            residue = residue % 1709; residue *= residue;
            residue = residue % 173; residue *= residue;
            residue = residue % 17;
            float applyresidue = residue;
            applyresidue *= 0.00000001f;
            applyresidue *= 0.00000001f;
            inputSampleR = applyresidue;
            //this denormalization routine produces a white noise at -300 dB which the noise
            //shaping will interact with to produce a bipolar output, but the noise is actually
            //all positive. That should stop any variables from going denormal, and the routine
            //only kicks in if digital black is input. As a final touch, if you save to 24-bit
            //the silence will return to being digital black again.
        }
*/
                last2SampleL = lastSampleL;
                lastSampleL = inputSampleL;

                last2SampleR = lastSampleR;
                lastSampleR = inputSampleR;

                //begin Gate
                if (engageGate)
                {
                    if (inputSampleL > 0)
                    {if (WasNegativeL == true){ZeroCrossL = absmax * 0.3f;}
                        WasNegativeL = false;}
                    else
                    {ZeroCrossL += 1; WasNegativeL = true;}

                    if (inputSampleR > 0)
                    {if (WasNegativeR == true){ZeroCrossR = absmax * 0.3f;}
                        WasNegativeR = false;}
                    else
                    {ZeroCrossR += 1; WasNegativeR = true;}

                    if (ZeroCrossL > absmax)
                    {ZeroCrossL = absmax;}

                    if (ZeroCrossR > absmax)
                    {ZeroCrossR = absmax;}

                    if (gateL == 0.0f)
                    {
                        //if gate is totally silent
                        if (fabsf(inputSampleL) > onthreshold)
                        {
                            if (gaterollerL == 0.0f) gaterollerL = ZeroCrossL;
                            else gaterollerL -= release;
                            // trigger from total silence only- if we're active then signal must clear offthreshold
                        }
                        else gaterollerL -= release;
                    }
                    else
                    {
                        //gate is not silent but closing
                        if (fabsf(inputSampleL) > offthreshold)
                        {
                            if (gaterollerL < ZeroCrossL) gaterollerL = ZeroCrossL;
                            else gaterollerL -= release;
                            //always trigger if gate is over offthreshold, otherwise close anyway
                        }
                        else gaterollerL -= release;
                    }

                    if (gateR == 0.0f)
                    {
                        //if gate is totally silent
                        if (fabsf(inputSampleR) > onthreshold)
                        {
                            if (gaterollerR == 0.0f) gaterollerR = ZeroCrossR;
                            else gaterollerR -= release;
                            // trigger from total silence only- if we're active then signal must clear offthreshold
                        }
                        else gaterollerR -= release;
                    }
                    else
                    {
                        //gate is not silent but closing
                        if (fabsf(inputSampleR) > offthreshold)
                        {
                            if (gaterollerR < ZeroCrossR) gaterollerR = ZeroCrossR;
                            else gaterollerR -= release;
                            //always trigger if gate is over offthreshold, otherwise close anyway
                        }
                        else gaterollerR -= release;
                    }

                    if (gaterollerL < 0.0f)
                    {gaterollerL = 0.0f;}
                    if (gaterollerR < 0.0f)
                    {gaterollerR = 0.0f;}

                    if (gaterollerL < 1.0f)
                    {
                        gateL = gaterollerL;
                        bridgerectifier = 1-fastcos(fabsf(inputSampleL));
                        if (inputSampleL > 0) inputSampleL = (inputSampleL*gateL)+(bridgerectifier*(1.0f-gateL));
                        else inputSampleL = (inputSampleL*gateL)-(bridgerectifier*(1.0f-gateL));
                        if (gateL == 0.0f) inputSampleL = 0.0f;
                    }
                    else
                    {gateL = 1.0f;}

                    if (gaterollerR < 1.0f)
                    {
                        gateR = gaterollerR;
                        bridgerectifier = 1-fastcos(fabsf(inputSampleR));
                        if (inputSampleR > 0) inputSampleR = (inputSampleR*gateR)+(bridgerectifier*(1.0f-gateR));
                        else inputSampleR = (inputSampleR*gateR)-(bridgerectifier*(1.0f-gateR));
                        if (gateR == 0.0f) inputSampleR = 0.0f;
                    }
                    else
                    {gateR = 1.0f;}
                }
                //end Gate, begin antialiasing

                flip = !flip;
                flipthree++;
                if (flipthree < 1 || flipthree > 3) flipthree = 1;
                //counters

                //begin highpass
                if (engageHighpass)
                {
                    if (flip)
                    {
                        highpassSampleLAA = (highpassSampleLAA * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLAA;
                        highpassSampleLBA = (highpassSampleLBA * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLBA;
                        highpassSampleLCA = (highpassSampleLCA * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLCA;
                        highpassSampleLDA = (highpassSampleLDA * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLDA;
                    }
                    else
                    {
                        highpassSampleLAB = (highpassSampleLAB * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLAB;
                        highpassSampleLBB = (highpassSampleLBB * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLBB;
                        highpassSampleLCB = (highpassSampleLCB * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLCB;
                        highpassSampleLDB = (highpassSampleLDB * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                        inputSampleL -= highpassSampleLDB;
                    }
                    highpassSampleLE = (highpassSampleLE * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                    inputSampleL -= highpassSampleLE;
                    highpassSampleLF = (highpassSampleLF * (1.0f - iirAmountD)) + (inputSampleL * iirAmountD);
                    inputSampleL -= highpassSampleLF;

                    if (flip)
                    {
                        highpassSampleRAA = (highpassSampleRAA * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRAA;
                        highpassSampleRBA = (highpassSampleRBA * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRBA;
                        highpassSampleRCA = (highpassSampleRCA * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRCA;
                        highpassSampleRDA = (highpassSampleRDA * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRDA;
                    }
                    else
                    {
                        highpassSampleRAB = (highpassSampleRAB * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRAB;
                        highpassSampleRBB = (highpassSampleRBB * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRBB;
                        highpassSampleRCB = (highpassSampleRCB * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRCB;
                        highpassSampleRDB = (highpassSampleRDB * (1.0f - iirAmountD)) + (inputSampleR * iirAmountD);
                        inputSampleR -= highpassSampleRDB;
                    }
                    highpassSampleRE = (highpassSampleRE * (1.f - iirAmountD)) + (inputSampleR * iirAmountD);
                    inputSampleR -= highpassSampleRE;
                    highpassSampleRF = (highpassSampleRF * (1.f - iirAmountD)) + (inputSampleR * iirAmountD);
                    inputSampleR -= highpassSampleRF;

                }
                //end highpass

                //begin compressor
                if (engageComp)
                {
                    //begin L
                    inputSampleL *= inputgain;

                    inputpos = (inputSampleL * fpOld) + (avgLA * fpNew) + 1.0f;
                    avgLA = inputSampleL;

                    if (inputpos < 0.0f) inputpos = 0.0f;
                    outputpos = inputpos / 2.0f;
                    if (outputpos > 1.0f) outputpos = 1.0f;
                    inputpos *= inputpos;
                    targetposL *= divisor;
                    targetposL += (inputpos * remainder);
                    calcpos = powf_fast_precise((1.0f/targetposL),2.f);

                    inputneg = (-inputSampleL * fpOld) + (nvgLA * fpNew) + 1.0f;
                    nvgLA = -inputSampleL;

                    if (inputneg < 0.0f) inputneg = 0.0f;
                    outputneg = inputneg / 2.0f;
                    if (outputneg > 1.0f) outputneg = 1.0f;
                    inputneg *= inputneg;
                    targetnegL *= divisor;
                    targetnegL += (inputneg * remainder);
                    calcneg = powf_fast_precise((1.0f/targetnegL),2.f);
                    //now we have mirrored targets for comp
                    //outputpos and outputneg go from 0 to 1

                    if (inputSampleL > 0.f)
                    { //working on pos
                        if (true == flip)
                        {
                            controlAposL *= divisor;
                            controlAposL += (calcpos*remainder);

                        }
                        else
                        {
                            controlBposL *= divisor;
                            controlBposL += (calcpos*remainder);
                        }
                    }
                    else
                    { //working on neg
                        if (true == flip)
                        {
                            controlAnegL *= divisor;
                            controlAnegL += (calcneg*remainder);
                        }
                        else
                        {
                            controlBnegL *= divisor;
                            controlBnegL += (calcneg*remainder);
                        }
                    }
                    //this causes each of the four to update only when active and in the correct 'flip'

                    if (true == flip)
                    {totalmultiplier = (controlAposL * outputpos) + (controlAnegL * outputneg);}
                    else
                    {totalmultiplier = (controlBposL * outputpos) + (controlBnegL * outputneg);}
                    //this combines the sides according to flip, blending relative to the input value

                    inputSampleL *= totalmultiplier;
                    inputSampleL /= compoutgain;
                    //end L

                    //begin R
                    inputSampleR *= inputgain;

                    inputpos = (inputSampleR * fpOld) + (avgRA * fpNew) + 1.0f;
                    avgRA = inputSampleR;

                    if (inputpos < 0.0f) inputpos = 0.0f;
                    outputpos = inputpos / 2.0f;
                    if (outputpos > 1.0f) outputpos = 1.0f;
                    inputpos *= inputpos;
                    targetposR *= divisor;
                    targetposR += (inputpos * remainder);
                    calcpos = powf_fast_precise((1.0f/targetposR),2.f);

                    inputneg = (-inputSampleR * fpOld) + (nvgRA * fpNew) + 1.0f;
                    nvgRA = -inputSampleR;

                    if (inputneg < 0.0f) inputneg = 0.0f;
                    outputneg = inputneg / 2.0f;
                    if (outputneg > 1.0f) outputneg = 1.0f;
                    inputneg *= inputneg;
                    targetnegR *= divisor;
                    targetnegR += (inputneg * remainder);
                    calcneg = powf_fast_precise((1.0f/targetnegR),2.f);
                    //now we have mirrored targets for comp
                    //outputpos and outputneg go from 0 to 1

                    if (inputSampleR > 0.f)
                    { //working on pos
                        if (true == flip)
                        {
                            controlAposR *= divisor;
                            controlAposR += (calcpos*remainder);

                        }
                        else
                        {
                            controlBposR *= divisor;
                            controlBposR += (calcpos*remainder);
                        }
                    }
                    else
                    { //working on neg
                        if (true == flip)
                        {
                            controlAnegR *= divisor;
                            controlAnegR += (calcneg*remainder);
                        }
                        else
                        {
                            controlBnegR *= divisor;
                            controlBnegR += (calcneg*remainder);
                        }
                    }
                    //this causes each of the four to update only when active and in the correct 'flip'

                    if (true == flip)
                    {totalmultiplier = (controlAposR * outputpos) + (controlAnegR * outputneg);}
                    else
                    {totalmultiplier = (controlBposR * outputpos) + (controlBnegR * outputneg);}
                    //this combines the sides according to flip, blending relative to the input value

                    inputSampleR *= totalmultiplier;
                    inputSampleR /= compoutgain;
                    //end R
                }
                //end compressor

                //begin EQ
                if (engageEQ)
                {
                    switch (flipthree)
                    {
                        case 1:
                            tripletFactorL = last2SampleL - inputSampleL;
                            tripletLA += tripletFactorL;
                            tripletLC -= tripletFactorL;
                            tripletFactorL = tripletLA * tripletIntensity;
                            iirHighSampleLC = (iirHighSampleLC * (1.0f - iirAmountA)) + (inputSampleL * iirAmountA);
                            highSampleL = inputSampleL - iirHighSampleLC;
                            iirLowSampleLC = (iirLowSampleLC * (1.0f - iirAmountB)) + (inputSampleL * iirAmountB);
                            bassSampleL = iirLowSampleLC;

                            tripletFactorR = last2SampleR - inputSampleR;
                            tripletRA += tripletFactorR;
                            tripletRC -= tripletFactorR;
                            tripletFactorR = tripletRA * tripletIntensity;
                            iirHighSampleRC = (iirHighSampleRC * (1.0f - iirAmountA)) + (inputSampleR * iirAmountA);
                            highSampleR = inputSampleR - iirHighSampleRC;
                            iirLowSampleRC = (iirLowSampleRC * (1.0f - iirAmountB)) + (inputSampleR * iirAmountB);
                            bassSampleR = iirLowSampleRC;
                            break;
                        case 2:
                            tripletFactorL = last2SampleL - inputSampleL;
                            tripletLB += tripletFactorL;
                            tripletLA -= tripletFactorL;
                            tripletFactorL = tripletLB * tripletIntensity;
                            iirHighSampleLD = (iirHighSampleLD * (1.0f - iirAmountA)) + (inputSampleL * iirAmountA);
                            highSampleL = inputSampleL - iirHighSampleLD;
                            iirLowSampleLD = (iirLowSampleLD * (1.0f - iirAmountB)) + (inputSampleL * iirAmountB);
                            bassSampleL = iirLowSampleLD;

                            tripletFactorR = last2SampleR - inputSampleR;
                            tripletRB += tripletFactorR;
                            tripletRA -= tripletFactorR;
                            tripletFactorR = tripletRB * tripletIntensity;
                            iirHighSampleRD = (iirHighSampleRD * (1.0f - iirAmountA)) + (inputSampleR * iirAmountA);
                            highSampleR = inputSampleR - iirHighSampleRD;
                            iirLowSampleRD = (iirLowSampleRD * (1.0f - iirAmountB)) + (inputSampleR * iirAmountB);
                            bassSampleR = iirLowSampleRD;
                            break;
                        case 3:
                            tripletFactorL = last2SampleL - inputSampleL;
                            tripletLC += tripletFactorL;
                            tripletLB -= tripletFactorL;
                            tripletFactorL = tripletLC * tripletIntensity;
                            iirHighSampleLE = (iirHighSampleLE * (1.0f - iirAmountA)) + (inputSampleL * iirAmountA);
                            highSampleL = inputSampleL - iirHighSampleLE;
                            iirLowSampleLE = (iirLowSampleLE * (1.0f - iirAmountB)) + (inputSampleL * iirAmountB);
                            bassSampleL = iirLowSampleLE;

                            tripletFactorR = last2SampleR - inputSampleR;
                            tripletRC += tripletFactorR;
                            tripletRB -= tripletFactorR;
                            tripletFactorR = tripletRC * tripletIntensity;
                            iirHighSampleRE = (iirHighSampleRE * (1.0f - iirAmountA)) + (inputSampleR * iirAmountA);
                            highSampleR = inputSampleR - iirHighSampleRE;
                            iirLowSampleRE = (iirLowSampleRE * (1.0f - iirAmountB)) + (inputSampleR * iirAmountB);
                            bassSampleR = iirLowSampleRE;
                            break;
                    }
                    tripletLA /= 2.0f;
                    tripletLB /= 2.0f;
                    tripletLC /= 2.0f;
                    highSampleL = highSampleL + tripletFactorL;

                    tripletRA /= 2.0f;
                    tripletRB /= 2.0f;
                    tripletRC /= 2.0f;
                    highSampleR = highSampleR + tripletFactorR;

                    if (flip)
                    {
                        iirHighSampleLA = (iirHighSampleLA * (1.0f - iirAmountA)) + (highSampleL * iirAmountA);
                        highSampleL -= iirHighSampleLA;
                        iirLowSampleLA = (iirLowSampleLA * (1.0f - iirAmountB)) + (bassSampleL * iirAmountB);
                        bassSampleL = iirLowSampleLA;

                        iirHighSampleRA = (iirHighSampleRA * (1.0f - iirAmountA)) + (highSampleR * iirAmountA);
                        highSampleR -= iirHighSampleRA;
                        iirLowSampleRA = (iirLowSampleRA * (1.0f - iirAmountB)) + (bassSampleR * iirAmountB);
                        bassSampleR = iirLowSampleRA;
                    }
                    else
                    {
                        iirHighSampleLB = (iirHighSampleLB * (1.0f - iirAmountA)) + (highSampleL * iirAmountA);
                        highSampleL -= iirHighSampleLB;
                        iirLowSampleLB = (iirLowSampleLB * (1.0f - iirAmountB)) + (bassSampleL * iirAmountB);
                        bassSampleL = iirLowSampleLB;

                        iirHighSampleRB = (iirHighSampleRB * (1.0f - iirAmountA)) + (highSampleR * iirAmountA);
                        highSampleR -= iirHighSampleRB;
                        iirLowSampleRB = (iirLowSampleRB * (1.0f - iirAmountB)) + (bassSampleR * iirAmountB);
                        bassSampleR = iirLowSampleRB;
                    }

                    iirHighSampleL = (iirHighSampleL * (1.0f - iirAmountA)) + (highSampleL * iirAmountA);
                    highSampleL -= iirHighSampleL;
                    iirLowSampleL = (iirLowSampleL * (1.0f - iirAmountB)) + (bassSampleL * iirAmountB);
                    bassSampleL = iirLowSampleL;

                    iirHighSampleR = (iirHighSampleR * (1.0f - iirAmountA)) + (highSampleR * iirAmountA);
                    highSampleR -= iirHighSampleR;
                    iirLowSampleR = (iirLowSampleR * (1.0f - iirAmountB)) + (bassSampleR * iirAmountB);
                    bassSampleR = iirLowSampleR;

                    midSampleL = (inputSampleL-bassSampleL)-highSampleL;
                    midSampleR = (inputSampleR-bassSampleR)-highSampleR;

                    //drive section
                    highSampleL *= (densityA+1.0f);
                    bridgerectifier = fabsf(highSampleL)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityA > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (highSampleL > 0) highSampleL = (highSampleL*(1-outA))+(bridgerectifier*outA);
                    else highSampleL = (highSampleL*(1-outA))-(bridgerectifier*outA);
                    //blend according to densityA control

                    highSampleR *= (densityA+1.0f);
                    bridgerectifier = fabsf(highSampleR)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityA > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (highSampleR > 0) highSampleR = (highSampleR*(1-outA))+(bridgerectifier*outA);
                    else highSampleR = (highSampleR*(1-outA))-(bridgerectifier*outA);
                    //blend according to densityA control

                    midSampleL *= (densityB+1.0f);
                    bridgerectifier = fabsf(midSampleL)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityB > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (midSampleL > 0) midSampleL = (midSampleL*(1-outB))+(bridgerectifier*outB);
                    else midSampleL = (midSampleL*(1-outB))-(bridgerectifier*outB);
                    //blend according to densityB control

                    midSampleR *= (densityB+1.0f);
                    bridgerectifier = fabsf(midSampleR)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityB > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (midSampleR > 0) midSampleR = (midSampleR*(1-outB))+(bridgerectifier*outB);
                    else midSampleR = (midSampleR*(1-outB))-(bridgerectifier*outB);
                    //blend according to densityB control

                    bassSampleL *= (densityC+1.0f);
                    bridgerectifier = fabsf(bassSampleL)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityC > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (bassSampleL > 0) bassSampleL = (bassSampleL*(1-outC))+(bridgerectifier*outC);
                    else bassSampleL = (bassSampleL*(1-outC))-(bridgerectifier*outC);
                    //blend according to densityC control

                    bassSampleR *= (densityC+1.0f);
                    bridgerectifier = fabsf(bassSampleR)*1.57079633f;
                    if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
                    //max value for sine function
                    if (densityC > 0) bridgerectifier = fastsin(bridgerectifier);
                    else bridgerectifier = 1-fastcos(bridgerectifier);
                    //produce either boosted or starved version
                    if (bassSampleR > 0) bassSampleR = (bassSampleR*(1-outC))+(bridgerectifier*outC);
                    else bassSampleR = (bassSampleR*(1-outC))-(bridgerectifier*outC);
                    //blend according to densityC control

                    inputSampleL = midSampleL;
                    inputSampleL += highSampleL;
                    inputSampleL += bassSampleL;

                    inputSampleR = midSampleR;
                    inputSampleR += highSampleR;
                    inputSampleR += bassSampleR;
                }
                //end EQ

                //begin Timing
                if (engageTiming == true)   //==
                {
                    if (count < 1 || count > 2048) count = 2048;

                    pL[count+2048] = pL[count] = inputSampleL;
                    pR[count+2048] = pR[count] = inputSampleR;

                    inputSampleL = pL[count+near]*nearLevel;
                    inputSampleR = pR[count+near]*nearLevel;

                    inputSampleL += pL[count+far]*farLevel;
                    inputSampleR += pR[count+far]*farLevel;

                    count -= 1;
                    //consider adding third sample just to bring out superhighs subtly, like old interpolation hacks
                    //or third and fifth samples, ditto
                }
                //end Timing

                //EQ lowpass is after all processing like the compressor that might produce hash
                if (engageLowpass)
                {
                    if (flip)
                    {
                        lowpassSampleLAA = (lowpassSampleLAA * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLAA;
                        lowpassSampleLBA = (lowpassSampleLBA * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLBA;
                        lowpassSampleLCA = (lowpassSampleLCA * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLCA;
                        lowpassSampleLDA = (lowpassSampleLDA * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLDA;
                        lowpassSampleLE = (lowpassSampleLE * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLE;

                        lowpassSampleRAA = (lowpassSampleRAA * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRAA;
                        lowpassSampleRBA = (lowpassSampleRBA * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRBA;
                        lowpassSampleRCA = (lowpassSampleRCA * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRCA;
                        lowpassSampleRDA = (lowpassSampleRDA * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRDA;
                        lowpassSampleRE = (lowpassSampleRE * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRE;
                    }
                    else
                    {
                        lowpassSampleLAB = (lowpassSampleLAB * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLAB;
                        lowpassSampleLBB = (lowpassSampleLBB * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLBB;
                        lowpassSampleLCB = (lowpassSampleLCB * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLCB;
                        lowpassSampleLDB = (lowpassSampleLDB * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLDB;
                        lowpassSampleLF = (lowpassSampleLF * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                        inputSampleL = lowpassSampleLF;

                        lowpassSampleRAB = (lowpassSampleRAB * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRAB;
                        lowpassSampleRBB = (lowpassSampleRBB * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRBB;
                        lowpassSampleRCB = (lowpassSampleRCB * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRCB;
                        lowpassSampleRDB = (lowpassSampleRDB * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRDB;
                        lowpassSampleRF = (lowpassSampleRF * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                        inputSampleR = lowpassSampleRF;
                    }
                    lowpassSampleLG = (lowpassSampleLG * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                    lowpassSampleRG = (lowpassSampleRG * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);

                    inputSampleL = (lowpassSampleLG * (1.0f - iirAmountC)) + (inputSampleL * iirAmountC);
                    inputSampleR = (lowpassSampleRG * (1.0f - iirAmountC)) + (inputSampleR * iirAmountC);
                }

                //built in output trim and dry/wet if desired
                if (outputgain != 1.0f) {
                    inputSampleL *= outputgain;
                    inputSampleR *= outputgain;
                }
/*
        //stereo 32 bit dither, made small and tidy.
        int expon; frexpf((float)inputSampleL, &expon);
         float dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
        inputSampleL += (dither-fpNShapeL); fpNShapeL = dither;
        frexpf((float)inputSampleR, &expon);
        dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
        inputSampleR += (dither-fpNShapeR); fpNShapeR = dither;
        //end 32 bit dither
*/
                buf[i * 2] = inputSampleL;
                buf[i * 2 + 1] = inputSampleR;

            }
        }
    }

    CStrip::CStrip() {
        A = 0.5; //Treble -12 to 12
        B = 0.5; //Mid -12 to 12
        C = 0.5; //Bass -12 to 12
        D = 1.0; //Lowpass 16.0K log 1 to 16 defaulting to 16K
        E = 0.4; //TrebFrq 6.0 log 1 to 16 defaulting to 6K
        F = 0.4; //BassFrq 100.0 log 30 to 1600 defaulting to 100 hz
        G = 0.0; //Hipass 30.0 log 30 to 1600 defaulting to 30
        H = 0.0; //Gate 0-1
        I = 0.0; //Compres 0-1
        J = 0.0; //CompSpd 0-1
        K = 0.0; //TimeLag 0-1
        L = 0.5; //OutGain -18 to 18

        lastSampleL = 0.0;
        last2SampleL = 0.0;
        lastSampleR = 0.0;
        last2SampleR = 0.0;

        iirHighSampleLA = 0.0;
        iirHighSampleLB = 0.0;
        iirHighSampleLC = 0.0;
        iirHighSampleLD = 0.0;
        iirHighSampleLE = 0.0;
        iirLowSampleLA = 0.0;
        iirLowSampleLB = 0.0;
        iirLowSampleLC = 0.0;
        iirLowSampleLD = 0.0;
        iirLowSampleLE = 0.0;
        iirHighSampleL = 0.0;
        iirLowSampleL = 0.0;

        iirHighSampleRA = 0.0;
        iirHighSampleRB = 0.0;
        iirHighSampleRC = 0.0;
        iirHighSampleRD = 0.0;
        iirHighSampleRE = 0.0;
        iirLowSampleRA = 0.0;
        iirLowSampleRB = 0.0;
        iirLowSampleRC = 0.0;
        iirLowSampleRD = 0.0;
        iirLowSampleRE = 0.0;
        iirHighSampleR = 0.0;
        iirLowSampleR = 0.0;

        tripletLA = 0.0;
        tripletLB = 0.0;
        tripletLC = 0.0;
        tripletFactorL = 0.0;

        tripletRA = 0.0;
        tripletRB = 0.0;
        tripletRC = 0.0;
        tripletFactorR = 0.0;

        lowpassSampleLAA = 0.0;
        lowpassSampleLAB = 0.0;
        lowpassSampleLBA = 0.0;
        lowpassSampleLBB = 0.0;
        lowpassSampleLCA = 0.0;
        lowpassSampleLCB = 0.0;
        lowpassSampleLDA = 0.0;
        lowpassSampleLDB = 0.0;
        lowpassSampleLE = 0.0;
        lowpassSampleLF = 0.0;
        lowpassSampleLG = 0.0;

        lowpassSampleRAA = 0.0;
        lowpassSampleRAB = 0.0;
        lowpassSampleRBA = 0.0;
        lowpassSampleRBB = 0.0;
        lowpassSampleRCA = 0.0;
        lowpassSampleRCB = 0.0;
        lowpassSampleRDA = 0.0;
        lowpassSampleRDB = 0.0;
        lowpassSampleRE = 0.0;
        lowpassSampleRF = 0.0;
        lowpassSampleRG = 0.0;

        highpassSampleLAA = 0.0;
        highpassSampleLAB = 0.0;
        highpassSampleLBA = 0.0;
        highpassSampleLBB = 0.0;
        highpassSampleLCA = 0.0;
        highpassSampleLCB = 0.0;
        highpassSampleLDA = 0.0;
        highpassSampleLDB = 0.0;
        highpassSampleLE = 0.0;
        highpassSampleLF = 0.0;

        highpassSampleRAA = 0.0;
        highpassSampleRAB = 0.0;
        highpassSampleRBA = 0.0;
        highpassSampleRBB = 0.0;
        highpassSampleRCA = 0.0;
        highpassSampleRCB = 0.0;
        highpassSampleRDA = 0.0;
        highpassSampleRDB = 0.0;
        highpassSampleRE = 0.0;
        highpassSampleRF = 0.0;

        flip = false;
        flipthree = 0;
        //end EQ

        //begin Gate
        WasNegativeL = false;
        ZeroCrossL = 0;
        gaterollerL = 0.0;
        gateL = 0.0;

        WasNegativeR = false;
        ZeroCrossR = 0;
        gaterollerR = 0.0;
        gateR = 0.0;
        //end Gate

        //begin Timing
        for(int fcount = 0; fcount < 4098; fcount++) {pL[fcount] = 0.0; pR[fcount] = 0.0;}
        count = 0;
        //end Timing

        //begin ButterComp
        controlAposL = 1.0;
        controlAnegL = 1.0;
        controlBposL = 1.0;
        controlBnegL = 1.0;
        targetposL = 1.0;
        targetnegL = 1.0;
        avgLA = avgLB = 0.0;
        nvgLA = nvgLB = 0.0;

        controlAposR = 1.0;
        controlAnegR = 1.0;
        controlBposR = 1.0;
        controlBnegR = 1.0;
        targetposR = 1.0;
        targetnegR = 1.0;
        avgRA = avgRB = 0.0;
        nvgRA = nvgRB = 0.0;
        //end ButterComp

        fpNShapeL = 0.0;
        fpNShapeR = 0.0;
        //this is reset: values being initialized only once. Startup values, whatever they are.
    }

    CStrip::~CStrip() {
    }

    void CStrip::SetTreble(const float &v) {
        A = v;
        //printf("A: %f\n", A);
    }

    void CStrip::SetMid(const float &v) {
        B = v;
        //printf("B: %f\n", B);
    }

    void CStrip::SetBass(const float &v) {
        C = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetLowpass(const float &v) {
        D = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetTrebfreq(const float &v) {
        E = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetBassfreq(const float &v) {
        F = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetHipass(const float &v) {
        G = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetGate(const float &v) {
        H = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetComp(const float &v) {
        I = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetCompspd(const float &v) {
        J = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetTimelag(const float &v) {
        K = v;
        //printf("C: %f\n", C);
    }

    void CStrip::SetOutgain(const float &v) {
        L = v;
        //printf("C: %f\n", C);
    }
}

