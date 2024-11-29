#pragma once

#pragma once

/** @brief audio sample rate
*
 *  All plugins and audio consumers can assume to be fed audio samples at this rate.
 *
 *  \f[
 *      r_{samples} = \frac{n_{samples}}{t}
 *  \f]
 */
#define TBD_SAMPLE_RATE 44100

/** @brief number of samples per channel for each audio processing run
 *
 *  Plugins or other audio consumers can expect this number of samples for each processing
 *  run.
 */
#define TBD_SAMPLES_PER_CHUNK 32

#define NG_OPEN 0
#define NG_BOTH 1
#define NG_LEFT 2
#define NG_RIGHT 3

namespace CTAG::AUDIO {

extern const char* tag;

}