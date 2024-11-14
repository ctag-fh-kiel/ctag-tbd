/*
 *	Simple Envelope Detectors (header)
 *
 *  File		: SimpleEnvelope.h
 *	Library		: SimpleSource
 *  Version		: 1.12
 *  Class		: EnvelopeDetector, AttRelEnvelope
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


#ifndef __SIMPLE_ENVELOPE_H__
#define __SIMPLE_ENVELOPE_H__

#include "SimpleHeader.h"	// common header

namespace chunkware_simple
{
	//-------------------------------------------------------------
	// DC offset (to prevent denormal)
	//-------------------------------------------------------------

	// USE:
	// 1. init envelope state to DC_OFFSET before processing
	// 2. add to input before envelope runtime function
	static const float DC_OFFSET = 1.0E-25;

	//-------------------------------------------------------------
	// envelope detector
	//-------------------------------------------------------------
	class EnvelopeDetector
	{
	public:
		EnvelopeDetector(
			  float ms = 1.0f
			, float sampleRate = 44100.0f
			);
		virtual ~EnvelopeDetector() {}

		// time constant
		virtual void   setTc( float ms );
		virtual float getTc( void ) const { return ms_; }

		// sample rate
		virtual void   setSampleRate( float sampleRate );
		virtual float getSampleRate( void ) const { return sampleRate_; }

		// runtime function
		INLINE void run( float in, float &state ) {
			state = in + coef_ * ( state - in );
		}

	protected:
			
		float sampleRate_;		// sample rate
		float ms_;				// time constant in ms
		float coef_;			// runtime coefficient
		virtual void setCoef( void );	// coef calculation

	};	// end SimpleComp class

	//-------------------------------------------------------------
	// attack/release envelope
	//-------------------------------------------------------------
	class AttRelEnvelope
	{
	public:
		AttRelEnvelope(
			  float att_ms = 10.0f
			, float rel_ms = 100.0f
			, float sampleRate = 44100.0f
			);
		virtual ~AttRelEnvelope() {}

		// attack time constant
		virtual void   setAttack( float ms );
		virtual float getAttack( void ) const { return att_.getTc(); }

		// release time constant
		virtual void   setRelease( float ms );
		virtual float getRelease( void ) const { return rel_.getTc(); }

		// sample rate dependencies
		virtual void   setSampleRate( float sampleRate );
		virtual float getSampleRate( void ) const { return att_.getSampleRate(); }

		// runtime function
		INLINE void run( float in, float &state ) {

			/* assumes that:
			* positive delta = attack
			* negative delta = release
			* good for linear & log values
			*/

			if ( in > state )
				att_.run( in, state );	// attack
			else
				rel_.run( in, state );	// release
		}

	private:
			
		EnvelopeDetector att_;
		EnvelopeDetector rel_;
		
	};	// end AttRelEnvelope class

}	// end namespace chunkware_simple

#endif	// end __SIMPLE_ENVELOPE_H__
