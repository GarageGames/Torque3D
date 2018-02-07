#include "config.h"

#include <xmmintrin.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alu.h"

#include "alSource.h"
#include "alAuxEffectSlot.h"
#include "mixer_defs.h"


const ALfloat *Resample_bsinc_SSE(const InterpState *state, const ALfloat *restrict src,
                                  ALsizei frac, ALint increment, ALfloat *restrict dst,
                                  ALsizei dstlen)
{
    const ALfloat *const filter = state->bsinc.filter;
    const __m128 sf4 = _mm_set1_ps(state->bsinc.sf);
    const ALsizei m = state->bsinc.m;
    const __m128 *fil, *scd, *phd, *spd;
    ALsizei pi, i, j, offset;
    ALfloat pf;
    __m128 r4;

    src += state->bsinc.l;
    for(i = 0;i < dstlen;i++)
    {
        // Calculate the phase index and factor.
#define FRAC_PHASE_BITDIFF (FRACTIONBITS-BSINC_PHASE_BITS)
        pi = frac >> FRAC_PHASE_BITDIFF;
        pf = (frac & ((1<<FRAC_PHASE_BITDIFF)-1)) * (1.0f/(1<<FRAC_PHASE_BITDIFF));
#undef FRAC_PHASE_BITDIFF

        offset = m*pi*4;
        fil = (const __m128*)ASSUME_ALIGNED(filter + offset, 16); offset += m;
        scd = (const __m128*)ASSUME_ALIGNED(filter + offset, 16); offset += m;
        phd = (const __m128*)ASSUME_ALIGNED(filter + offset, 16); offset += m;
        spd = (const __m128*)ASSUME_ALIGNED(filter + offset, 16);

        // Apply the scale and phase interpolated filter.
        r4 = _mm_setzero_ps();
        {
            const __m128 pf4 = _mm_set1_ps(pf);
#define MLA4(x, y, z) _mm_add_ps(x, _mm_mul_ps(y, z))
            for(j = 0;j < m;j+=4,fil++,scd++,phd++,spd++)
            {
                /* f = ((fil + sf*scd) + pf*(phd + sf*spd)) */
                const __m128 f4 = MLA4(
                    MLA4(*fil, sf4, *scd),
                    pf4, MLA4(*phd, sf4, *spd)
                );
                /* r += f*src */
                r4 = MLA4(r4, f4, _mm_loadu_ps(&src[j]));
            }
#undef MLA4
        }
        r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(0, 1, 2, 3)));
        r4 = _mm_add_ps(r4, _mm_movehl_ps(r4, r4));
        dst[i] = _mm_cvtss_f32(r4);

        frac += increment;
        src  += frac>>FRACTIONBITS;
        frac &= FRACTIONMASK;
    }
    return dst;
}


static inline void ApplyCoeffs(ALsizei Offset, ALfloat (*restrict Values)[2],
                               const ALsizei IrSize,
                               const ALfloat (*restrict Coeffs)[2],
                               ALfloat left, ALfloat right)
{
    const __m128 lrlr = _mm_setr_ps(left, right, left, right);
    __m128 vals = _mm_setzero_ps();
    __m128 coeffs;
    ALsizei i;

    Values = ASSUME_ALIGNED(Values, 16);
    Coeffs = ASSUME_ALIGNED(Coeffs, 16);
    if((Offset&1))
    {
        const ALsizei o0 = Offset&HRIR_MASK;
        const ALsizei o1 = (Offset+IrSize-1)&HRIR_MASK;
        __m128 imp0, imp1;

        coeffs = _mm_load_ps(&Coeffs[0][0]);
        vals = _mm_loadl_pi(vals, (__m64*)&Values[o0][0]);
        imp0 = _mm_mul_ps(lrlr, coeffs);
        vals = _mm_add_ps(imp0, vals);
        _mm_storel_pi((__m64*)&Values[o0][0], vals);
        for(i = 1;i < IrSize-1;i += 2)
        {
            const ALsizei o2 = (Offset+i)&HRIR_MASK;

            coeffs = _mm_load_ps(&Coeffs[i+1][0]);
            vals = _mm_load_ps(&Values[o2][0]);
            imp1 = _mm_mul_ps(lrlr, coeffs);
            imp0 = _mm_shuffle_ps(imp0, imp1, _MM_SHUFFLE(1, 0, 3, 2));
            vals = _mm_add_ps(imp0, vals);
            _mm_store_ps(&Values[o2][0], vals);
            imp0 = imp1;
        }
        vals = _mm_loadl_pi(vals, (__m64*)&Values[o1][0]);
        imp0 = _mm_movehl_ps(imp0, imp0);
        vals = _mm_add_ps(imp0, vals);
        _mm_storel_pi((__m64*)&Values[o1][0], vals);
    }
    else
    {
        for(i = 0;i < IrSize;i += 2)
        {
            const ALsizei o = (Offset + i)&HRIR_MASK;

            coeffs = _mm_load_ps(&Coeffs[i][0]);
            vals = _mm_load_ps(&Values[o][0]);
            vals = _mm_add_ps(vals, _mm_mul_ps(lrlr, coeffs));
            _mm_store_ps(&Values[o][0], vals);
        }
    }
}

