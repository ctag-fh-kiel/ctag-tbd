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

#include <tbd/sound_utils/freeverb3/rms.hpp>
#include <tbd/sound_utils/freeverb3/fv3_type_float.h>
#include <tbd/sound_utils/freeverb3/fv3_ns_start.h>

FV3_(rms)::FV3_(rms)() {
    sum = bufs = 0;
    bufsize = bufidx = 0;
    buffer = nullptr;
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

    buffer = (float*) utils_f::fv3_malloc(size * sizeof(float));

    bufs = bufsize = size;
    mute();
}

void FV3_(rms)::free() {
    if (buffer != nullptr && bufsize != 0)
        utils_f::fv3_free(buffer);
    buffer = nullptr;
    bufidx = bufsize = 0;
}

void FV3_(rms)::mute() {
    if (buffer == nullptr || bufsize == 0) return;
    FV3_(utils)::mute(buffer, bufsize);
    sum = 0;
    bufidx = 0;
}

#include <tbd/sound_utils/freeverb3/fv3_ns_end.h>
