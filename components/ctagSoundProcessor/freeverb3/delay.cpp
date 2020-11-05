/**
 *  Simple Delay
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

#include "delay.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

// simple delay

FV3_(delay)::FV3_(delay)() {
    isExternalMem = false;
    feedback = 1.;
    bufsize = bufidx = 0;
    buffer = NULL;
}

FV3_(delay)::~FV3_(delay)() {
    free();
}

int32_t FV3_(delay)::getsize() {
    return bufsize;
}

void FV3_(delay)::setsize(float *buf, const uint32_t size) {
    if (buf == NULL) {
        buffer = NULL;
        return;
    }
    isExternalMem = true;
    FV3_(utils)::mute(buf, size);

    if (bufsize > 0 && bufsize <= size) {
        int32_t prefix_i = size - bufsize;
        for (int32_t i = 0; i < bufsize; i++) buf[prefix_i + i] = this->process(0);
    }
    if (bufsize > 0 && bufsize > size) {
        int32_t cut = bufsize - size;
        for (int32_t i = 0; i < cut; i++) this->process(0);
        for (int32_t i = 0; i < size; i++) buf[i] = this->process(0);
    }

    bufidx = 0;
    bufsize = size;
    buffer = buf;
}

void FV3_(delay)::setsize(int32_t size) {
    if (size <= 0) return;
    fv3_float_t *new_buffer = NULL;

    new_buffer = (fv3_float_t *) heap_caps_malloc(size * sizeof(float), MALLOC_CAP_SPIRAM);

    if (new_buffer != NULL) {
        ESP_LOGW("delay", "Allocated %d bytes", (int) (size * sizeof(float)));
    } else {
        ESP_LOGE("delay", "Bad alloc!");
        return;
    }

    FV3_(utils)::mute(new_buffer, size);

    if (bufsize > 0 && bufsize <= size) {
        int32_t prefix_i = size - bufsize;
        for (int32_t i = 0; i < bufsize; i++) new_buffer[prefix_i + i] = this->process(0);
    }
    if (bufsize > 0 && bufsize > size) {
        int32_t cut = bufsize - size;
        for (int32_t i = 0; i < cut; i++) this->process(0);
        for (int32_t i = 0; i < size; i++) new_buffer[i] = this->process(0);
    }

    this->free();
    bufidx = 0;
    bufsize = size;
    buffer = new_buffer;
}

void FV3_(delay)::free() {
    if (isExternalMem) return;
    if (buffer == NULL || bufsize == 0) return;
    heap_caps_free(buffer);
    buffer = NULL;
    bufidx = bufsize = 0;
}

void FV3_(delay)::mute() {
    if (buffer == NULL || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    bufidx = 0;
}

void FV3_(delay)::setfeedback(fv3_float_t val) {
    feedback = val;
}

fv3_float_t FV3_(delay)::getfeedback() {
    return feedback;
}

// modulated delay

FV3_(delaym)::FV3_(delaym)() {
    bufsize = readidx = writeidx = modulationsize = 0;
    feedback = 1.;
    z_1 = modulationsize_f = 0;
    buffer = NULL;
}

FV3_(delaym)::~FV3_(delaym)() {
    if (bufsize != 0) delete[] buffer;
}

int32_t FV3_(delaym)::getsize() {
    return bufsize;
}

int32_t FV3_(delaym)::getdelaysize() {
    return (bufsize - modulationsize);
}

int32_t FV3_(delaym)::getmodulationsize() {
    return modulationsize;
}

void FV3_(delaym)::setsize(int32_t size) {
    setsize(size, 0);
}

void FV3_(delaym)::setsize(int32_t size, int32_t modsize) {
    if (size <= 0) return;
    if (modsize < 0) modsize = 0;
    if (modsize > size) modsize = size;
    int32_t newsize = size + modsize;
    fv3_float_t *new_buffer = NULL;

    new_buffer = (fv3_float_t *) heap_caps_malloc(newsize * sizeof(float), MALLOC_CAP_SPIRAM);

    if (new_buffer != NULL) {
        ESP_LOGW("delay", "Allocated %d bytes", (int) (newsize * sizeof(float)));
    } else {
        ESP_LOGE("delay", "Bad alloc!");
        return;
    }

    FV3_(utils)::mute(new_buffer, newsize);

    this->free();
    bufsize = newsize;
    readidx = modsize * 2;
    writeidx = 0;
    modulationsize = modsize;
    modulationsize_f = (fv3_float_t) modulationsize;
    buffer = new_buffer;
    z_1 = 0;
}

void FV3_(delaym)::free() {
    if (buffer == NULL || bufsize == 0) return;
    heap_caps_free(buffer);
    buffer = NULL;
    writeidx = bufsize = 0;
    z_1 = 0;
}

void FV3_(delaym)::mute() {
    if (buffer == NULL || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    writeidx = 0;
    z_1 = 0;
    readidx = modulationsize * 2;
}

void FV3_(delaym)::setfeedback(fv3_float_t val) {
    feedback = val;
}

fv3_float_t FV3_(delaym)::getfeedback() {
    return feedback;
}

#include "fv3_ns_end.h"
