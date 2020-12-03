#include "TDelay.hpp"
#include "esp_heap_caps.h"
#include <cmath>
#include <cstdlib>

namespace airwindows {

    void TDelay::Process(float *buf, int sz, int ch) {
        if(isBypass) return;

        float dry = powf(A, 2.f);
        float wet = powf(B, 2.f);
        int targetdelay = (int) (88000.f * C);
        float feedback = (D * 1.3f);
        float leanfat = ((E * 2.0f) - 1.0f);
        float fatwet = fabsf(leanfat);
        int fatness = (int) floor((F * 29.0f) + 3.0f);
        int count;

        float storedelay;
        float sum = 0.0f;
        float floattotal = 0.0f;
        int sumtotal = 0.f;

        for (int i = 0; i < sz; i++) {
            float inputSample = buf[i * 2 + ch];

            static int noisesource = 0;
            int residue;
            float applyresidue;

            noisesource = noisesource % 1700021;
            noisesource++;
            residue = noisesource * noisesource;
            residue = residue % 170003;
            residue *= residue;
            residue = residue % 17011;
            residue *= residue;
            residue = residue % 1709;
            residue *= residue;
            residue = residue % 173;
            residue *= residue;
            residue = residue % 17;
            applyresidue = residue;
            applyresidue *= 0.00000001f;
            applyresidue *= 0.00000001f;
            inputSample += applyresidue;
            if (inputSample < 1.2e-38f && -inputSample < 1.2e-38f) {
                inputSample -= applyresidue;
            }

            if (gcount < 0 || gcount > 128) { gcount = 128; }
            count = gcount;
            if (delay < 0 || delay > maxdelay) { delay = maxdelay; }

            sum = inputSample + (d[delay] * feedback);
            p[count + 128] = p[count] = sumtotal = (int) (sum * 8388608.0f);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

            switch (fatness) {
                case 32:
                    sumtotal += p[count + 127]; //sumtotalR += pR[count+127]; //note NO break statement.
                case 31:
                    sumtotal += p[count + 113]; //sumtotalR += pR[count+113]; //This jumps to the relevant tap
                case 30:
                    sumtotal += p[count + 109]; //sumtotalR += pR[count+109]; //and then includes all smaller taps.
                case 29:
                    sumtotal += p[count + 107]; //sumtotalR += pR[count+107];
                case 28:
                    sumtotal += p[count + 103]; //sumtotalR += pR[count+103];
                case 27:
                    sumtotal += p[count + 101]; //sumtotalR += pR[count+101];
                case 26:
                    sumtotal += p[count + 97]; //sumtotalR += pR[count+97];
                case 25:
                    sumtotal += p[count + 89]; //sumtotalR += pR[count+89];
                case 24:
                    sumtotal += p[count + 83]; //sumtotalR += pR[count+83];
                case 23:
                    sumtotal += p[count + 79]; //sumtotalR += pR[count+79];
                case 22:
                    sumtotal += p[count + 73]; //sumtotalR += pR[count+73];
                case 21:
                    sumtotal += p[count + 71]; //sumtotalR += pR[count+71];
                case 20:
                    sumtotal += p[count + 67]; //sumtotalR += pR[count+67];
                case 19:
                    sumtotal += p[count + 61]; //sumtotalR += pR[count+61];
                case 18:
                    sumtotal += p[count + 59]; //sumtotalR += pR[count+59];
                case 17:
                    sumtotal += p[count + 53]; //sumtotalR += pR[count+53];
                case 16:
                    sumtotal += p[count + 47]; //sumtotalR += pR[count+47];
                case 15:
                    sumtotal += p[count + 43]; //sumtotalR += pR[count+43];
                case 14:
                    sumtotal += p[count + 41]; //sumtotalR += pR[count+41];
                case 13:
                    sumtotal += p[count + 37]; //sumtotalR += pR[count+37];
                case 12:
                    sumtotal += p[count + 31]; //sumtotalR += pR[count+31];
                case 11:
                    sumtotal += p[count + 29]; //sumtotalR += pR[count+29];
                case 10:
                    sumtotal += p[count + 23]; //sumtotalR += pR[count+23];
                case 9:
                    sumtotal += p[count + 19]; //sumtotalR += pR[count+19];
                case 8:
                    sumtotal += p[count + 17]; //sumtotalR += pR[count+17];
                case 7:
                    sumtotal += p[count + 13]; //sumtotalR += pR[count+13];
                case 6:
                    sumtotal += p[count + 11]; //sumtotalR += pR[count+11];
                case 5:
                    sumtotal += p[count + 7]; //sumtotalR += pR[count+7];
                case 4:
                    sumtotal += p[count + 5]; //sumtotalR += pR[count+5];
                case 3:
                    sumtotal += p[count + 3]; //sumtotalR += pR[count+3];
                case 2:
                    sumtotal += p[count + 2]; //sumtotalR += pR[count+2];
                case 1:
                    sumtotal += p[count + 1]; //sumtotalR += pR[count+1];
            }
#pragma GCC diagnostic pop
            floattotal = (float) (sumtotal / fatness + 1);
            floattotal /= 8388608.0f;
            floattotal *= fatwet;
            if (leanfat < 0.f) {
                storedelay = sum - floattotal;
            } else {
                storedelay = (sum * (1.f - fatwet)) + floattotal;

            }

            chase += abs(maxdelay - targetdelay);
            if (chase > 9000) {
                if (maxdelay > targetdelay) {
                    d[delay] = storedelay; //dR[delay] = storedelayR;
                    maxdelay -= 1;
                    delay -= 1;
                    if (delay < 0) { delay = maxdelay; }
                    d[delay] = storedelay; //dR[delay] = storedelayR;
                }
                if (maxdelay < targetdelay) {
                    maxdelay += 1;
                    delay += 1;
                    if (delay > maxdelay) { delay = 0; }
                    d[delay] = storedelay; //dR[delay] = storedelayR;
                }
                chase = 0;
            } else {
                d[delay] = storedelay; //dR[delay] = storedelayR;
            }

            gcount--;
            delay--;
            if (delay < 0 || delay > maxdelay) { delay = maxdelay; }
            //yes this is a second bounds check. it's cheap, check EVERY time

            inputSample = (inputSample * dry) + (d[delay] * wet);
            //inputSampleR = (inputSampleR * dry) + (dR[delay] * wet);

            //stereo 32 bit dither, made small and tidy.
            int expon;
            frexpf((float) inputSample, &expon);
            float dither = (rand() / (RAND_MAX * 7.737125245533627e+25)) * powf(2.f, expon + 62);
            inputSample += (dither - fpNShape);
            fpNShape = dither;
            /*
            frexpf((float)inputSampleR, &expon);
            dither = (rand()/(RAND_MAX*7.737125245533627e+25))*powf(2,expon+62);
            inputSampleR += (dither-fpNShapeR); fpNShapeR = dither;
             */
            //end 32 bit dither

            buf[i * 2 + ch] = inputSample;
        }
    }

