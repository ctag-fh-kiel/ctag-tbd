// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "comb.hpp"
#include <cstring>

comb::comb() {
    filterstore = 0;
    bufidx = 0;
}

void comb::setbuffer(float *buf, int size) {
    buffer = buf;
    bufsize = size;
}

void comb::mute() {
    memset(buffer, 0, bufsize * sizeof(float));
    /*
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
     */
}

void comb::setdamp(float val) {
    damp1 = val;
    damp2 = 1.f - val;
}

float comb::getdamp() {
    return damp1;
}

void comb::setfeedback(float val) {
    feedback = val;
}

float comb::getfeedback() {
    return feedback;
}

// ends
