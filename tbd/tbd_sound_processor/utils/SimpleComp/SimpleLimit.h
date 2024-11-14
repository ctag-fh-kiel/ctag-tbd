/*
 *	Simple Limiter (header)
 *
 *  File		: SimpleLimit.h
 *	Library		: SimpleSource
 *  Version		: 1.12
 *  Class		: SimpleLimit
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


#ifndef __SIMPLE_LIMIT_H__
#define __SIMPLE_LIMIT_H__

#include "SimpleHeader.h"		// common header
#include "SimpleEnvelope.h"		// for base class of FastEnvelope
#include "SimpleGain.h"			// for gain functions
#include <array>

namespace chunkware_simple
{
	//-------------------------------------------------------------
	// simple limiter
	//-------------------------------------------------------------
	class SimpleLimit
	{
	public:
		SimpleLimit();
		virtual ~SimpleLimit() {}

		// parameters
		virtual void setThresh( float dB );
		virtual void setAttack( float ms );
		virtual void setRelease( float ms );

		virtual float getThresh( void )  const { return threshdB_; }
		virtual float getAttack( void )  const { return att_.getTc(); }
		virtual float getRelease( void ) const { return rel_.getTc(); }

		// latency
		virtual unsigned int getLatency( void ) const { return peakHold_; }

		// sample rate dependencies
		virtual void   setSampleRate( float sampleRate );
		virtual float getSampleRate( void ) { return att_.getSampleRate(); }
		
		// runtime
		virtual void initRuntime( void );			// call before runtime (in resume())
		void process( float &in1, float &in2 );	// limiter runtime process

	protected:

		// class for faster attack/release
		class FastEnvelope : public EnvelopeDetector
		{
		public:
			FastEnvelope( float ms = 1.0f, float sampleRate = 44100.0f )
				: EnvelopeDetector( ms, sampleRate )
			{}
			virtual ~FastEnvelope() {}

		protected:
			// override setCoef() - coefficient calculation
			virtual void setCoef( void );
		};
		
	private:
		
		// transfer function
		float threshdB_;	// threshold (dB)
		float thresh_;		// threshold (linear)

		// max peak
		unsigned int peakHold_;		// peak hold (samples)
		unsigned int peakTimer_;	// peak hold timer
		float maxPeak_;			// max peak

		// attack/release envelope
		FastEnvelope att_;			// attack
		FastEnvelope rel_;			// release
		float env_;				// over-threshold envelope (linear)

		// buffer
		// BUFFER_SIZE default can handle up to ~10ms at 96kHz
		// change this if you require more
		static const int BUFFER_SIZE = 1024;	// buffer size (always a power of 2!)
		unsigned int mask_;						// buffer mask
		unsigned int cur_;						// cursor
		std::array< float, BUFFER_SIZE> outBuffer_[ 2 ];	// output buffer
		
	};	// end SimpleLimit class

}	// end namespace chunkware_simple

// include inlined process function
#include "SimpleLimitProcess.inl"

#endif	// end __SIMPLE_LIMIT_H__
