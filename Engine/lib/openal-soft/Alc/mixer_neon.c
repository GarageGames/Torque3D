#include "config.h"

#include <arm_neon.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alu.h"
#include "hrtf.h"


static inline void ApplyCoeffsStep(ALuint Offset, ALfloat (*restrict Values)[2],
                                   const ALuint IrSize,
                                   ALfloat (*restrict Coeffs)[2],
                                   const ALfloat (*restrict CoeffStep)[2],
                                   ALfloat left, ALfloat right)
{
    ALuint c;
    float32x4_t leftright4;
    {
        float32x2_t leftright2 = vdup_n_f32(0.0);
        leftright2 = vset_lane_f32(left, leftright2, 0);
        leftright2 = vset_lane_f32(right, leftright2, 1);
        leftright4 = vcombine_f32(leftright2, leftright2);
    }
    for(c = 0;c < IrSize;c += 2)
    {
        const ALuint o0 = (Offset+c)&HRIR_MASK;
        const ALuint o1 = (o0+1)&HRIR_MASK;
        float32x4_t vals = vcombine_f32(vld1_f32((float32_t*)&Values[o0][0]),
                                        vld1_f32((float32_t*)&Values[o1][0]));
        float32x4_t coefs = vld1q_f32((float32_t*)&Coeffs[c][0]);
        float32x4_t deltas = vld1q_f32(&CoeffStep[c][0]);

        vals = vmlaq_f32(vals, coefs, leftright4);
        coefs = vaddq_f32(coefs, deltas);

        vst1_f32((float32_t*)&Values[o0][0], vget_low_f32(vals));
        vst1_f32((float32_t*)&Values[o1][0], vget_high_f32(vals));
        vst1q_f32(&Coeffs[c][0], coefs);
    }
}

static inline void ApplyCoeffs(ALuint Offset, ALfloat (*restrict Values)[2],
                               const ALuint IrSize,
                               ALfloat (*restrict Coeffs)[2],
                               ALfloat left, ALfloat right)
{
    ALuint c;
    float32x4_t leftright4;
    {
        float32x2_t leftright2 = vdup_n_f32(0.0);
        leftright2 = vset_lane_f32(left, leftright2, 0);
        leftright2 = vset_lane_f32(right, leftright2, 1);
        leftright4 = vcombine_f32(leftright2, leftright2);
    }
    for(c = 0;c < IrSize;c += 2)
    {
        const ALuint o0 = (Offset+c)&HRIR_MASK;
        const ALuint o1 = (o0+1)&HRIR_MASK;
        float32x4_t vals = vcombine_f32(vld1_f32((float32_t*)&Values[o0][0]),
                                        vld1_f32((float32_t*)&Values[o1][0]));
        float32x4_t coefs = vld1q_f32((float32_t*)&Coeffs[c][0]);

        vals = vmlaq_f32(vals, coefs, leftright4);

        vst1_f32((float32_t*)&Values[o0][0], vget_low_f32(vals));
        vst1_f32((float32_t*)&Values[o1][0], vget_high_f32(vals));
    }
}

#define MixHrtf MixHrtf_Neon
#define MixDirectHrtf MixDirectHrtf_Neon
#include "mixer_inc.c"
#undef MixHrtf


void Mix_Neon(const ALfloat *data, ALuint OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
              ALfloat *CurrentGains, const ALfloat *TargetGains, ALuint Counter, ALuint OutPos,
              ALuint BufferSize)
{
    ALfloat gain, delta, step;
    float32x4_t gain4;
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
            minsize = minu(BufferSize, (pos+3)&~3);
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

void MixRow_Neon(ALfloat *OutBuffer, const ALfloat *Gains, const ALfloat (*restrict data)[BUFFERSIZE], ALuint InChans, ALuint InPos, ALuint BufferSize)
{
    float32x4_t gain4;
    ALuint c;

    for(c = 0;c < InChans;c++)
    {
        ALuint pos = 0;
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
