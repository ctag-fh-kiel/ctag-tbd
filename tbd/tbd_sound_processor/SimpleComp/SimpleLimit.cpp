/*
 *	Simple Limiter (source)
 *
 *  File		: SimpleLimit.cpp
 *	Library		: SimpleSource
 *  Version		: 1.12
 *  Implements	: SimpleLimit
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


#include "SimpleLimit.h"
#include "helpers/ctagFastMath.hpp"

namespace chunkware_simple
{
	//-------------------------------------------------------------
	SimpleLimit::SimpleLimit()
		: threshdB_( 0.0f )
		, thresh_( 1.0f )
		, peakHold_( 0 )
		, peakTimer_( 0 )
		, maxPeak_( 1.0f )
		, att_( 1.0f )
		, rel_( 10.0f )
		, env_( 1.0f )
		, mask_( BUFFER_SIZE - 1 )
		, cur_( 0 )
	{
		setAttack( 1.0f );
		std::fill(outBuffer_[ 0 ].begin(), outBuffer_[ 0 ].end(), 0.0f);
        std::fill(outBuffer_[ 1 ].begin(), outBuffer_[ 1 ].end(), 0.0f);
	}

	//-------------------------------------------------------------
	void SimpleLimit::setThresh( float dB )
	{
		threshdB_ = dB;
		thresh_ = dB2lin( dB );
	}

	//-------------------------------------------------------------
	void SimpleLimit::setAttack( float ms )
	{
		unsigned int samp = int( 0.001f * ms * att_.getSampleRate() );

		assert( samp < BUFFER_SIZE );

		peakHold_ = samp;
		att_.setTc( ms );
	}

	//-------------------------------------------------------------
	void SimpleLimit::setRelease( float ms )
	{
		rel_.setTc( ms );
	}

	//-------------------------------------------------------------
	void SimpleLimit::setSampleRate( float sampleRate )
	{
		att_.setSampleRate( sampleRate );
		rel_.setSampleRate( sampleRate );
	}

	//-------------------------------------------------------------
	void SimpleLimit::initRuntime( void )
	{
		peakTimer_ = 0;
		maxPeak_ = thresh_;
		env_ = thresh_;
		cur_ = 0;
        std::fill(outBuffer_[ 0 ].begin(), outBuffer_[ 0 ].end(), 0.0f);
        std::fill(outBuffer_[ 1 ].begin(), outBuffer_[ 1 ].end(), 0.0f);
	}

	//-------------------------------------------------------------
	void SimpleLimit::FastEnvelope::setCoef( void )
	{
		// rises to 99% of in value over duration of time constant
		coef_ = CTAG::SP::HELPERS::powf_fast_precise( 0.01f, (1000.0f / (ms_ * sampleRate_) ) );
	}

}	// end namespace chunkware_simple
