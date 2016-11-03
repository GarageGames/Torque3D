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
    ALfloat a1, a2; /* Transfer function coefficients "a" (a0 is pre-applied) */
    ALfloat b0, b1, b2; /* Transfer function coefficients "b" */
} ALfilterState;
/* Currently only a C-based filter process method is implemented. */
#define ALfilterState_process ALfilterState_processC

/* Calculates the rcpQ (i.e. 1/Q) coefficient for shelving filters, using the
 * reference gain and shelf slope parameter.
 * 0 < gain
 * 0 < slope <= 1
 */
inline ALfloat calc_rcpQ_from_slope(ALfloat gain, ALfloat slope)
{
    return sqrtf((gain + 1.0f/gain)*(1.0f/slope - 1.0f) + 2.0f);
}
/* Calculates the rcpQ (i.e. 1/Q) coefficient for filters, using the frequency
 * multiple (i.e. ref_freq / sampling_freq) and bandwidth.
 * 0 < freq_mult < 0.5.
 */
inline ALfloat calc_rcpQ_from_bandwidth(ALfloat freq_mult, ALfloat bandwidth)
{
    ALfloat w0 = F_TAU * freq_mult;
    return 2.0f*sinhf(logf(2.0f)/2.0f*bandwidth*w0/sinf(w0));
}

inline void ALfilterState_clear(ALfilterState *filter)
{
    filter->x[0] = 0.0f;
    filter->x[1] = 0.0f;
    filter->y[0] = 0.0f;
    filter->y[1] = 0.0f;
}

void ALfilterState_setParams(ALfilterState *filter, ALfilterType type, ALfloat gain, ALfloat freq_mult, ALfloat rcpQ);

void ALfilterState_processC(ALfilterState *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALuint numsamples);

inline void ALfilterState_processPassthru(ALfilterState *filter, const ALfloat *restrict src, ALuint numsamples)
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


typedef struct ALfilter {
    // Filter type (AL_FILTER_NULL, ...)
    ALenum type;

    ALfloat Gain;
    ALfloat GainHF;
    ALfloat HFReference;
    ALfloat GainLF;
    ALfloat LFReference;

    void (*SetParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint val);
    void (*SetParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALint *vals);
    void (*SetParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val);
    void (*SetParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals);

    void (*GetParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *val);
    void (*GetParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *vals);
    void (*GetParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val);
    void (*GetParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals);

    /* Self ID */
    ALuint id;
} ALfilter;

#define ALfilter_SetParami(x, c, p, v)  ((x)->SetParami((x),(c),(p),(v)))
#define ALfilter_SetParamiv(x, c, p, v) ((x)->SetParamiv((x),(c),(p),(v)))
#define ALfilter_SetParamf(x, c, p, v)  ((x)->SetParamf((x),(c),(p),(v)))
#define ALfilter_SetParamfv(x, c, p, v) ((x)->SetParamfv((x),(c),(p),(v)))

#define ALfilter_GetParami(x, c, p, v)  ((x)->GetParami((x),(c),(p),(v)))
#define ALfilter_GetParamiv(x, c, p, v) ((x)->GetParamiv((x),(c),(p),(v)))
#define ALfilter_GetParamf(x, c, p, v)  ((x)->GetParamf((x),(c),(p),(v)))
#define ALfilter_GetParamfv(x, c, p, v) ((x)->GetParamfv((x),(c),(p),(v)))

inline void LockFiltersRead(ALCdevice *device)
{ LockUIntMapRead(&device->FilterMap); }
inline void UnlockFiltersRead(ALCdevice *device)
{ UnlockUIntMapRead(&device->FilterMap); }
inline void LockFiltersWrite(ALCdevice *device)
{ LockUIntMapWrite(&device->FilterMap); }
inline void UnlockFiltersWrite(ALCdevice *device)
{ UnlockUIntMapWrite(&device->FilterMap); }

inline struct ALfilter *LookupFilter(ALCdevice *device, ALuint id)
{ return (struct ALfilter*)LookupUIntMapKeyNoLock(&device->FilterMap, id); }
inline struct ALfilter *RemoveFilter(ALCdevice *device, ALuint id)
{ return (struct ALfilter*)RemoveUIntMapKeyNoLock(&device->FilterMap, id); }

ALvoid ReleaseALFilters(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
