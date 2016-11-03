
#include "config.h"

#include "alu.h"
#include "uhjfilter.h"

/* This is the maximum number of samples processed for each inner loop
 * iteration. */
#define MAX_UPDATE_SAMPLES  128


static const ALfloat Filter1Coeff[4] = {
    0.6923878f, 0.9360654322959f, 0.9882295226860f, 0.9987488452737f
};
static const ALfloat Filter2Coeff[4] = {
    0.4021921162426f, 0.8561710882420f, 0.9722909545651f, 0.9952884791278f
};

static void allpass_process(AllPassState *state, ALfloat *restrict dst, const ALfloat *restrict src, const ALfloat aa, ALuint todo)
{
    ALuint i;

    if(todo > 1)
    {
        dst[0] = aa*(src[0] + state->y[1]) - state->x[1];
        dst[1] = aa*(src[1] + state->y[0]) - state->x[0];
        for(i = 2;i < todo;i++)
            dst[i] = aa*(src[i] + dst[i-2]) - src[i-2];
        state->x[1] = src[i-2];
        state->x[0] = src[i-1];
        state->y[1] = dst[i-2];
        state->y[0] = dst[i-1];
    }
    else if(todo == 1)
    {
        dst[0] = aa*(src[0] + state->y[1]) - state->x[1];
        state->x[1] = state->x[0];
        state->x[0] = src[0];
        state->y[1] = state->y[0];
        state->y[0] = dst[0];
    }
}


/* NOTE: There seems to be a bit of an inconsistency in how this encoding is
 * supposed to work. Some references, such as
 *
 * http://members.tripod.com/martin_leese/Ambisonic/UHJ_file_format.html
 *
 * specify a pre-scaling of sqrt(2) on the W channel input, while other
 * references, such as
 *
 * https://en.wikipedia.org/wiki/Ambisonic_UHJ_format#Encoding.5B1.5D
 * and
 * https://wiki.xiph.org/Ambisonics#UHJ_format
 *
 * do not. The sqrt(2) scaling is in line with B-Format decoder coefficients
 * which include such a scaling for the W channel input, however the original
 * source for this equation is a 1985 paper by Michael Gerzon, which does not
 * apparently include the scaling. Applying the extra scaling creates a louder
 * result with a narrower stereo image compared to not scaling, and I don't
 * know which is the intended result.
 */

void EncodeUhj2(Uhj2Encoder *enc, ALfloat *restrict LeftOut, ALfloat *restrict RightOut, ALfloat (*restrict InSamples)[BUFFERSIZE], ALuint SamplesToDo)
{
    ALfloat D[MAX_UPDATE_SAMPLES], S[MAX_UPDATE_SAMPLES];
    ALfloat temp[2][MAX_UPDATE_SAMPLES];
    ALuint base, i;

    for(base = 0;base < SamplesToDo;)
    {
        ALuint todo = minu(SamplesToDo - base, MAX_UPDATE_SAMPLES);

        /* D = 0.6554516*Y */
        for(i = 0;i < todo;i++)
            temp[0][i] = 0.6554516f*InSamples[2][base+i];
        allpass_process(&enc->Filter1_Y[0], temp[1], temp[0],
                        Filter1Coeff[0]*Filter1Coeff[0], todo);
        allpass_process(&enc->Filter1_Y[1], temp[0], temp[1],
                        Filter1Coeff[1]*Filter1Coeff[1], todo);
        allpass_process(&enc->Filter1_Y[2], temp[1], temp[0],
                        Filter1Coeff[2]*Filter1Coeff[2], todo);
        /* NOTE: Filter1 requires a 1 sample delay for the final output, so
         * take the last processed sample from the previous run as the first
         * output sample.
         */
        D[0] = enc->Filter1_Y[3].y[0];
        allpass_process(&enc->Filter1_Y[3], temp[0], temp[1],
                        Filter1Coeff[3]*Filter1Coeff[3], todo);
        for(i = 1;i < todo;i++)
            D[i] = temp[0][i-1];

        /* D += j(-0.3420201*W + 0.5098604*X) */
        for(i = 0;i < todo;i++)
            temp[0][i] = -0.3420201f*InSamples[0][base+i] +
                          0.5098604f*InSamples[1][base+i];
        allpass_process(&enc->Filter2_WX[0], temp[1], temp[0],
                        Filter2Coeff[0]*Filter2Coeff[0], todo);
        allpass_process(&enc->Filter2_WX[1], temp[0], temp[1],
                        Filter2Coeff[1]*Filter2Coeff[1], todo);
        allpass_process(&enc->Filter2_WX[2], temp[1], temp[0],
                        Filter2Coeff[2]*Filter2Coeff[2], todo);
        allpass_process(&enc->Filter2_WX[3], temp[0], temp[1],
                        Filter2Coeff[3]*Filter2Coeff[3], todo);
        for(i = 0;i < todo;i++)
            D[i] += temp[0][i];

        /* S = 0.9396926*W + 0.1855740*X */
        for(i = 0;i < todo;i++)
            temp[0][i] = 0.9396926f*InSamples[0][base+i] +
                         0.1855740f*InSamples[1][base+i];
        allpass_process(&enc->Filter1_WX[0], temp[1], temp[0],
                        Filter1Coeff[0]*Filter1Coeff[0], todo);
        allpass_process(&enc->Filter1_WX[1], temp[0], temp[1],
                        Filter1Coeff[1]*Filter1Coeff[1], todo);
        allpass_process(&enc->Filter1_WX[2], temp[1], temp[0],
                        Filter1Coeff[2]*Filter1Coeff[2], todo);
        S[0] = enc->Filter1_WX[3].y[0];
        allpass_process(&enc->Filter1_WX[3], temp[0], temp[1],
                        Filter1Coeff[3]*Filter1Coeff[3], todo);
        for(i = 1;i < todo;i++)
            S[i] = temp[0][i-1];

        /* Left = (S + D)/2.0 */
        for(i = 0;i < todo;i++)
            *(LeftOut++) += (S[i] + D[i]) * 0.5f;
        /* Right = (S - D)/2.0 */
        for(i = 0;i < todo;i++)
            *(RightOut++) += (S[i] - D[i]) * 0.5f;

        base += todo;
    }
}
