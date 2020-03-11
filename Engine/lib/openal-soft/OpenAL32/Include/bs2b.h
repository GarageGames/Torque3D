/*-
 * Copyright (c) 2005 Boris Mikhaylov
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef BS2B_H
#define BS2B_H

/* Number of crossfeed levels */
#define BS2B_CLEVELS           3

/* Normal crossfeed levels */
#define BS2B_HIGH_CLEVEL       3
#define BS2B_MIDDLE_CLEVEL     2
#define BS2B_LOW_CLEVEL        1

/* Easy crossfeed levels */
#define BS2B_HIGH_ECLEVEL      BS2B_HIGH_CLEVEL    + BS2B_CLEVELS
#define BS2B_MIDDLE_ECLEVEL    BS2B_MIDDLE_CLEVEL  + BS2B_CLEVELS
#define BS2B_LOW_ECLEVEL       BS2B_LOW_CLEVEL     + BS2B_CLEVELS

/* Default crossfeed levels */
#define BS2B_DEFAULT_CLEVEL    BS2B_HIGH_ECLEVEL
/* Default sample rate (Hz) */
#define BS2B_DEFAULT_SRATE     44100

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct bs2b {
    int level;  /* Crossfeed level */
    int srate;   /* Sample rate (Hz) */

    /* Lowpass IIR filter coefficients */
    float a0_lo;
    float b1_lo;

    /* Highboost IIR filter coefficients */
    float a0_hi;
    float a1_hi;
    float b1_hi;

    /* Buffer of last filtered sample.
     * [0] - first channel, [1] - second channel
     */
    struct t_last_sample {
        float asis;
        float lo;
        float hi;
    } last_sample[2];
};

/* Clear buffers and set new coefficients with new crossfeed level and sample
 * rate values.
 * level - crossfeed level of *LEVEL values.
 * srate - sample rate by Hz.
 */
void bs2b_set_params(struct bs2b *bs2b, int level, int srate);

/* Return current crossfeed level value */
int bs2b_get_level(struct bs2b *bs2b);

/* Return current sample rate value */
int bs2b_get_srate(struct bs2b *bs2b);

/* Clear buffer */
void bs2b_clear(struct bs2b *bs2b);

void bs2b_cross_feed(struct bs2b *bs2b, float *restrict Left, float *restrict Right, int SamplesToDo);

#ifdef __cplusplus
}    /* extern "C" */
#endif /* __cplusplus */

#endif /* BS2B_H */
