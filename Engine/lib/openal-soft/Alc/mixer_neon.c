#include "config.h"

#include <arm_neon.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alu.h"
#include "hrtf.h"
#include "mixer_defs.h"


const ALfloat *Resample_lerp_Neon(const InterpState* UNUSED(state),
  const ALfloat *restrict src, ALsizei frac, ALint increment,
  ALfloat *restrict dst, ALsizei numsamples)
{
    const int32x4_t increment4 = vdupq_n_s32(increment*4);
    const float32x4_t fracOne4 = vdupq_n_f32(1.0f/FRACTIONONE);
    const int32x4_t fracMask4 = vdupq_n_s32(FRACTIONMASK);
    alignas(16) ALint pos_[4];
    alignas(16) ALsizei frac_[4];
    int32x4_t pos4;
    int32x4_t frac4;
    ALsizei i;

    InitiatePositionArrays(frac, increment, frac_, pos_, 4);

    frac4 = vld1q_s32(frac_);
    pos4 = vld1q_s32(pos_);

    for(i = 0;numsamples-i > 3;i += 4)
    {
        const float32x4_t val1 = (float32x4_t){src[pos_[0]], src[pos_[1]], src[pos_[2]], src[pos_[3]]};
        const float32x4_t val2 = (float32x4_t){src[pos_[0]+1], src[pos_[1]+1], src[pos_[2]+1], src[pos_[3]+1]};

        /* val1 + (val2-val1)*mu */
        const float32x4_t r0 = vsubq_f32(val2, val1);
        const float32x4_t mu = vmulq_f32(vcvtq_f32_s32(frac4), fracOne4);
        const float32x4_t out = vmlaq_f32(val1, mu, r0);

        vst1q_f32(&dst[i], out);

        frac4 = vaddq_s32(frac4, increment4);
        pos4 = vaddq_s32(pos4, vshrq_n_s32(frac4, FRACTIONBITS));
        frac4 = vandq_s32(frac4, fracMask4);

        vst1q_s32(pos_, pos4);
    }

    if(i < numsamples)
    {
        /* NOTE: These four elements represent the position *after* the last
         * four samples, so the lowest element is the next position to
         * resample.
         */
        ALint pos = pos_[0];
        frac = vgetq_lane_s32(frac4, 0);
        do {
            dst[i] = lerp(src[pos], src[pos+1], frac * (1.0f/FRACTIONONE));

            frac += increment;
            pos  += frac>>FRACTIONBITS;
            frac &= FRACTIONMASK;
        } while(++i < numsamples);
    }
    return dst;
}

