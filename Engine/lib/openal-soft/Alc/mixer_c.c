#include "config.h"

#include <assert.h>

#include "alMain.h"
#include "alu.h"
#include "alSource.h"
#include "alAuxEffectSlot.h"


static inline ALfloat point32(const ALfloat *restrict vals, ALuint UNUSED(frac))
{ return vals[0]; }
static inline ALfloat lerp32(const ALfloat *restrict vals, ALuint frac)
{ return lerp(vals[0], vals[1], frac * (1.0f/FRACTIONONE)); }
static inline ALfloat fir4_32(const ALfloat *restrict vals, ALuint frac)
{ return resample_fir4(vals[-1], vals[0], vals[1], vals[2], frac); }
static inline ALfloat fir8_32(const ALfloat *restrict vals, ALuint frac)
{ return resample_fir8(vals[-3], vals[-2], vals[-1], vals[0], vals[1], vals[2], vals[3], vals[4], frac); }


const ALfloat *Resample_copy32_C(const BsincState* UNUSED(state), const ALfloat *restrict src, ALuint UNUSED(frac),
  ALuint UNUSED(increment), ALfloat *restrict dst, ALuint numsamples)
{
#if defined(HAVE_SSE) || defined(HAVE_NEON)
    /* Avoid copying the source data if it's aligned like the destination. */
    if((((intptr_t)src)&15) == (((intptr_t)dst)&15))
        return src;
#endif
    memcpy(dst, src, numsamples*sizeof(ALfloat));
    return dst;
}

#define DECL_TEMPLATE(Sampler)                                                \
const ALfloat *Resample_##Sampler##_C(const BsincState* UNUSED(state),        \
  const ALfloat *restrict src, ALuint frac, ALuint increment,                 \
  ALfloat *restrict dst, ALuint numsamples)                                   \
{                                                                             \
    ALuint i;                                                                 \
    for(i = 0;i < numsamples;i++)                                             \
    {                                                                         \
        dst[i] = Sampler(src, frac);                                          \
                                                                              \
        frac += increment;                                                    \
        src  += frac>>FRACTIONBITS;                                           \
        frac &= FRACTIONMASK;                                                 \
    }                                                                         \
    return dst;                                                               \
}

DECL_TEMPLATE(point32)
DECL_TEMPLATE(lerp32)
DECL_TEMPLATE(fir4_32)
DECL_TEMPLATE(fir8_32)

#undef DECL_TEMPLATE

const ALfloat *Resample_bsinc32_C(const BsincState *state, const ALfloat *restrict src,
                                  ALuint frac, ALuint increment, ALfloat *restrict dst,
                                  ALuint dstlen)
{
    const ALfloat *fil, *scd, *phd, *spd;
    const ALfloat sf = state->sf;
    const ALuint m = state->m;
    const ALint l = state->l;
    ALuint j_f, pi, i;
    ALfloat pf, r;
    ALint j_s;

    for(i = 0;i < dstlen;i++)
    {
        // Calculate the phase index and factor.
#define FRAC_PHASE_BITDIFF (FRACTIONBITS-BSINC_PHASE_BITS)
        pi = frac >> FRAC_PHASE_BITDIFF;
        pf = (frac & ((1<<FRAC_PHASE_BITDIFF)-1)) * (1.0f/(1<<FRAC_PHASE_BITDIFF));
#undef FRAC_PHASE_BITDIFF

        fil = state->coeffs[pi].filter;
        scd = state->coeffs[pi].scDelta;
        phd = state->coeffs[pi].phDelta;
        spd = state->coeffs[pi].spDelta;

        // Apply the scale and phase interpolated filter.
        r = 0.0f;
        for(j_f = 0,j_s = l;j_f < m;j_f++,j_s++)
            r += (fil[j_f] + sf*scd[j_f] + pf*(phd[j_f] + sf*spd[j_f])) *
                    src[j_s];
        dst[i] = r;

        frac += increment;
        src  += frac>>FRACTIONBITS;
        frac &= FRACTIONMASK;
    }
    return dst;
}


