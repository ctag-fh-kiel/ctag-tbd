/**
 *  Simple Slot
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

#include <tbd/sound_utils/freeverb3/slot.hpp>
#include <tbd/sound_utils/freeverb3/fv3_type_float.h>
#include <tbd/sound_utils/freeverb3/fv3_ns_start.h>

FV3_(slot)::FV3_(slot)() {
    size = ch = 0;
    data = nullptr;
    L = R = nullptr;
}

FV3_(slot)::~FV3_(slot)() {
    free();
}

void FV3_(slot)::alloc(int32_t nsize, int32_t nch) {
    if (nsize <= 0 || nch <= 0) return;
    free();

    data = (fv3_float_t **) utils_f::fv3_malloc(nch * sizeof(float));
    
    for (int32_t t = 0; t < nch; t++) {
        data[t] = (fv3_float_t *) utils_f::fv3_malloc(nsize * sizeof(float));
    }

    size = nsize;
    ch = nch;
    L = this->c(0);
    R = this->c(1);
    mute();
}

fv3_float_t *FV3_(slot)::c(int32_t nch) {
    if (ch == 0 || size == 0 || data == nullptr) return nullptr;
    if (nch < ch) return data[nch];
    return data[0];
}

void FV3_(slot)::free() {
    if (size > 0 && ch > 0 && data != nullptr) {
        for (int32_t t = 0; t < ch; t++)
            utils_f::fv3_free(data[t]);
        utils_f::fv3_free(data);
    }
    size = ch = 0;
    data = nullptr;
    L = R = nullptr;
}

fv3_float_t **FV3_(slot)::getArray() {
    return data;
}

void FV3_(slot)::mute() {
    if (ch == 0 || size == 0 || data == nullptr) return;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t], size);
}

void FV3_(slot)::mute(int32_t limit) {
    if (ch == 0 || size == 0 || data == nullptr || limit < 0) return;
    if (limit > size) limit = size;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t], limit);
}

void FV3_(slot)::mute(int32_t offset, int32_t limit) {
    if (ch == 0 || size == 0 || data == nullptr || offset < 0 || limit < 0) return;
    if (offset > size) offset = size;
    if (offset + limit > size) limit = size - offset;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t] + offset, limit);
}

#include <tbd/sound_utils/freeverb3/fv3_ns_end.h>
