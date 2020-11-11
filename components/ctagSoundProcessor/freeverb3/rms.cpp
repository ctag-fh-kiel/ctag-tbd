/**
 *  RMS
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

#include "rms.hpp"
#include "fv3_type_float.h"
#include "fv3_ns_start.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

FV3_(rms)::FV3_(rms)() {
    sum = bufs = 0;
    bufsize = bufidx = 0;
    buffer = NULL;
}

FV3_(rms)::~FV3_(rms)() {
    this->free();
}

int32_t FV3_(rms)::getsize() {
    return bufsize;
}

void FV3_(rms)::setsize(int32_t size) {
    if (size <= 0) return;
    this->free();

    buffer = (float*) heap_caps_malloc(size * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if(buffer == NULL){
        ESP_LOGE("rms", "Cannot alloc mem trying SPIRAM!");
        buffer = (float*) heap_caps_malloc(size * sizeof(float), MALLOC_CAP_SPIRAM);
        if(buffer == NULL) {
            ESP_LOGE("rms", "Cannot alloc mem on SPIRAM!");
            return;
        }
    }/*else{
        ESP_LOGE("rms", "Mem alloc success requested size %d, freesize %d, largest block %d!",
                 (int)(size * sizeof(float)), heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
    }*/

    bufs = bufsize = size;
    mute();
}

void FV3_(rms)::free() {
    if (buffer != NULL && bufsize != 0)
        heap_caps_free(buffer);
    buffer = NULL;
    bufidx = bufsize = 0;
}

void FV3_(rms)::mute() {
    if (buffer == NULL || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    sum = 0;
    bufidx = 0;
}

#include "fv3_ns_end.h"
