/**
 *  DelayLine Processor Base Class
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

#include "delayline.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"

FV3_(delayline)::FV3_(delayline)() {
    currentfs = FV3_REVBASE_DEFAULT_FS;
    bufsize = baseidx = 0, buffer = nullptr;
}

FV3_(delayline)::~FV3_(delayline)() {
    free();
}

void FV3_(delayline)::setSampleRate(fv3_float_t fs) {
    currentfs = fs;
}

fv3_float_t FV3_(delayline)::getSampleRate() {
    return currentfs;
}

int32_t FV3_(delayline)::getsize() {
    return bufsize;
}

void FV3_(delayline)::setsize(int32_t size) {
    if (size <= 0) return;
    fv3_float_t *new_buffer = nullptr;

    new_buffer = (float*) utils_f::fv3_malloc(size * sizeof(float));

    FV3_(utils)::mute(new_buffer, size);

    if (bufsize > 0 && bufsize <= size) {
        for (int32_t i = 0; i < bufsize; i++) new_buffer[size - bufsize + i] = at(i);
    }
    if (bufsize > 0 && bufsize > size) {
        //int32_t cut = bufsize - size;
        for (int32_t i = 0; i < size; i++) new_buffer[i] = at(i);
    }

    this->free();
    bufsize = size;
    buffer = new_buffer;
}

void FV3_(delayline)::free() {
    if (buffer == nullptr || bufsize == 0) return;
    utils_f::fv3_free(buffer);
    buffer = nullptr;
    baseidx = bufsize = 0;
}

void FV3_(delayline)::mute() {
    if (buffer == nullptr || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
}

fv3_float_t FV3_(delayline)::process(fv3_float_t input) {
    // simple delay line example
    baseidx--;
    if (baseidx < 0) baseidx += bufsize;
    fv3_float_t lastOut = (*this)[0];
    (*this)[0] = input;
    return lastOut;
}

int32_t FV3_(delayline)::p_(fv3_float_t ms) {
    int32_t base = static_cast<int32_t>(currentfs * ms * 0.001);
    if (primeMode) { while (!FV3_(utils)::isPrime(base)) base++; }
    return base;
}

#include "fv3_ns_end.h"