void ALfilterState_processC(ALfilterState *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALuint numsamples)
{
    ALuint i;
    if(numsamples > 1)
    {
        dst[0] = filter->b0 * src[0] +
                 filter->b1 * filter->x[0] +
                 filter->b2 * filter->x[1] -
                 filter->a1 * filter->y[0] -
                 filter->a2 * filter->y[1];
        dst[1] = filter->b0 * src[1] +
                 filter->b1 * src[0] +
                 filter->b2 * filter->x[0] -
                 filter->a1 * dst[0] -
                 filter->a2 * filter->y[0];
        for(i = 2;i < numsamples;i++)
            dst[i] = filter->b0 * src[i] +
                     filter->b1 * src[i-1] +
                     filter->b2 * src[i-2] -
                     filter->a1 * dst[i-1] -
                     filter->a2 * dst[i-2];
        filter->x[0] = src[i-1];
        filter->x[1] = src[i-2];
        filter->y[0] = dst[i-1];
        filter->y[1] = dst[i-2];
    }
    else if(numsamples == 1)
    {
        dst[0] = filter->b0 * src[0] +
                 filter->b1 * filter->x[0] +
                 filter->b2 * filter->x[1] -
                 filter->a1 * filter->y[0] -
                 filter->a2 * filter->y[1];
        filter->x[1] = filter->x[0];
        filter->x[0] = src[0];
        filter->y[1] = filter->y[0];
        filter->y[0] = dst[0];
    }
}


static inline void ApplyCoeffsStep(ALuint Offset, ALfloat (*restrict Values)[2],
                                   const ALuint IrSize,
                                   ALfloat (*restrict Coeffs)[2],
                                   const ALfloat (*restrict CoeffStep)[2],
                                   ALfloat left, ALfloat right)
{
    ALuint c;
    for(c = 0;c < IrSize;c++)
    {
        const ALuint off = (Offset+c)&HRIR_MASK;
        Values[off][0] += Coeffs[c][0] * left;
        Values[off][1] += Coeffs[c][1] * right;
        Coeffs[c][0] += CoeffStep[c][0];
        Coeffs[c][1] += CoeffStep[c][1];
    }
}

static inline void ApplyCoeffs(ALuint Offset, ALfloat (*restrict Values)[2],
                               const ALuint IrSize,
                               ALfloat (*restrict Coeffs)[2],
                               ALfloat left, ALfloat right)
{
    ALuint c;
    for(c = 0;c < IrSize;c++)
    {
        const ALuint off = (Offset+c)&HRIR_MASK;
        Values[off][0] += Coeffs[c][0] * left;
        Values[off][1] += Coeffs[c][1] * right;
    }
}

#define MixHrtf MixHrtf_C
#define MixDirectHrtf MixDirectHrtf_C
#include "mixer_inc.c"
#undef MixHrtf


void Mix_C(const ALfloat *data, ALuint OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
           ALfloat *CurrentGains, const ALfloat *TargetGains, ALuint Counter, ALuint OutPos,
           ALuint BufferSize)
{
    ALfloat gain, delta, step;
    ALuint c;

    delta = (Counter > 0) ? 1.0f/(ALfloat)Counter : 0.0f;

    for(c = 0;c < OutChans;c++)
    {
        ALuint pos = 0;
        gain = CurrentGains[c];
        step = (TargetGains[c] - gain) * delta;
        if(fabsf(step) > FLT_EPSILON)
        {
            ALuint minsize = minu(BufferSize, Counter);
            for(;pos < minsize;pos++)
            {
                OutBuffer[c][OutPos+pos] += data[pos]*gain;
                gain += step;
            }
            if(pos == Counter)
                gain = TargetGains[c];
            CurrentGains[c] = gain;
        }

        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;
        for(;pos < BufferSize;pos++)
            OutBuffer[c][OutPos+pos] += data[pos]*gain;
    }
}

/* Basically the inverse of the above. Rather than one input going to multiple
 * outputs (each with its own gain), it's multiple inputs (each with its own
 * gain) going to one output. This applies one row (vs one column) of a matrix
 * transform. And as the matrices are more or less static once set up, no
 * stepping is necessary.
 */
void MixRow_C(ALfloat *OutBuffer, const ALfloat *Gains, const ALfloat (*restrict data)[BUFFERSIZE], ALuint InChans, ALuint InPos, ALuint BufferSize)
{
    ALuint c, i;

    for(c = 0;c < InChans;c++)
    {
        ALfloat gain = Gains[c];
        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;

        for(i = 0;i < BufferSize;i++)
            OutBuffer[i] += data[c][InPos+i] * gain;
    }
}
