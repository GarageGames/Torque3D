
#include "config.h"

#include "alu.h"
#include "uhjfilter.h"

/* This is the maximum number of samples processed for each inner loop
 * iteration. */
#define MAX_UPDATE_SAMPLES  128


static const ALfloat Filter1CoeffSqr[4] = {
    0.479400865589f, 0.876218493539f, 0.976597589508f, 0.997499255936f
};
static const ALfloat Filter2CoeffSqr[4] = {
    0.161758498368f, 0.733028932341f, 0.945349700329f, 0.990599156685f
};

static void allpass_process(AllPassState *state, ALfloat *restrict dst, const ALfloat *restrict src, const ALfloat aa, ALsizei todo)
{
    ALfloat z1 = state->z[0];
    ALfloat z2 = state->z[1];
    ALsizei i;

    for(i = 0;i < todo;i++)
    {
        ALfloat input = src[i];
        ALfloat output = input*aa + z1;
        z1 = z2; z2 = output*aa - input;
        dst[i] = output;
    }

    state->z[0] = z1;
    state->z[1] = z2;
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

void EncodeUhj2(Uhj2Encoder *enc, ALfloat *restrict LeftOut, ALfloat *restrict RightOut, ALfloat (*restrict InSamples)[BUFFERSIZE], ALsizei SamplesToDo)
{
    ALfloat D[MAX_UPDATE_SAMPLES], S[MAX_UPDATE_SAMPLES];
    ALfloat temp[2][MAX_UPDATE_SAMPLES];
    ALsizei base, i;

    ASSUME(SamplesToDo > 0);

    for(base = 0;base < SamplesToDo;)
    {
        ALsizei todo = mini(SamplesToDo - base, MAX_UPDATE_SAMPLES);
        ASSUME(todo > 0);

        /* D = 0.6554516*Y */
        for(i = 0;i < todo;i++)
            temp[0][i] = 0.6554516f*InSamples[2][base+i];
        allpass_process(&enc->Filter1_Y[0], temp[1], temp[0], Filter1CoeffSqr[0], todo);
        allpass_process(&enc->Filter1_Y[1], temp[0], temp[1], Filter1CoeffSqr[1], todo);
        allpass_process(&enc->Filter1_Y[2], temp[1], temp[0], Filter1CoeffSqr[2], todo);
        allpass_process(&enc->Filter1_Y[3], temp[0], temp[1], Filter1CoeffSqr[3], todo);
        /* NOTE: Filter1 requires a 1 sample delay for the final output, so
         * take the last processed sample from the previous run as the first
         * output sample.
         */
        D[0] = enc->LastY;
        for(i = 1;i < todo;i++)
            D[i] = temp[0][i-1];
        enc->LastY = temp[0][i-1];

        /* D += j(-0.3420201*W + 0.5098604*X) */
        for(i = 0;i < todo;i++)
            temp[0][i] = -0.3420201f*InSamples[0][base+i] +
                          0.5098604f*InSamples[1][base+i];
        allpass_process(&enc->Filter2_WX[0], temp[1], temp[0], Filter2CoeffSqr[0], todo);
        allpass_process(&enc->Filter2_WX[1], temp[0], temp[1], Filter2CoeffSqr[1], todo);
        allpass_process(&enc->Filter2_WX[2], temp[1], temp[0], Filter2CoeffSqr[2], todo);
        allpass_process(&enc->Filter2_WX[3], temp[0], temp[1], Filter2CoeffSqr[3], todo);
        for(i = 0;i < todo;i++)
            D[i] += temp[0][i];

        /* S = 0.9396926*W + 0.1855740*X */
        for(i = 0;i < todo;i++)
            temp[0][i] = 0.9396926f*InSamples[0][base+i] +
                         0.1855740f*InSamples[1][base+i];
        allpass_process(&enc->Filter1_WX[0], temp[1], temp[0], Filter1CoeffSqr[0], todo);
        allpass_process(&enc->Filter1_WX[1], temp[0], temp[1], Filter1CoeffSqr[1], todo);
        allpass_process(&enc->Filter1_WX[2], temp[1], temp[0], Filter1CoeffSqr[2], todo);
        allpass_process(&enc->Filter1_WX[3], temp[0], temp[1], Filter1CoeffSqr[3], todo);
        S[0] = enc->LastWX;
        for(i = 1;i < todo;i++)
            S[i] = temp[0][i-1];
        enc->LastWX = temp[0][i-1];

        /* Left = (S + D)/2.0 */
        for(i = 0;i < todo;i++)
            *(LeftOut++) += (S[i] + D[i]) * 0.5f;
        /* Right = (S - D)/2.0 */
        for(i = 0;i < todo;i++)
            *(RightOut++) += (S[i] - D[i]) * 0.5f;

        base += todo;
    }
}