const ALfloat *Resample_bsinc_Neon(const InterpState *state,
  const ALfloat *restrict src, ALsizei frac, ALint increment,
  ALfloat *restrict dst, ALsizei dstlen)
{
    const ALfloat *const filter = state->bsinc.filter;
    const float32x4_t sf4 = vdupq_n_f32(state->bsinc.sf);
    const ALsizei m = state->bsinc.m;
    const float32x4_t *fil, *scd, *phd, *spd;
    ALsizei pi, i, j, offset;
    float32x4_t r4;
    ALfloat pf;

    src += state->bsinc.l;
    for(i = 0;i < dstlen;i++)
    {
        // Calculate the phase index and factor.
#define FRAC_PHASE_BITDIFF (FRACTIONBITS-BSINC_PHASE_BITS)
        pi = frac >> FRAC_PHASE_BITDIFF;
        pf = (frac & ((1<<FRAC_PHASE_BITDIFF)-1)) * (1.0f/(1<<FRAC_PHASE_BITDIFF));
#undef FRAC_PHASE_BITDIFF

        offset = m*pi*4;
        fil = ASSUME_ALIGNED(filter + offset, 16); offset += m;
        scd = ASSUME_ALIGNED(filter + offset, 16); offset += m;
        phd = ASSUME_ALIGNED(filter + offset, 16); offset += m;
        spd = ASSUME_ALIGNED(filter + offset, 16);

        // Apply the scale and phase interpolated filter.
        r4 = vdupq_n_f32(0.0f);
        {
            const float32x4_t pf4 = vdupq_n_f32(pf);
            for(j = 0;j < m;j+=4,fil++,scd++,phd++,spd++)
            {
                /* f = ((fil + sf*scd) + pf*(phd + sf*spd)) */
                const float32x4_t f4 = vmlaq_f32(
                    vmlaq_f32(*fil, sf4, *scd),
                    pf4, vmlaq_f32(*phd, sf4, *spd)
                );
                /* r += f*src */
                r4 = vmlaq_f32(r4, f4, vld1q_f32(&src[j]));
            }
        }
        r4 = vaddq_f32(r4, vcombine_f32(vrev64_f32(vget_high_f32(r4)),
                                        vrev64_f32(vget_low_f32(r4))));
        dst[i] = vget_lane_f32(vadd_f32(vget_low_f32(r4), vget_high_f32(r4)), 0);

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
    ALsizei c;
    float32x4_t leftright4;
    {
        float32x2_t leftright2 = vdup_n_f32(0.0);
        leftright2 = vset_lane_f32(left, leftright2, 0);
        leftright2 = vset_lane_f32(right, leftright2, 1);
        leftright4 = vcombine_f32(leftright2, leftright2);
    }
    Values = ASSUME_ALIGNED(Values, 16);
    Coeffs = ASSUME_ALIGNED(Coeffs, 16);
    for(c = 0;c < IrSize;c += 2)
    {
        const ALsizei o0 = (Offset+c)&HRIR_MASK;
        const ALsizei o1 = (o0+1)&HRIR_MASK;
        float32x4_t vals = vcombine_f32(vld1_f32((float32_t*)&Values[o0][0]),
                                        vld1_f32((float32_t*)&Values[o1][0]));
        float32x4_t coefs = vld1q_f32((float32_t*)&Coeffs[c][0]);

        vals = vmlaq_f32(vals, coefs, leftright4);

        vst1_f32((float32_t*)&Values[o0][0], vget_low_f32(vals));
        vst1_f32((float32_t*)&Values[o1][0], vget_high_f32(vals));
    }
}

#define MixHrtf MixHrtf_Neon
#define MixHrtfBlend MixHrtfBlend_Neon
#define MixDirectHrtf MixDirectHrtf_Neon
#include "mixer_inc.c"
#undef MixHrtf


void Mix_Neon(const ALfloat *data, ALsizei OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
              ALfloat *CurrentGains, const ALfloat *TargetGains, ALsizei Counter, ALsizei OutPos,
              ALsizei BufferSize)
{
    ALfloat gain, delta, step;
    float32x4_t gain4;
    ALsizei c;

    data = ASSUME_ALIGNED(data, 16);
    OutBuffer = ASSUME_ALIGNED(OutBuffer, 16);

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
                float32x4_t step4;
                gain4 = vsetq_lane_f32(gain, gain4, 0);
                gain4 = vsetq_lane_f32(gain + step, gain4, 1);
                gain4 = vsetq_lane_f32(gain + step + step, gain4, 2);
                gain4 = vsetq_lane_f32(gain + step + step + step, gain4, 3);
                step4 = vdupq_n_f32(step + step + step + step);
                do {
                    const float32x4_t val4 = vld1q_f32(&data[pos]);
                    float32x4_t dry4 = vld1q_f32(&OutBuffer[c][OutPos+pos]);
                    dry4 = vmlaq_f32(dry4, val4, gain4);
                    gain4 = vaddq_f32(gain4, step4);
                    vst1q_f32(&OutBuffer[c][OutPos+pos], dry4);
                    pos += 4;
                } while(minsize-pos > 3);
                /* NOTE: gain4 now represents the next four gains after the
                 * last four mixed samples, so the lowest element represents
                 * the next gain to apply.
                 */
                gain = vgetq_lane_f32(gain4, 0);
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
        gain4 = vdupq_n_f32(gain);
        for(;BufferSize-pos > 3;pos += 4)
        {
            const float32x4_t val4 = vld1q_f32(&data[pos]);
            float32x4_t dry4 = vld1q_f32(&OutBuffer[c][OutPos+pos]);
            dry4 = vmlaq_f32(dry4, val4, gain4);
            vst1q_f32(&OutBuffer[c][OutPos+pos], dry4);
        }
        for(;pos < BufferSize;pos++)
            OutBuffer[c][OutPos+pos] += data[pos]*gain;
    }
}

void MixRow_Neon(ALfloat *OutBuffer, const ALfloat *Gains, const ALfloat (*restrict data)[BUFFERSIZE], ALsizei InChans, ALsizei InPos, ALsizei BufferSize)
{
    float32x4_t gain4;
    ALsizei c;

    data = ASSUME_ALIGNED(data, 16);
    OutBuffer = ASSUME_ALIGNED(OutBuffer, 16);

    for(c = 0;c < InChans;c++)
    {
        ALsizei pos = 0;
        ALfloat gain = Gains[c];
        if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
            continue;

        gain4 = vdupq_n_f32(gain);
        for(;BufferSize-pos > 3;pos += 4)
        {
            const float32x4_t val4 = vld1q_f32(&data[c][InPos+pos]);
            float32x4_t dry4 = vld1q_f32(&OutBuffer[pos]);
            dry4 = vmlaq_f32(dry4, val4, gain4);
            vst1q_f32(&OutBuffer[pos], dry4);
        }
        for(;pos < BufferSize;pos++)
            OutBuffer[pos] += data[c][InPos+pos]*gain;
    }
}
