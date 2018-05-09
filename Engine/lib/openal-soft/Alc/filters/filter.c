
#include "config.h"

#include "AL/alc.h"
#include "AL/al.h"

#include "alMain.h"
#include "defs.h"

extern inline void BiquadFilter_clear(BiquadFilter *filter);
extern inline void BiquadFilter_copyParams(BiquadFilter *restrict dst, const BiquadFilter *restrict src);
extern inline void BiquadFilter_passthru(BiquadFilter *filter, ALsizei numsamples);
extern inline ALfloat calc_rcpQ_from_slope(ALfloat gain, ALfloat slope);
extern inline ALfloat calc_rcpQ_from_bandwidth(ALfloat f0norm, ALfloat bandwidth);


void BiquadFilter_setParams(BiquadFilter *filter, BiquadType type, ALfloat gain, ALfloat f0norm, ALfloat rcpQ)
{
    ALfloat alpha, sqrtgain_alpha_2;
    ALfloat w0, sin_w0, cos_w0;
    ALfloat a[3] = { 1.0f, 0.0f, 0.0f };
    ALfloat b[3] = { 1.0f, 0.0f, 0.0f };

    // Limit gain to -100dB
    assert(gain > 0.00001f);

    w0 = F_TAU * f0norm;
    sin_w0 = sinf(w0);
    cos_w0 = cosf(w0);
    alpha = sin_w0/2.0f * rcpQ;

    /* Calculate filter coefficients depending on filter type */
    switch(type)
    {
        case BiquadType_HighShelf:
            sqrtgain_alpha_2 = 2.0f * sqrtf(gain) * alpha;
            b[0] =       gain*((gain+1.0f) + (gain-1.0f)*cos_w0 + sqrtgain_alpha_2);
            b[1] = -2.0f*gain*((gain-1.0f) + (gain+1.0f)*cos_w0                   );
            b[2] =       gain*((gain+1.0f) + (gain-1.0f)*cos_w0 - sqrtgain_alpha_2);
            a[0] =             (gain+1.0f) - (gain-1.0f)*cos_w0 + sqrtgain_alpha_2;
            a[1] =  2.0f*     ((gain-1.0f) - (gain+1.0f)*cos_w0                   );
            a[2] =             (gain+1.0f) - (gain-1.0f)*cos_w0 - sqrtgain_alpha_2;
            break;
        case BiquadType_LowShelf:
            sqrtgain_alpha_2 = 2.0f * sqrtf(gain) * alpha;
            b[0] =       gain*((gain+1.0f) - (gain-1.0f)*cos_w0 + sqrtgain_alpha_2);
            b[1] =  2.0f*gain*((gain-1.0f) - (gain+1.0f)*cos_w0                   );
            b[2] =       gain*((gain+1.0f) - (gain-1.0f)*cos_w0 - sqrtgain_alpha_2);
            a[0] =             (gain+1.0f) + (gain-1.0f)*cos_w0 + sqrtgain_alpha_2;
            a[1] = -2.0f*     ((gain-1.0f) + (gain+1.0f)*cos_w0                   );
            a[2] =             (gain+1.0f) + (gain-1.0f)*cos_w0 - sqrtgain_alpha_2;
            break;
        case BiquadType_Peaking:
            gain = sqrtf(gain);
            b[0] =  1.0f + alpha * gain;
            b[1] = -2.0f * cos_w0;
            b[2] =  1.0f - alpha * gain;
            a[0] =  1.0f + alpha / gain;
            a[1] = -2.0f * cos_w0;
            a[2] =  1.0f - alpha / gain;
            break;

        case BiquadType_LowPass:
            b[0] = (1.0f - cos_w0) / 2.0f;
            b[1] =  1.0f - cos_w0;
            b[2] = (1.0f - cos_w0) / 2.0f;
            a[0] =  1.0f + alpha;
            a[1] = -2.0f * cos_w0;
            a[2] =  1.0f - alpha;
            break;
        case BiquadType_HighPass:
            b[0] =  (1.0f + cos_w0) / 2.0f;
            b[1] = -(1.0f + cos_w0);
            b[2] =  (1.0f + cos_w0) / 2.0f;
            a[0] =   1.0f + alpha;
            a[1] =  -2.0f * cos_w0;
            a[2] =   1.0f - alpha;
            break;
        case BiquadType_BandPass:
            b[0] =  alpha;
            b[1] =  0;
            b[2] = -alpha;
            a[0] =  1.0f + alpha;
            a[1] = -2.0f * cos_w0;
            a[2] =  1.0f - alpha;
            break;
    }

    filter->a1 = a[1] / a[0];
    filter->a2 = a[2] / a[0];
    filter->b0 = b[0] / a[0];
    filter->b1 = b[1] / a[0];
    filter->b2 = b[2] / a[0];
}


void BiquadFilter_processC(BiquadFilter *filter, ALfloat *restrict dst, const ALfloat *restrict src, ALsizei numsamples)
{
    const ALfloat a1 = filter->a1;
    const ALfloat a2 = filter->a2;
    const ALfloat b0 = filter->b0;
    const ALfloat b1 = filter->b1;
    const ALfloat b2 = filter->b2;
    ALfloat z1 = filter->z1;
    ALfloat z2 = filter->z2;
    ALsizei i;

    ASSUME(numsamples > 0);

    /* Processing loop is Transposed Direct Form II. This requires less storage
     * compared to Direct Form I (only two delay components, instead of a four-
     * sample history; the last two inputs and outputs), and works better for
     * floating-point which favors summing similarly-sized values while being
     * less bothered by overflow.
     *
     * See: http://www.earlevel.com/main/2003/02/28/biquads/
     */
    for(i = 0;i < numsamples;i++)
    {
        ALfloat input = src[i];
        ALfloat output = input*b0 + z1;
        z1 = input*b1 - output*a1 + z2;
        z2 = input*b2 - output*a2;
        dst[i] = output;
    }

    filter->z1 = z1;
    filter->z2 = z2;
}
