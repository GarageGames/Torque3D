/*
 * Sinc interpolator coefficient and delta generator for the OpenAL Soft
 * cross platform audio library.
 *
 * Copyright (C) 2015 by Christopher Fitzgerald.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 *
 * Or visit:  http://www.gnu.org/licenses/old-licenses/lgpl-2.0.html
 *
 * --------------------------------------------------------------------------
 *
 * This is a modified version of the bandlimited windowed sinc interpolator
 * algorithm presented here:
 *
 *   Smith, J.O. "Windowed Sinc Interpolation", in
 *   Physical Audio Signal Processing,
 *   https://ccrma.stanford.edu/~jos/pasp/Windowed_Sinc_Interpolation.html,
 *   online book,
 *   accessed October 2012.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI                         (3.14159265358979323846)
#endif

// The number of distinct scale and phase intervals within the filter table.
#define BSINC_SCALE_COUNT (16)
#define BSINC_PHASE_COUNT (16)

#define BSINC_REJECTION (60.0)
#define BSINC_POINTS_MIN (12)

static double MinDouble(double a, double b)
{ return (a <= b) ? a : b; }

static double MaxDouble(double a, double b)
{ return (a >= b) ? a : b; }

/* NOTE: This is the normalized (instead of just sin(x)/x) cardinal sine
 *       function.
 *       2 f_t sinc(2 f_t x)
 *       f_t -- normalized transition frequency (0.5 is nyquist)
 *       x   -- sample index (-N to N)
 */
static double Sinc(const double x)
{
    if(fabs(x) < 1e-15)
        return 1.0;
    return sin(M_PI * x) / (M_PI * x);
}

static double BesselI_0(const double x)
{
    double term, sum, last_sum, x2, y;
    int i;

    term = 1.0;
    sum = 1.0;
    x2 = x / 2.0;
    i = 1;

    do {
        y = x2 / i;
        i++;
        last_sum = sum;
        term *= y * y;
        sum += term;
    } while(sum != last_sum);

    return sum;
}

/* NOTE: k is assumed normalized (-1 to 1)
 *       beta is equivalent to 2 alpha
 */
static double Kaiser(const double b, const double k)
{
    double k2;

    if((k < -1.0) || (k > 1.0))
        return 0.0;

    k2 = MaxDouble(1.0 - (k * k), 0.0);

    return BesselI_0(b * sqrt(k2)) / BesselI_0(b);
}

/* NOTE: Calculates the transition width of the Kaiser window.  Rejection is
 *       in dB.
 */
static double CalcKaiserWidth(const double rejection, const int order)
{
    double w_t = 2.0 * M_PI;

    if(rejection > 21.0)
       return (rejection - 7.95) / (order * 2.285 * w_t);

    return 5.79 / (order * w_t);
}

static double CalcKaiserBeta(const double rejection)
{
    if(rejection > 50.0)
       return 0.1102 * (rejection - 8.7);
    else if(rejection >= 21.0)
       return (0.5842 * pow(rejection - 21.0, 0.4)) +
              (0.07886 * (rejection - 21.0));
    return 0.0;
}

