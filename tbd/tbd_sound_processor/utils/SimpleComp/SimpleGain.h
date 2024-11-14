/*
 *	Gain Functions (header)
 *
 *  File		: SimpleGain.h
 *	Library		: SimpleSource
 *  Version		: 1.12
 *  Class		: 
 *
 *	� 2006, ChunkWare Music Software, OPEN-SOURCE
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a
 *	copy of this software and associated documentation files (the "Software"),
 *	to deal in the Software without restriction, including without limitation
 *	the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *	and/or sell copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *	DEALINGS IN THE SOFTWARE.
 */


#ifndef __SIMPLE_GAIN_H__
#define __SIMPLE_GAIN_H__

#include "SimpleHeader.h"		// common header
#include "helpers/ctagFastMath.hpp"

namespace chunkware_simple
{
	//-------------------------------------------------------------
	// gain functions
	//-------------------------------------------------------------

	// linear -> dB conversion
	static INLINE float lin2dB( float lin ) {
        /*
		static const float LOG_2_DB = 8.6858896380650365530225783783321f;	// 20 / ln( 10 )
		return CTAG::SP::HELPERS::logf_fast( lin ) * LOG_2_DB;
         */

        return CTAG::SP::HELPERS::fast_dBV(lin);
	}

	// dB -> linear conversion
	static INLINE float dB2lin( float dB ) {
        /*
		static const float DB_2_LOG = 0.11512925464970228420089957273422f;	// ln( 10 ) / 20
		return CTAG::SP::HELPERS::expf_fast( dB * DB_2_LOG );
         */
        return CTAG::SP::HELPERS::fast_VdB(dB);
	}

}	// end namespace chunkware_simple

#endif	// end __SIMPLE_GAIN_H__
