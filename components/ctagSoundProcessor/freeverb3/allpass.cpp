/**
 *  Allpass filter
 *
 *  Copyright (C) 2000 Jezar at Dreampoint
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

#include "allpass.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

// simple allpass filter

FV3_(allpass)::FV3_(allpass)() {
    isExternalMem = false;
    bufsize = bufidx = 0;
    decay = 1;
    buffer = NULL;
}

FV3_(allpass)::FV3_(~allpass)() {
    if (isExternalMem) return;
    free();
}

int32_t FV3_(allpass)::getsize() {
    return bufsize;
}

void FV3_(allpass)::setsize(fv3_float_t *buf, const uint32_t size) {
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

void FV3_(allpass)::setsize(int32_t size) {
    if (size <= 0) return;
    fv3_float_t *new_buffer = NULL;
    new_buffer = (fv3_float_t *) heap_caps_malloc(size * sizeof(float), MALLOC_CAP_SPIRAM);
    if (new_buffer != NULL) {
        ESP_LOGW("allpass", "Allocated %d bytes for allpass", (int) (size * sizeof(float)));
    } else {
        ESP_LOGE("allpass", "Bad alloc!");
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

    free();
    bufidx = 0;
    bufsize = size;
    buffer = new_buffer;
}

void FV3_(allpass)::free() {
    if (buffer == NULL || bufsize == 0) return;
    heap_caps_free(buffer);
    buffer = NULL;
    bufidx = bufsize = 0;
}

void FV3_(allpass)::mute() {
    if (buffer == NULL || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    bufidx = 0;
}

void FV3_(allpass)::setfeedback(fv3_float_t val) {
    feedback = val;
}

fv3_float_t FV3_(allpass)::getfeedback() {
    return feedback;
}

void FV3_(allpass)::setdecay(fv3_float_t val) {
    decay = val;
}

fv3_float_t FV3_(allpass)::getdecay() {
    return decay;
}

// modulated allpass filter

FV3_(allpassm)::FV3_(allpassm)() {
    isExternalMem = false;
    bufsize = readidx = writeidx = modulationsize = 0;
    feedback = feedback_mod = z_1 = modulationsize_f = 0;
    buffer = NULL;
    decay = 1;
}

FV3_(allpassm)::FV3_(~allpassm)() {
    if (isExternalMem) return;
    free();
}

int32_t FV3_(allpassm)::getsize() {
    return bufsize;
}

int32_t FV3_(allpassm)::getdelaysize() {
    return (bufsize - modulationsize);
}

int32_t FV3_(allpassm)::getmodulationsize() {
    return modulationsize;
}

void FV3_(allpassm)::setsize(int32_t size) {
    setsize(size, 0);
}

void FV3_(allpassm)::setsize(fv3_float_t *buf, const uint32_t size, const uint32_t modsize) {
    if (buf == NULL) {
        buffer = NULL;
        return;
    }
    isExternalMem = true;
    int32_t newsize = size + modsize;
    FV3_(utils)::mute(buf, newsize);
    bufsize = newsize;
    readidx = modsize * 2;
    writeidx = 0;
    modulationsize = modsize;
    modulationsize_f = (fv3_float_t) modulationsize;
    buffer = buf;
    z_1 = 0;
}

void FV3_(allpassm)::setsize(int32_t size, int32_t modsize) {
    if (size <= 0) return;
    if (modsize < 0) modsize = 0;
    if (modsize > size) modsize = size;
    int32_t newsize = size + modsize;
    fv3_float_t *new_buffer = NULL;

    new_buffer = (fv3_float_t *) heap_caps_malloc(newsize * sizeof(float), MALLOC_CAP_SPIRAM);
    if (new_buffer != NULL) {
        ESP_LOGW("allpassm", "Allocated %d bytes for allpassm", (int) (newsize * sizeof(float)));
    } else {
        ESP_LOGE("allpassm", "Bad alloc!");
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

void FV3_(allpassm)::free() {
    if (buffer == NULL || bufsize == 0) return;
    heap_caps_free(buffer);
    buffer = NULL;
    writeidx = bufsize = 0;
    z_1 = 0;
}

void FV3_(allpassm)::mute() {
    if (buffer == NULL || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    writeidx = 0;
    z_1 = 0;
    readidx = modulationsize * 2;
    feedback_mod = feedback;
}

void FV3_(allpassm)::setfeedback(fv3_float_t val) {
    feedback_mod = feedback = val;
}

fv3_float_t FV3_(allpassm)::getfeedback() {
    return feedback;
}

void FV3_(allpassm)::setdecay(fv3_float_t val) {
    decay = val;
}

fv3_float_t FV3_(allpassm)::getdecay() {
    return decay;
}

void FV3_(allpassm)::set_90degfq(fv3_float_t fc, fv3_float_t fs) {
    fv3_float_t tant = std::tan(M_PI * fc / fs);
    feedback = (tant - 1) / (tant + 1);
}

// 2nd order allpass filter

FV3_(allpass2)::FV3_(allpass2)() {
    bufsize1 = bufidx1 = bufsize2 = bufidx2 = 0;
    decay1 = decay2 = 1;
    feedback1 = feedback2 = 0;
    buffer1 = buffer2 = NULL;
}

FV3_(allpass2)::FV3_(~allpass2)() {
    this->free();
}

void FV3_(allpass2)::setsize(int32_t size1, int32_t size2) {
    if (size1 <= 0 || size2 <= 0) return;
    free();

    buffer1 = (fv3_float_t *) heap_caps_malloc(size1 * sizeof(float), MALLOC_CAP_SPIRAM);
    if (buffer1 != NULL) {
        ESP_LOGW("allpass2", "Allocated %d bytes", (int) (size1 * sizeof(float)));
    } else {
        ESP_LOGE("allpass2", "Bad alloc!");
        return;
    }

    buffer2 = (fv3_float_t *) heap_caps_malloc(size2 * sizeof(float), MALLOC_CAP_SPIRAM);
    if (buffer2 != NULL) {
        ESP_LOGW("allpass2", "Allocated %d bytes for allpass2", (int) (size2 * sizeof(float)));
    } else {
        ESP_LOGE("allpass2", "Bad alloc!");
        return;
    }

    bufsize1 = size1;
    bufsize2 = size2;
    mute();
}

void FV3_(allpass2)::free() {
    if (buffer1 == NULL || bufsize1 == 0 || buffer2 == NULL || bufsize2 == 0) return;
    heap_caps_free(buffer1);
    heap_caps_free(buffer2);
    buffer1 = buffer2 = NULL;
    bufidx1 = bufidx2 = bufsize1 = bufsize2 = 0;
}

void FV3_(allpass2)::mute() {
    if (buffer1 == NULL || bufsize1 == 0 || buffer2 == NULL || bufsize2 == 0) return;
    FV3_(utils)::mute(buffer1, bufsize1);
    FV3_(utils)::mute(buffer2, bufsize2);
}

void FV3_(allpass2)::setfeedback1(fv3_float_t val) {
    feedback1 = val;
}

void FV3_(allpass2)::setfeedback2(fv3_float_t val) {
    feedback2 = val;
}

void FV3_(allpass2)::setdecay1(fv3_float_t val) {
    decay1 = val;
}

void FV3_(allpass2)::setdecay2(fv3_float_t val) {
    decay2 = val;
}

// 3rd order allpass filter

FV3_(allpass3)::FV3_(allpass3)() {
    bufsize1 = readidx1 = writeidx1 = bufsize2 = bufidx2 = bufsize3 = bufidx3 = modulationsize = 0;
    decay1 = decay2 = decay3 = 1;
    buffer1 = buffer2 = buffer3 = NULL;
    feedback1 = feedback2 = feedback3 = modulationsize_f = 0;
}

FV3_(allpass3)::FV3_(~allpass3)() {
    this->free();
}

void FV3_(allpass3)::setsize(int32_t size1, int32_t size2, int32_t size3) {
    setsize(size1, 0, size2, size3);
}

void FV3_(allpass3)::setsize(int32_t size1, int32_t size1mod, int32_t size2, int32_t size3) {
    if (size1 <= 0 || size2 <= 0 || size3 <= 0) return;
    if (size1mod < 0) size1mod = 0;
    if (size1mod > size1) size1mod = size1;
    this->free();

    buffer1 = (fv3_float_t *) heap_caps_malloc((size1 + size1mod) * sizeof(float), MALLOC_CAP_SPIRAM);
    if (buffer1 != NULL) {
        ESP_LOGW("allpass3", "Allocated %d bytes for allpass3", (int) ((size1 + size1mod) * sizeof(float)));
    } else {
        ESP_LOGE("allpass3", "Bad alloc!");
        return;
    }

    buffer2 = (fv3_float_t *) heap_caps_malloc(size2 * sizeof(float), MALLOC_CAP_SPIRAM);
    if (buffer2 != NULL) {
        ESP_LOGW("allpass3", "Allocated %d bytes for allpass3", (int) (size2 * sizeof(float)));
    } else {
        ESP_LOGE("allpass3", "Bad alloc!");
        return;
    }

    buffer3 = (fv3_float_t *) heap_caps_malloc(size3 * sizeof(float), MALLOC_CAP_SPIRAM);
    if (buffer3 != NULL) {
        ESP_LOGW("allpass3", "Allocated %d bytes for allpass3", (int) (size3 * sizeof(float)));
    } else {
        ESP_LOGE("allpass3", "Bad alloc!");
        return;
    }

    bufsize1 = size1 + size1mod;
    readidx1 = size1mod * 2;
    writeidx1 = 0;
    modulationsize = size1mod;
    modulationsize_f = (fv3_float_t) modulationsize;
    bufsize2 = size2;
    bufsize3 = size3;
    mute();
}

void FV3_(allpass3)::free() {
    if (buffer1 == NULL || bufsize1 == 0 || buffer2 == NULL || bufsize2 == 0 || buffer3 == NULL || bufsize3 == 0)
        return;
    heap_caps_free(buffer1);
    heap_caps_free(buffer2);
    heap_caps_free(buffer3);
    buffer1 = buffer2 = buffer3 = NULL;
    readidx1 = writeidx1 = bufidx2 = bufidx3 = bufsize1 = bufsize2 = bufsize3 = 0;
}

void FV3_(allpass3)::mute() {
    if (buffer1 == NULL || bufsize1 == 0 || buffer2 == NULL || bufsize2 == 0 || buffer3 == NULL || bufsize3 == 0)
        return;
    FV3_(utils)::mute(buffer1, bufsize1);
    FV3_(utils)::mute(buffer2, bufsize2);
    FV3_(utils)::mute(buffer3, bufsize3);
    writeidx1 = 0;
    readidx1 = modulationsize * 2;
}

void FV3_(allpass3)::setfeedback1(fv3_float_t val) {
    feedback1 = val;
}

void FV3_(allpass3)::setfeedback2(fv3_float_t val) {
    feedback2 = val;
}

void FV3_(allpass3)::setfeedback3(fv3_float_t val) {
    feedback3 = val;
}

void FV3_(allpass3)::setdecay1(fv3_float_t val) {
    decay1 = val;
}

void FV3_(allpass3)::setdecay2(fv3_float_t val) {
    decay2 = val;
}

void FV3_(allpass3)::setdecay3(fv3_float_t val) {
    decay3 = val;
}

#include "fv3_ns_end.h"
