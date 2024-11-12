/*
 *	Simple Compressor (runtime function)
 *
 *  File		: SimpleCompProcess.inl
 *	Library		: SimpleSource
 *  Version		: 1.12
 *  Implements	: void SimpleComp::process( float &in1, float &in2 )
 *				  void SimpleComp::process( float &in1, float &in2, float keyLinked )
 *				  void SimpleCompRms::process( float &in1, float &in2 )
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


#ifndef __SIMPLE_COMP_PROCESS_INL__
#define __SIMPLE_COMP_PROCESS_INL__

namespace chunkware_simple
{
	//-------------------------------------------------------------
	INLINE void SimpleComp::process( float &in1, float &in2 )
	{
		// create sidechain

		float rect1 = fabsf( in1 );	// rectify input
		float rect2 = fabsf( in2 );

		/* if desired, one could use another EnvelopeDetector to smooth
		 * the rectified signal.
		 */

		float link = std::max( rect1, rect2 );	// link channels with greater of 2

		process( in1, in2, link );	// rest of process
	}

	//-------------------------------------------------------------
	INLINE void SimpleComp::process( float &in1, float &in2, float keyLinked )
	{
		//keyLinked = fabsf( keyLinked );		// rectify (just in case)

		// convert key to dB
		keyLinked += DC_OFFSET;				// add DC offset to avoid log( 0 )
		float keydB = lin2dB( keyLinked );	// convert linear -> dB

		// threshold
		float overdB = keydB - threshdB_;	// delta over threshold
		if ( overdB < 0.0f )
			overdB = 0.0f;

		// attack/release

		overdB += DC_OFFSET;					// add DC offset to avoid denormal
		AttRelEnvelope::run( overdB, envdB_ );	// run attack/release envelope
		overdB = envdB_ - DC_OFFSET;			// subtract DC offset

		/* REGARDING THE DC OFFSET: In this case, since the offset is added before 
		 * the attack/release processes, the envelope will never fall below the offset,
		 * thereby avoiding denormals. However, to prevent the offset from causing
		 * constant gain reduction, we must subtract it from the envelope, yielding
		 * a minimum value of 0dB.
		 */

		// transfer function
		float gr = overdB * ( ratio_ - 1.0f );	// gain reduction (dB)
		gr = dB2lin( gr );						// convert dB -> linear

		// output gain
		in1 *= gr;	// apply gain reduction to input
		in2 *= gr;
	}

	//-------------------------------------------------------------
	INLINE void SimpleCompRms::process( float &in1, float &in2 )
	{
		// create sidechain

		float inSq1 = in1 * in1;	// square input
		float inSq2 = in2 * in2;

		float sum = inSq1 + inSq2;			// power summing
		sum += DC_OFFSET;					// DC offset, to prevent denormal
		ave_.run( sum, aveOfSqrs_ );		// average of squares
		float rms = sqrt( aveOfSqrs_ );	// rms (sort of ...)

		/* REGARDING THE RMS AVERAGER: Ok, so this isn't a REAL RMS
		 * calculation. A true RMS is an FIR moving average. This
		 * approximation is a 1-pole IIR. Nonetheless, in practice,
		 * and in the interest of simplicity, this method will suffice,
		 * giving comparable results.
		 */

		SimpleComp::process( in1, in2, rms );	// rest of process
	}

}	// end namespace chunkware_simple

#endif	// end __SIMPLE_COMP_PROCESS_INL__