#define MixHrtf MixHrtf_SSE
#define MixHrtfBlend MixHrtfBlend_SSE
#define MixDirectHrtf MixDirectHrtf_SSE
#include "mixer_inc.c"
#undef MixHrtf


void Mix_SSE(const ALfloat *data, ALsizei OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
             ALfloat *CurrentGains, const ALfloat *TargetGains, ALsizei Counter, ALsizei OutPos,
             ALsizei BufferSize)
{
    ALfloat gain, delta, step;
    __m128 gain4;
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
            /* Mix with applying gain steps in aligned multiples of 4. */
            if(minsize-pos > 3)
            {
                __m128 step4;
                gain4 = _mm_setr_ps(
                    gain,
                    gain + step,
                    gain + step + step,
                    gain + step + step + step
                );
                step4 = _mm_set1_ps(step + step + step + step);
                do {
                    const __m128 val4 = _mm_load_ps(&data[pos]);
                    __m128 dry4 = _mm_load_ps(&OutBuffer[c][OutPos+pos]);
                    dry4 = _mm_add_ps(dry4, _mm_mul_ps(val4, gain4));
                    gain4 = _mm_add_ps(gain4, step4);
                    _mm_store_ps(&OutBuffer[c][OutPos+pos], dry4);
                    pos += 4;
                } while(minsize-pos > 3);
                /* NOTE: gain4 now represents the next four gains after the
                 * last four mixed samples, so the lowest element represents
                 * the next gain to apply.
                 */
                gain = _mm_cvtss_f32(gain4);
            }
            /* Mix with applying left over gain steps that aren't aligned multiples of 4. */
            for(;pos < minsize;pos++)
            {
                OutBuffer[c][OutPos+pos] += data[pos]*gain;
                gain += step;
            }
            if(pos == Counter)
                gain = TargetGains[c];
            CurrentGains[c] = gain;

            /* Mix until pos is aligned with 4 or the mix is done. */
            minsize = mini(BufferSize, (pos+3)&~3);
            for(;pos < minsize;pos++)
                OutBuffer[c][OutPos+pos] += data[pos]*gain;
        }

        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;
        gain4 = _mm_set1_ps(gain);
        for(;BufferSize-pos > 3;pos += 4)
        {
            const __m128 val4 = _mm_load_ps(&data[pos]);
            __m128 dry4 = _mm_load_ps(&OutBuffer[c][OutPos+pos]);
            dry4 = _mm_add_ps(dry4, _mm_mul_ps(val4, gain4));
            _mm_store_ps(&OutBuffer[c][OutPos+pos], dry4);
        }
        for(;pos < BufferSize;pos++)
            OutBuffer[c][OutPos+pos] += data[pos]*gain;
    }
}

void MixRow_SSE(ALfloat *OutBuffer, const ALfloat *Gains, const ALfloat (*restrict data)[BUFFERSIZE], ALsizei InChans, ALsizei InPos, ALsizei BufferSize)
{
    __m128 gain4;
    ALsizei c;

    for(c = 0;c < InChans;c++)
    {
        ALsizei pos = 0;
        ALfloat gain = Gains[c];
        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;

        gain4 = _mm_set1_ps(gain);
        for(;BufferSize-pos > 3;pos += 4)
        {
            const __m128 val4 = _mm_load_ps(&data[c][InPos+pos]);
            __m128 dry4 = _mm_load_ps(&OutBuffer[pos]);
            dry4 = _mm_add_ps(dry4, _mm_mul_ps(val4, gain4));
            _mm_store_ps(&OutBuffer[pos], dry4);
        }
        for(;pos < BufferSize;pos++)
            OutBuffer[pos] += data[c][InPos+pos]*gain;
    }
}