/* Generates the coefficient, delta, and index tables required by the bsinc resampler */
static void BsiGenerateTables()
{
    static double filter[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT + 1][2 * BSINC_POINTS_MIN];
    static double scDeltas[BSINC_SCALE_COUNT - 1][BSINC_PHASE_COUNT][2 * BSINC_POINTS_MIN];
    static double phDeltas[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT + 1][2 * BSINC_POINTS_MIN];
    static double spDeltas[BSINC_SCALE_COUNT - 1][BSINC_PHASE_COUNT][2 * BSINC_POINTS_MIN];
    static int mt[BSINC_SCALE_COUNT];
    static double at[BSINC_SCALE_COUNT];
    double width, beta, scaleBase, scaleRange;
    int si, pi, i;

    memset(filter, 0, sizeof(filter));
    memset(scDeltas, 0, sizeof(scDeltas));
    memset(phDeltas, 0, sizeof(phDeltas));
    memset(spDeltas, 0, sizeof(spDeltas));

    /* Calculate windowing parameters.  The width describes the transition
       band, but it may vary due to the linear interpolation between scales
       of the filter.
    */
    width = CalcKaiserWidth(BSINC_REJECTION, BSINC_POINTS_MIN);
    beta = CalcKaiserBeta(BSINC_REJECTION);
    scaleBase = width / 2.0;
    scaleRange = 1.0 - scaleBase;

    // Determine filter scaling.
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const double scale = scaleBase + (scaleRange * si / (BSINC_SCALE_COUNT - 1));
        const double a = MinDouble(BSINC_POINTS_MIN, BSINC_POINTS_MIN / (2.0 * scale));
        int m = 2 * (int)floor(a);

        // Make sure the number of points is a multiple of 4 (for SSE).
        m += ~(m - 1) & 3;

        mt[si] = m;
        at[si] = a;
    }

    /* Calculate the Kaiser-windowed Sinc filter coefficients for each scale
       and phase.
    */
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);
        const int l = (m / 2) - 1;
        const double a = at[si];
        const double scale = scaleBase + (scaleRange * si / (BSINC_SCALE_COUNT - 1));
        const double cutoff = (0.5 * scale) - (scaleBase * MaxDouble(0.5, scale));

        for(pi = 0; pi <= BSINC_PHASE_COUNT; pi++)
        {
            const double phase = l + ((double)pi / BSINC_PHASE_COUNT);

            for(i = 0; i < m; i++)
            {
                const double x = i - phase;
                filter[si][pi][o + i] = Kaiser(beta, x / a) * 2.0 * cutoff * Sinc(2.0 * cutoff * x);
            }
        }
    }

    /* Linear interpolation between scales is simplified by pre-calculating
       the delta (b - a) in: x = a + f (b - a)

       Given a difference in points between scales, the destination points
       will be 0, thus: x = a + f (-a)
    */
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            for(i = 0; i < m; i++)
                scDeltas[si][pi][o + i] = filter[si + 1][pi][o + i] - filter[si][pi][o + i];
        }
    }

    // Linear interpolation between phases is also simplified.
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            for(i = 0; i < m; i++)
                phDeltas[si][pi][o + i] = filter[si][pi + 1][o + i] - filter[si][pi][o + i];
        }
    }

    /* This last simplification is done to complete the bilinear equation for
       the combination of scale and phase.
    */
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            for(i = 0; i < m; i++)
                spDeltas[si][pi][o + i] = phDeltas[si + 1][pi][o + i] - phDeltas[si][pi][o + i];
        }
    }

    // Calculate the table size.
    i = mt[0];
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
        i += BSINC_PHASE_COUNT * mt[si];
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
        i += 2 * BSINC_PHASE_COUNT * mt[si];
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
        i += BSINC_PHASE_COUNT * mt[si];

    fprintf(stdout, "static const float bsincTab[%d] =\n{\n", i);

    /* Only output enough coefficients for the first (cut) scale as needed to
       perform interpolation without extra branching.
    */
    fprintf(stdout, "    /* %2d,%2d */", mt[0], 0);
    for(i = 0; i < mt[0]; i++)
        fprintf(stdout, " %+14.9ef,", filter[0][0][i]);
    fprintf(stdout, "\n\n");

    for(si = 1; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            fprintf(stdout, "    /* %2d,%2d */", m, pi);
            for(i = 0; i < m; i++)
                fprintf(stdout, " %+14.9ef,", filter[si][pi][o + i]);
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\n");

    // There are N-1 scale deltas for N scales.
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            fprintf(stdout, "    /* %2d,%2d */", m, pi);
            for(i = 0; i < m; i++)
                fprintf(stdout, " %+14.9ef,", scDeltas[si][pi][o + i]);
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\n");

    // Exclude phases for the first (cut) scale.
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            fprintf(stdout, "    /* %2d,%2d */", m, pi);
            for(i = 0; i < m; i++)
                fprintf(stdout, " %+14.9ef,", phDeltas[si][pi][o + i]);
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\n");

    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        const int m = mt[si];
        const int o = BSINC_POINTS_MIN - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            fprintf(stdout, "    /* %2d,%2d */", m, pi);
            for(i = 0; i < m; i++)
                fprintf(stdout, " %+14.9ef,", spDeltas[si][pi][o + i]);
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "};\n\n");

    /* The scaleBase is calculated from the Kaiser window transition width.
       It represents the absolute limit to the filter before it fully cuts
       the signal.  The limit in octaves can be calculated by taking the
       base-2 logarithm of its inverse: log_2(1 / scaleBase)
    */
    fprintf(stdout, "    static const ALfloat scaleBase = %.9ef, scaleRange = %.9ef;\n", scaleBase, 1.0 / scaleRange);
    fprintf(stdout, "    static const ALuint m[BSINC_SCALE_COUNT] = {");

    fprintf(stdout, " %d", mt[0]);
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
        fprintf(stdout, ", %d", mt[si]);

    fprintf(stdout, " };\n");
    fprintf(stdout, "    static const ALuint to[4][BSINC_SCALE_COUNT] =\n    {\n        { 0");

    i = mt[0];
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
    {
        fprintf(stdout, ", %d", i);
        i += BSINC_PHASE_COUNT * mt[si];
    }
    fprintf(stdout, " },\n        {");
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        fprintf(stdout, " %d,", i);
        i += BSINC_PHASE_COUNT * mt[si];
    }
    fprintf(stdout, " 0 },\n        { 0");
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
    {
        fprintf(stdout, ", %d", i);
        i += BSINC_PHASE_COUNT * mt[si];
    }
    fprintf (stdout, " },\n        {");
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
    {
        fprintf(stdout, " %d,", i);
        i += BSINC_PHASE_COUNT * mt[si];
    }
    fprintf(stdout, " 0 }\n    };\n");

    fprintf(stdout, "    static const ALuint tm[2][BSINC_SCALE_COUNT] = \n    {\n        { 0");
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
        fprintf(stdout, ", %d", mt[si]);
    fprintf(stdout, " },\n        {");
    for(si = 0; si < (BSINC_SCALE_COUNT - 1); si++)
        fprintf(stdout, " %d,", mt[si]);
    fprintf(stdout, " 0 }\n    };\n");
}

int main(void)
{
    BsiGenerateTables();
    return 0;
}
