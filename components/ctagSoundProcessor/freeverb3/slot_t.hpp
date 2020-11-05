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

class _FV3_(slot) {
public:
    _FV3_(slot)();

    virtual _FV3_(~slot)();

    void alloc(int32_t nsize, int32_t nch);

    _fv3_float_t *c(int32_t nch);

    void free();

    void mute();

    void mute(int32_t limit);

    void mute(int32_t offset, int32_t limit);

    int32_t getsize() { return size; }

    int32_t getch() { return ch; }

    _fv3_float_t **getArray();

    _fv3_float_t *L, *R;

private:
    _FV3_(slot)(const _FV3_(slot) &x);

    _FV3_(slot) &operator=(const _FV3_(slot) &x);

    int32_t size, ch;
    _fv3_float_t **data;
};
