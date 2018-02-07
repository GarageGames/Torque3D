#ifndef _AL_FILTER_H_
#define _AL_FILTER_H_

#include "alMain.h"

#include "math_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOWPASSFREQREF  (5000.0f)
#define HIGHPASSFREQREF  (250.0f)


/* Filters implementation is based on the "Cookbook formulae for audio
 * EQ biquad filter coefficients" by Robert Bristow-Johnson
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
/* Implementation note: For the shelf filters, the specified gain is for the
 * reference frequency, which is the centerpoint of the transition band. This
 * better matches EFX filter design. To set the gain for the shelf itself, use
 * the square root of the desired linear gain (or halve the dB gain).
 */

typedef enum ALfilterType {
    /** EFX-style low-pass filter, specifying a gain and reference frequency. */
    ALfilterType_HighShelf,
    /** EFX-style high-pass filter, specifying a gain and reference frequency. */
    ALfilterType_LowShelf,
    /** Peaking filter, specifying a gain and reference frequency. */
    ALfilterType_Peaking,

    /** Low-pass cut-off filter, specifying a cut-off frequency. */
    ALfilterType_LowPass,
    /** High-pass cut-off filter, specifying a cut-off frequency. */
    ALfilterType_HighPass,
    /** Band-pass filter, specifying a center frequency. */
    ALfilterType_BandPass,
} ALfilterType;

typedef struct ALfilterState {
    ALfloat x[2]; /* History of two last input samples  */
    ALfloat y[2]; /* History of two last output samples */
    ALfloat b0, b1, b2; /* Transfer function coefficients "b" */
    ALfloat a1, a2; /* Transfer function coefficients "a" (a0 is pre-applied) */
} ALfilterState;
/* Currently only a C-based filter process method is implemented. */
#define ALfilterState_process ALfilterState_processC

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

inline void ALfilterState_clear(ALfilterState *filter)
{
    filter->x[0] = 0.0f;
    filter->x[1] = 0.0f;
    filter->y[0] = 0.0f;
    filter->y[1] = 0.0f;
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
void ALfilterState_setParams(ALfilterState *filter, ALfilterType type, ALfloat gain, ALfloat f0norm, ALfloat rcpQ);

inline void ALfilterState_copyParams(ALfilterState *restrict dst, const ALfilterState *restrict src)
{
    dst->b0 = src->b0;
    dst->b1 = src->b1;
    dst->b2 = src->b2;
    dst->a1 = src->a1;
    dst->a2 = src->a2;
}

void ALfilterState_processC(ALfilterState *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALsizei numsamples);

inline void ALfilterState_processPassthru(ALfilterState *filter, const ALfloat *restrict src, ALsizei numsamples)
{
    if(numsamples >= 2)
    {
        filter->x[1] = src[numsamples-2];
        filter->x[0] = src[numsamples-1];
        filter->y[1] = src[numsamples-2];
        filter->y[0] = src[numsamples-1];
    }
    else if(numsamples == 1)
    {
        filter->x[1] = filter->x[0];
        filter->x[0] = src[0];
        filter->y[1] = filter->y[0];
        filter->y[0] = src[0];
    }
}


struct ALfilter;

typedef struct ALfilterVtable {
    void (*const setParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint val);
    void (*const setParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALint *vals);
    void (*const setParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val);
    void (*const setParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals);

    void (*const getParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *val);
    void (*const getParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *vals);
    void (*const getParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val);
    void (*const getParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals);
} ALfilterVtable;

#define DEFINE_ALFILTER_VTABLE(T)           \
const struct ALfilterVtable T##_vtable = {  \
    T##_setParami, T##_setParamiv,          \
    T##_setParamf, T##_setParamfv,          \
    T##_getParami, T##_getParamiv,          \
    T##_getParamf, T##_getParamfv,          \
}

typedef struct ALfilter {
    // Filter type (AL_FILTER_NULL, ...)
    ALenum type;

    ALfloat Gain;
    ALfloat GainHF;
    ALfloat HFReference;
    ALfloat GainLF;
    ALfloat LFReference;

    const struct ALfilterVtable *vtbl;

    /* Self ID */
    ALuint id;
} ALfilter;

void ReleaseALFilters(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