    void TDelay::SetWet(const float &v) {
        B = v;
    }

    void TDelay::SetLNFT(const float &v) {
        E = v;
    }

    void TDelay::SetDepth(const float &v) {
        F = v;
    }

    void TDelay::SetDelay(const float &v) {
        C = v;
    }

    void TDelay::SetFeedback(const float &v) {
        D = v;
    }

    void TDelay::SetBypass(const bool v) {
        isBypass = v;
    }

    TDelay::TDelay() {
        d = (float *) heap_caps_malloc(length * sizeof(float), MALLOC_CAP_SPIRAM);
        p = (int *) heap_caps_malloc(258 * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        A = 1.0;
        B = 0.0;
        C = 0.5;
        D = 0.0; //0 to 130%
        E = 1.0; //-1.0 to 1.0
        F = 0.0; //8 taps

        for (int count = 0; count < 258; count++) { p[count] = 0; }
        for (delay = 0; delay < length; delay++) { d[delay] = 0.0; }
        maxdelay = 0;
        delay = 0;
        gcount = 0;
        chase = 0;

        fpNShape = 0.0;
    }

    TDelay::~TDelay() {
        heap_caps_free(d);
        heap_caps_free(p);
    }

    void TDelay::SetDry(const float &v) {
        A = v;
    }
}