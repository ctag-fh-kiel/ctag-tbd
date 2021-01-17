// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _comb_
#define _comb_

#include "denormals.hh"
#include "esp_system.h"

class comb {
public:
    comb();

    void setbuffer(float *buf, int size);

    inline float     process(float inp);

    void mute();

    void setdamp(float val);

    float getdamp();

    void setfeedback(float val);

    float getfeedback();

private:
    float feedback;
    float filterstore;
    float damp1;
    float damp2;
    float *buffer;
    int bufsize;
    int bufidx;
};


// Big to inline - but crucial for speed

inline float  comb::process(float input) {
    float output;

    output = buffer[bufidx];

    output += 1e-18f;
    output -= 1e-18f;

    //undenormalise(output);

    filterstore = (output * damp2) + (filterstore * damp1);
    //undenormalise(filterstore);

    filterstore += 1e-18f;
    filterstore -= 1e-18f;


    buffer[bufidx] = input + (filterstore * feedback);
    bufidx++;
    bufidx %= bufsize;
    //if(++bufidx>=bufsize) bufidx = 0;

    return output;
}

#endif //_comb_

//ends
