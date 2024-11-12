// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#ifndef _denormals_
#define _denormals_

#define undenormalise(sample) if(((* (uint32_t*)&sample ) & 0x7f800000 ) == 0) sample = 0.0f

#endif//_denormals_

//ends
/*
 * Denormal numbers
----------------

Here's a small recap on all proposed solutions to prevent the FPU from
denormalizing:
When you feed the FPU really small values (what's the exact 'threshold'
value?) the CPU will go into denormal mode to maintain precision; as such
precision isn't required for audio applications and denormal operations are
MUCH slower than normal operations, we want to avoid them.

All methods have been proposed by people other than me, and things I say
here may be inaccurate or completely wrong :). 'Algorithms' have not been
tested and can be implemented more efficiently no doubt.
If I made some horrible mistakes, or left some stuff out, please let me/the
list know.


** Checking denormal processing solution:

To detect if a denormal number has occured, just trace your code and
look up on the STAT FPU register ... if 0x0002 flag is set then a denormal
operation occured (this flag stays fixed until next FINIT)

** Checking denormal macro solution:

NOTES:

This is the least computationally efficient method of the lot, but has the
advantage of being inaudible.
Please note that in every feedback loop, you should also check for
denormals (rendering it useless on algorithms with loads of filters,
feedback loops, etc).

CODE:

#define IS_DENORMAL(f) (((*(unsigned int *)&f)&0x7f800000)==0)

// inner-loop:

is1 = *(++in1); is1 = IS_DENORMAL(is1) ? 0.f : is1;
is2 = *(++in2); is2 = IS_DENORMAL(is2) ? 0.f : is2;

** Adding noise solution:

NOTES:

Less overhead than the first solution, but still 2 mem accesses. Because a
number of the values of denormalBuf will be denormals themselves, there
will always be *some* denormal overhead. However, a small percentage
denormals probably isn't a problem.
Use this eq. to calculate the appropriate value of id (presuming rand()
generates evenly distrubuted values):
id = 1/percentageOfDenormalsAllowed * denormalThreshold
(I do not know the exact value of the denormalThreshold, the value at which
the FPU starts to denormal).

Possible additions to this algorithm include, noiseshaping the noise
buffer, which would allow a smaller value of percentageOfDenormalsAllowed
without becomming audible - however, in some algorithms, with filters and
such, I think this might cause the noise to be removed, thus rendering it
useless. Checking for denormals on noise generation might have a similar
effect I suspect.

CODE:

// on construction:
float **denormalBuf = new float[getBlockSize()];

float id = 1.0E-20;

for (int i = 0; i < getBlockSize(); i++)
{
   denormalBuf[i] = (float)rand()/32768.f * id;
}

// inner-loop:

float noise = *(++noiseBuf);
is1 = *(++in1) + noise;
is2 = *(++in2) + noise;
..

** Flipping number solution:

NOTES:

In my opinion the way-to-go method for most applications; very little
overhead (no compare/if/etc or memory access needed), there isn't a
percentage of the values that will denormal and it will be inaudible in
most cases.
The exact value of id will probably differ from algorithm to algorithm, but
the proposed value of 1.0E-30 seemed too small to me.

CODE:

// on construction:
float id = 1.0E-25;

// inner-loop:
is1 = *(++in1) + id;
is2 = *(++in2) + id;
id = -id;
..

** Adding offset solution:

NOTES:

This is the most efficient method of the lot and is also inaudible.
However, some filters will probably remove the added DC offset, thus
rendering it useless.

CODE:

// inner-loop:

is1 = *(++in1) + 1.0E-25;
is2 = *(++in2) + 1.0E-25;

** Fix-up solution

You can also walk through your filter and clamp any numbers which are close
enough to being denormal at the end of each block, as long as your blocks are
not too large. For instance, if you implement EQ using a bi-quad, you can check
the delay slots at the end of each process() call, and if any slot has a magnitude
of 10^-15 or smaller, you just clamp it to 0. This will ensure that your filter
doesn't run for long times with denormal numbers; ideally (depending on the
coefficients) it won't reach 10^-35 from the 10^-15 initial state within the time
of one block of samples.
That solution uses the least cycles, and also has the nice property of generating
absolute-0 output values for long stretches of absolute-0 input values; the others
don't.
 */
