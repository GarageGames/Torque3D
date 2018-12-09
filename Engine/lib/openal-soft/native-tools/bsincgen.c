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

#define _UNICODE
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "../common/win_main_utf8.h"


#ifndef M_PI
#define M_PI                         (3.14159265358979323846)
#endif

#if !(defined(_ISOC99_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L))
#define log2(x)  (log(x) / log(2.0))
#endif

/* Same as in alu.h! */
#define FRACTIONBITS (12)
#define FRACTIONONE  (1<<FRACTIONBITS)

// The number of distinct scale and phase intervals within the filter table.
// Must be the same as in alu.h!
#define BSINC_SCALE_COUNT (16)
#define BSINC_PHASE_COUNT (16)

/* 48 points includes the doubling for downsampling, so the maximum number of
 * base sample points is 24, which is 23rd order.
 */
#define BSINC_POINTS_MAX (48)

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
    if(!(k >= -1.0 && k <= 1.0))
        return 0.0;
    return BesselI_0(b * sqrt(1.0 - k*k)) / BesselI_0(b);
}

/* Calculates the (normalized frequency) transition width of the Kaiser window.
 * Rejection is in dB.
 */
static double CalcKaiserWidth(const double rejection, const int order)
{
    double w_t = 2.0 * M_PI;

    if(rejection > 21.0)
       return (rejection - 7.95) / (order * 2.285 * w_t);
    /* This enforces a minimum rejection of just above 21.18dB */
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
static void BsiGenerateTables(FILE *output, const char *tabname, const double rejection, const int order)
{
    static double   filter[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT + 1][BSINC_POINTS_MAX];
    static double scDeltas[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT    ][BSINC_POINTS_MAX];
    static double phDeltas[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT + 1][BSINC_POINTS_MAX];
    static double spDeltas[BSINC_SCALE_COUNT][BSINC_PHASE_COUNT    ][BSINC_POINTS_MAX];
    static int mt[BSINC_SCALE_COUNT];
    static double at[BSINC_SCALE_COUNT];
    const int num_points_min = order + 1;
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
    width = CalcKaiserWidth(rejection, order);
    beta = CalcKaiserBeta(rejection);
    scaleBase = width / 2.0;
    scaleRange = 1.0 - scaleBase;

    // Determine filter scaling.
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const double scale = scaleBase + (scaleRange * si / (BSINC_SCALE_COUNT - 1));
        const double a = MinDouble(floor(num_points_min / (2.0 * scale)), num_points_min);
        const int m = 2 * (int)a;

        mt[si] = m;
        at[si] = a;
    }

    /* Calculate the Kaiser-windowed Sinc filter coefficients for each scale
       and phase.
    */
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = num_points_min - (m / 2);
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
        const int o = num_points_min - (m / 2);

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
        const int o = num_points_min - (m / 2);

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
        const int o = num_points_min - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            for(i = 0; i < m; i++)
                spDeltas[si][pi][o + i] = phDeltas[si + 1][pi][o + i] - phDeltas[si][pi][o + i];
        }
    }

    // Make sure the number of points is a multiple of 4 (for SIMD).
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
        mt[si] = (mt[si]+3) & ~3;

    fprintf(output,
"/* This %d%s order filter has a rejection of -%.0fdB, yielding a transition width\n"
" * of ~%.3f (normalized frequency). Order increases when downsampling to a\n"
" * limit of one octave, after which the quality of the filter (transition\n"
" * width) suffers to reduce the CPU cost. The bandlimiting will cut all sound\n"
" * after downsampling by ~%.2f octaves.\n"
" */\n"
"const BSincTable %s = {\n",
            order, (((order%100)/10) == 1) ? "th" :
                   ((order%10) == 1) ? "st" :
                   ((order%10) == 2) ? "nd" :
                   ((order%10) == 3) ? "rd" : "th",
            rejection, width, log2(1.0/scaleBase), tabname);

    /* The scaleBase is calculated from the Kaiser window transition width.
       It represents the absolute limit to the filter before it fully cuts
       the signal.  The limit in octaves can be calculated by taking the
       base-2 logarithm of its inverse: log_2(1 / scaleBase)
    */
    fprintf(output, "    /* scaleBase */ %.9ef, /* scaleRange */ %.9ef,\n", scaleBase, 1.0 / scaleRange);

    fprintf(output, "    /* m */ {");
    fprintf(output, " %d", mt[0]);
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
        fprintf(output, ", %d", mt[si]);
    fprintf(output, " },\n");

    fprintf(output, "    /* filterOffset */ {");
    fprintf(output, " %d", 0);
    i = mt[0]*4*BSINC_PHASE_COUNT;
    for(si = 1; si < BSINC_SCALE_COUNT; si++)
    {
        fprintf(output, ", %d", i);
        i += mt[si]*4*BSINC_PHASE_COUNT;
    }

    fprintf(output, " },\n");

    // Calculate the table size.
    i = 0;
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
        i += 4 * BSINC_PHASE_COUNT * mt[si];

    fprintf(output, "\n    /* Tab (%d entries) */ {\n", i);
    for(si = 0; si < BSINC_SCALE_COUNT; si++)
    {
        const int m = mt[si];
        const int o = num_points_min - (m / 2);

        for(pi = 0; pi < BSINC_PHASE_COUNT; pi++)
        {
            fprintf(output, "        /* %2d,%2d (%d) */", si, pi, m);
            fprintf(output, "\n       ");
            for(i = 0; i < m; i++)
                fprintf(output, " %+14.9ef,", filter[si][pi][o + i]);
            fprintf(output, "\n       ");
            for(i = 0; i < m; i++)
                fprintf(output, " %+14.9ef,", scDeltas[si][pi][o + i]);
            fprintf(output, "\n       ");
            for(i = 0; i < m; i++)
                fprintf(output, " %+14.9ef,", phDeltas[si][pi][o + i]);
            fprintf(output, "\n       ");
            for(i = 0; i < m; i++)
                fprintf(output, " %+14.9ef,", spDeltas[si][pi][o + i]);
            fprintf(output, "\n");
        }
    }
    fprintf(output, "    }\n};\n\n");
}


