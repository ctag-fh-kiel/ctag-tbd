/**
 *  Progenitor reverberator after Griesinger ca.1978.
 *
 *  Copyright (C) 1977-1978 David Griesinger
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _FV3_PROGENITOR_HPP
#define _FV3_PROGENITOR_HPP

#include "revbase.hpp"
#include "comb.hpp"
#include "allpass.hpp"
#include "efilter.hpp"
#include "biquad.hpp"
#include "fv3_defs.h"
#include "helpers/ctagFastMath.hpp"
#include <cmath>

#define FV3_PROGENITOR_DEFAULT_FS 34125
#define FV3_PROGENITOR_NUM_ALLPASS 8
#define FV3_PROGENITOR_NUM_DELAY 6
#define FV3_PROGENITOR_NUM_INDEX 7
#define FV3_PROGENITOR_OUT_INDEX 11
#define FV3_PROGENITOR_OLPF_LIMIT 2.5

namespace fv3 {

#define _fv3_float_t float
#define _FV3_(name) name ## _f

#include "progenitor_t.hpp"

#undef _FV3_
#undef _fv3_float_t

};

#endif
