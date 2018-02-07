#include "config.h"

#include <assert.h>

#include "alMain.h"
#include "alu.h"
#include "alSource.h"
#include "alAuxEffectSlot.h"


static inline ALfloat do_point(const ALfloat *restrict vals, ALsizei UNUSED(frac))
{ return vals[0]; }
static inline ALfloat do_lerp(const ALfloat *restrict vals, ALsizei frac)
{ return lerp(vals[0], vals[1], frac * (1.0f/FRACTIONONE)); }
static inline ALfloat do_cubic(const ALfloat *restrict vals, ALsizei frac)
{ return cubic(vals[0], vals[1], vals[2], vals[3], frac * (1.0f/FRACTIONONE)); }

const ALfloat *Resample_copy_C(const InterpState* UNUSED(state),
  const ALfloat *restrict src, ALsizei UNUSED(frac), ALint UNUSED(increment),
  ALfloat *restrict dst, ALsizei numsamples)
{
#if defined(HAVE_SSE) || defined(HAVE_NEON)
    /* Avoid copying the source data if it's aligned like the destination. */
    if((((intptr_t)src)&15) == (((intptr_t)dst)&15))
        return src;
#endif
    memcpy(dst, src, numsamples*sizeof(ALfloat));
    return dst;
}

#define DECL_TEMPLATE(Tag, Sampler, O)                                        \
const ALfloat *Resample_##Tag##_C(const InterpState* UNUSED(state),           \
  const ALfloat *restrict src, ALsizei frac, ALint increment,                 \
  ALfloat *restrict dst, ALsizei numsamples)                                  \
{                                                                             \
    ALsizei i;                                                                \
                                                                              \
    src -= O;                                                                 \
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

DECL_TEMPLATE(point, do_point, 0)
DECL_TEMPLATE(lerp, do_lerp, 0)
DECL_TEMPLATE(cubic, do_cubic, 1)

#undef DECL_TEMPLATE

const ALfloat *Resample_bsinc_C(const InterpState *state, const ALfloat *restrict src,
                                ALsizei frac, ALint increment, ALfloat *restrict dst,
                                ALsizei dstlen)
{
    const ALfloat *fil, *scd, *phd, *spd;
    const ALfloat *const filter = state->bsinc.filter;
    const ALfloat sf = state->bsinc.sf;
    const ALsizei m = state->bsinc.m;
    ALsizei j_f, pi, i;
    ALfloat pf, r;

    src += state->bsinc.l;
    for(i = 0;i < dstlen;i++)
    {
        // Calculate the phase index and factor.
#define FRAC_PHASE_BITDIFF (FRACTIONBITS-BSINC_PHASE_BITS)
        pi = frac >> FRAC_PHASE_BITDIFF;
        pf = (frac & ((1<<FRAC_PHASE_BITDIFF)-1)) * (1.0f/(1<<FRAC_PHASE_BITDIFF));
#undef FRAC_PHASE_BITDIFF

        fil = ASSUME_ALIGNED(filter + m*pi*4, 16);
        scd = ASSUME_ALIGNED(fil + m, 16);
        phd = ASSUME_ALIGNED(scd + m, 16);
        spd = ASSUME_ALIGNED(phd + m, 16);

        // Apply the scale and phase interpolated filter.
        r = 0.0f;
        for(j_f = 0;j_f < m;j_f++)
            r += (fil[j_f] + sf*scd[j_f] + pf*(phd[j_f] + sf*spd[j_f])) * src[j_f];
        dst[i] = r;

        frac += increment;
        src  += frac>>FRACTIONBITS;
        frac &= FRACTIONMASK;
    }
    return dst;
}


void ALfilterState_processC(ALfilterState *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALsizei numsamples)
{
    ALsizei i;
    if(LIKELY(numsamples > 1))
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


static inline void ApplyCoeffs(ALsizei Offset, ALfloat (*restrict Values)[2],
                               const ALsizei IrSize,
                               const ALfloat (*restrict Coeffs)[2],
                               ALfloat left, ALfloat right)
{
    ALsizei c;
    for(c = 0;c < IrSize;c++)
    {
        const ALsizei off = (Offset+c)&HRIR_MASK;
        Values[off][0] += Coeffs[c][0] * left;
        Values[off][1] += Coeffs[c][1] * right;
    }
}

#define MixHrtf MixHrtf_C
#define MixHrtfBlend MixHrtfBlend_C
#define MixDirectHrtf MixDirectHrtf_C
#include "mixer_inc.c"
#undef MixHrtf


void Mix_C(const ALfloat *data, ALsizei OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
           ALfloat *CurrentGains, const ALfloat *TargetGains, ALsizei Counter, ALsizei OutPos,
           ALsizei BufferSize)
{
    ALfloat gain, delta, step;
    ALsizei c;

    delta = (Counter > 0) ? 1.0f/(ALfloat)Counter : 0.0f;

    for(c = 0;c < OutChans;c++)
    {
        ALsizei pos = 0;
        gain = CurrentGains[c];
        step = (TargetGains[c] - gain) * delta;
        if(fabsf(step) > FLT_EPSILON)
        {
            ALsizei minsize = mini(BufferSize, Counter);
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
void MixRow_C(ALfloat *OutBuffer, const ALfloat *Gains, const ALfloat (*restrict data)[BUFFERSIZE], ALsizei InChans, ALsizei InPos, ALsizei BufferSize)
{
    ALsizei c, i;

    for(c = 0;c < InChans;c++)
    {
        ALfloat gain = Gains[c];
        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;

        for(i = 0;i < BufferSize;i++)
            OutBuffer[i] += data[c][InPos+i] * gain;
    }
}