int main(int argc, char *argv[])
{
    FILE *output;

    GET_UNICODE_ARGS(&argc, &argv);

    if(argc > 2)
    {
        fprintf(stderr, "Usage: %s [output file]\n", argv[0]);
        return 1;
    }

    if(argc == 2)
    {
        output = fopen(argv[1], "wb");
        if(!output)
        {
            fprintf(stderr, "Failed to open %s for writing\n", argv[1]);
            return 1;
        }
    }
    else
        output = stdout;

    fprintf(output, "/* Generated by bsincgen, do not edit! */\n\n"
"static_assert(BSINC_SCALE_COUNT == %d, \"Unexpected BSINC_SCALE_COUNT value!\");\n"
"static_assert(BSINC_PHASE_COUNT == %d, \"Unexpected BSINC_PHASE_COUNT value!\");\n"
"static_assert(FRACTIONONE == %d, \"Unexpected FRACTIONONE value!\");\n\n"
"typedef struct BSincTable {\n"
"    const float scaleBase, scaleRange;\n"
"    const int m[BSINC_SCALE_COUNT];\n"
"    const int filterOffset[BSINC_SCALE_COUNT];\n"
"    alignas(16) const float Tab[];\n"
"} BSincTable;\n\n", BSINC_SCALE_COUNT, BSINC_PHASE_COUNT, FRACTIONONE);
    /* A 23rd order filter with a -60dB drop at nyquist. */
    BsiGenerateTables(output, "bsinc24", 60.0, 23);
    /* An 11th order filter with a -40dB drop at nyquist. */
    BsiGenerateTables(output, "bsinc12", 40.0, 11);

    if(output != stdout)
        fclose(output);
    output = NULL;

    return 0;
}
