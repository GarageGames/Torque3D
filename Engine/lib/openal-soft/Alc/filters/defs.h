#ifndef ALC_FILTER_H
#define ALC_FILTER_H

#include "AL/al.h"
#include "math_defs.h"

/* Filters implementation is based on the "Cookbook formulae for audio
 * EQ biquad filter coefficients" by Robert Bristow-Johnson
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
/* Implementation note: For the shelf filters, the specified gain is for the
 * reference frequency, which is the centerpoint of the transition band. This
 * better matches EFX filter design. To set the gain for the shelf itself, use
 * the square root of the desired linear gain (or halve the dB gain).
 */

typedef enum BiquadType {
    /** EFX-style low-pass filter, specifying a gain and reference frequency. */
    BiquadType_HighShelf,
    /** EFX-style high-pass filter, specifying a gain and reference frequency. */
    BiquadType_LowShelf,
    /** Peaking filter, specifying a gain and reference frequency. */
    BiquadType_Peaking,

    /** Low-pass cut-off filter, specifying a cut-off frequency. */
    BiquadType_LowPass,
    /** High-pass cut-off filter, specifying a cut-off frequency. */
    BiquadType_HighPass,
    /** Band-pass filter, specifying a center frequency. */
    BiquadType_BandPass,
} BiquadType;

typedef struct BiquadFilter {
    ALfloat z1, z2; /* Last two delayed components for direct form II. */
    ALfloat b0, b1, b2; /* Transfer function coefficients "b" (numerator) */
    ALfloat a1, a2; /* Transfer function coefficients "a" (denominator; a0 is
                     * pre-applied). */
} BiquadFilter;
/* Currently only a C-based filter process method is implemented. */
#define BiquadFilter_process BiquadFilter_processC

/**
 * Calculates the rcpQ (i.e. 1/Q) coefficient for shelving filters, using the
 * reference gain and shelf slope parameter.
 * \param gain 0 < gain
 * \param slope 0 < slope <= 1
 */
inline ALfloat calc_rcpQ_from_slope(ALfloat gain, ALfloat slope)
{
    return sqrtf((gain + 1.0f/gain)*(1.0f/slope - 1.0f) + 2.0f);
}
/**
 * Calculates the rcpQ (i.e. 1/Q) coefficient for filters, using the normalized
 * reference frequency and bandwidth.
 * \param f0norm 0 < f0norm < 0.5.
 * \param bandwidth 0 < bandwidth
 */
inline ALfloat calc_rcpQ_from_bandwidth(ALfloat f0norm, ALfloat bandwidth)
{
    ALfloat w0 = F_TAU * f0norm;
    return 2.0f*sinhf(logf(2.0f)/2.0f*bandwidth*w0/sinf(w0));
}

inline void BiquadFilter_clear(BiquadFilter *filter)
{
    filter->z1 = 0.0f;
    filter->z2 = 0.0f;
}

/**
 * Sets up the filter state for the specified filter type and its parameters.
 *
 * \param filter The filter object to prepare.
 * \param type The type of filter for the object to apply.
 * \param gain The gain for the reference frequency response. Only used by the
 *             Shelf and Peaking filter types.
 * \param f0norm The normalized reference frequency (ref_freq / sample_rate).
 *               This is the center point for the Shelf, Peaking, and BandPass
 *               filter types, or the cutoff frequency for the LowPass and
 *               HighPass filter types.
 * \param rcpQ The reciprocal of the Q coefficient for the filter's transition
 *             band. Can be generated from calc_rcpQ_from_slope or
 *             calc_rcpQ_from_bandwidth depending on the available data.
 */
void BiquadFilter_setParams(BiquadFilter *filter, BiquadType type, ALfloat gain, ALfloat f0norm, ALfloat rcpQ);

inline void BiquadFilter_copyParams(BiquadFilter *restrict dst, const BiquadFilter *restrict src)
{
    dst->b0 = src->b0;
    dst->b1 = src->b1;
    dst->b2 = src->b2;
    dst->a1 = src->a1;
    dst->a2 = src->a2;
}

void BiquadFilter_processC(BiquadFilter *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALsizei numsamples);

inline void BiquadFilter_passthru(BiquadFilter *filter, ALsizei numsamples)
{
    if(LIKELY(numsamples >= 2))
    {
        filter->z1 = 0.0f;
        filter->z2 = 0.0f;
    }
    else if(numsamples == 1)
    {
        filter->z1 = filter->z2;
        filter->z2 = 0.0f;
    }
}

#endif /* ALC_FILTER_H */
