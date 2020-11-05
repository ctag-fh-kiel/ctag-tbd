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

#include "slot.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

FV3_(slot)::FV3_(slot)() {
    size = ch = 0;
    data = NULL;
    L = R = NULL;
}

FV3_(slot)::~FV3_(slot)() {
    free();
}

void FV3_(slot)::alloc(int32_t nsize, int32_t nch) {
    if (nsize <= 0 || nch <= 0) return;
    free();

    data = (fv3_float_t **) heap_caps_malloc(nch * sizeof(float), MALLOC_CAP_DEFAULT);
    if (data != NULL) {
        ESP_LOGW("slot", "Allocated %d bytes data", (int) (nch * sizeof(float)));
    } else {
        ESP_LOGE("slot", "Bad alloc!");
        return;
    }

    bool slot_bad_alloc = false;
    for (int32_t t = 0; t < nch; t++) {
        data[t] = (fv3_float_t *) heap_caps_malloc(nsize * sizeof(float), MALLOC_CAP_SPIRAM);
        if (data != NULL) {
            ESP_LOGW("slot", "Allocated %d bytes data[t]", (int) (nch * sizeof(float)));
        } else {
            ESP_LOGE("slot", "Bad alloc!");
            return;
        }
        if (data[t] == NULL) slot_bad_alloc = true;
    }
    if (slot_bad_alloc == true) {
        data = NULL;
//        for (int32_t t = 0; t < ch; t++) FV3_(utils)::aligned_free(data[t]);
        //      delete[] data;
        //std::fprintf(stderr, "slot::alloc(%ld, %ld) bad_alloc\n", nsize, nch);
    }
    size = nsize;
    ch = nch;
    L = this->c(0);
    R = this->c(1);
    mute();
}

fv3_float_t *FV3_(slot)::c(int32_t nch) {
    if (ch == 0 || size == 0 || data == NULL) return NULL;
    if (nch < ch) return data[nch];
    return data[0];
}

void FV3_(slot)::free() {
    if (size > 0 && ch > 0 && data != NULL) {
        for (int32_t t = 0; t < ch; t++)
            heap_caps_free(data[t]);
        heap_caps_free(data);
    }
    size = ch = 0;
    data = NULL;
    L = R = NULL;
}

fv3_float_t **FV3_(slot)::getArray() {
    return data;
}

void FV3_(slot)::mute() {
    if (ch == 0 || size == 0 || data == NULL) return;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t], size);
}

void FV3_(slot)::mute(int32_t limit) {
    if (ch == 0 || size == 0 || data == NULL || limit < 0) return;
    if (limit > size) limit = size;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t], limit);
}

void FV3_(slot)::mute(int32_t offset, int32_t limit) {
    if (ch == 0 || size == 0 || data == NULL || offset < 0 || limit < 0) return;
    if (offset > size) offset = size;
    if (offset + limit > size) limit = size - offset;
    for (int32_t t = 0; t < ch; t++) FV3_(utils)::mute(data[t] + offset, limit);
}

#include "fv3_ns_end.h"
