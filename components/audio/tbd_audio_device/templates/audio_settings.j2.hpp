#pragma once

/** @brief audio sample rate
 *
 *  All plugins and audio consumers can assume to be fed audio samples at this rate.
 *
 *  \f[
 *      r_{samples} = \frac{n_{samples}}{t}
 *  \f]
 */
#define TBD_SAMPLE_RATE {{sample_rate}}

/** @brief number of samples per channel for each audio processing run
 *
 *  Plugins or other audio consumers can expect this number of samples for each processing
 *  run.
 */
#define TBD_SAMPLES_PER_CHUNK {{chunk_size}}

/** @brief number of audio channels
 *
 *  Currently all TBD devices only support 2 channels (stereo in, stereo out).
 */
#define TBD_AUDIO_CHANNELS {{num_channels}}

/** @brief number of elements in chunk buffer
 *
 *  Total number of floats for both channels in audio chunk buffer.
 */
#define TBD_CHUNK_BUFFER_LENGTH TBD_SAMPLES_PER_CHUNK * TBD_AUDIO_CHANNELS

/** @brief chunk buffer size in bytes
 *
 *  Byte size of chunk buffer for memory copy operations.
 */
#define TBD_CHUNK_BUFFER_SIZE TBD_CHUNK_BUFFER_LENGTH * sizeof(float)
