/**
 * OpenAL cross platform audio library, SSE3 mixer functions
 *
 * Copyright (C) 2014 by Timothy Arceri <t_arceri@yahoo.com.au>.
 * Copyright (C) 2015 by Chris Robinson <chris.kcat@gmail.com>.
 *
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>

#include "alu.h"
#include "mixer_defs.h"


const ALfloat *Resample_fir4_32_SSE3(const BsincState* UNUSED(state), const ALfloat *restrict src,
                                     ALuint frac, ALuint increment, ALfloat *restrict dst,
                                     ALuint numsamples)
{
    const __m128i increment4 = _mm_set1_epi32(increment*4);
    const __m128i fracMask4 = _mm_set1_epi32(FRACTIONMASK);
    union { alignas(16) ALuint i[4]; float f[4]; } pos_;
    union { alignas(16) ALuint i[4]; float f[4]; } frac_;
    __m128i frac4, pos4;
    ALuint pos;
    ALuint i;

    InitiatePositionArrays(frac, increment, frac_.i, pos_.i, 4);

    frac4 = _mm_castps_si128(_mm_load_ps(frac_.f));
    pos4 = _mm_castps_si128(_mm_load_ps(pos_.f));

    --src;
    for(i = 0;numsamples-i > 3;i += 4)
    {
        const __m128 val0 = _mm_loadu_ps(&src[pos_.i[0]]);
        const __m128 val1 = _mm_loadu_ps(&src[pos_.i[1]]);
        const __m128 val2 = _mm_loadu_ps(&src[pos_.i[2]]);
        const __m128 val3 = _mm_loadu_ps(&src[pos_.i[3]]);
        __m128 k0 = _mm_load_ps(ResampleCoeffs.FIR4[frac_.i[0]]);
        __m128 k1 = _mm_load_ps(ResampleCoeffs.FIR4[frac_.i[1]]);
        __m128 k2 = _mm_load_ps(ResampleCoeffs.FIR4[frac_.i[2]]);
        __m128 k3 = _mm_load_ps(ResampleCoeffs.FIR4[frac_.i[3]]);
        __m128 out;

        k0 = _mm_mul_ps(k0, val0);
        k1 = _mm_mul_ps(k1, val1);
        k2 = _mm_mul_ps(k2, val2);
        k3 = _mm_mul_ps(k3, val3);
        k0 = _mm_hadd_ps(k0, k1);
        k2 = _mm_hadd_ps(k2, k3);
        out = _mm_hadd_ps(k0, k2);

        _mm_store_ps(&dst[i], out);

        frac4 = _mm_add_epi32(frac4, increment4);
        pos4 = _mm_add_epi32(pos4, _mm_srli_epi32(frac4, FRACTIONBITS));
        frac4 = _mm_and_si128(frac4, fracMask4);

        _mm_store_ps(pos_.f, _mm_castsi128_ps(pos4));
        _mm_store_ps(frac_.f, _mm_castsi128_ps(frac4));
    }

    /* NOTE: These four elements represent the position *after* the last four
     * samples, so the lowest element is the next position to resample.
     */
    pos = pos_.i[0];
    frac = frac_.i[0];

    for(;i < numsamples;i++)
    {
        dst[i] = resample_fir4(src[pos], src[pos+1], src[pos+2], src[pos+3], frac);

        frac += increment;
        pos  += frac>>FRACTIONBITS;
        frac &= FRACTIONMASK;
    }
    return dst;
}

const ALfloat *Resample_fir8_32_SSE3(const BsincState* UNUSED(state), const ALfloat *restrict src,
                                     ALuint frac, ALuint increment, ALfloat *restrict dst,
                                     ALuint numsamples)
{
    const __m128i increment4 = _mm_set1_epi32(increment*4);
    const __m128i fracMask4 = _mm_set1_epi32(FRACTIONMASK);
    union { alignas(16) ALuint i[4]; float f[4]; } pos_;
    union { alignas(16) ALuint i[4]; float f[4]; } frac_;
    __m128i frac4, pos4;
    ALuint pos;
    ALuint i, j;

    InitiatePositionArrays(frac, increment, frac_.i, pos_.i, 4);

    frac4 = _mm_castps_si128(_mm_load_ps(frac_.f));
    pos4 = _mm_castps_si128(_mm_load_ps(pos_.f));

    src -= 3;
    for(i = 0;numsamples-i > 3;i += 4)
    {
        __m128 out[2];
        for(j = 0;j < 8;j+=4)
        {
            const __m128 val0 = _mm_loadu_ps(&src[pos_.i[0]+j]);
            const __m128 val1 = _mm_loadu_ps(&src[pos_.i[1]+j]);
            const __m128 val2 = _mm_loadu_ps(&src[pos_.i[2]+j]);
            const __m128 val3 = _mm_loadu_ps(&src[pos_.i[3]+j]);
            __m128 k0 = _mm_load_ps(&ResampleCoeffs.FIR8[frac_.i[0]][j]);
            __m128 k1 = _mm_load_ps(&ResampleCoeffs.FIR8[frac_.i[1]][j]);
            __m128 k2 = _mm_load_ps(&ResampleCoeffs.FIR8[frac_.i[2]][j]);
            __m128 k3 = _mm_load_ps(&ResampleCoeffs.FIR8[frac_.i[3]][j]);

            k0 = _mm_mul_ps(k0, val0);
            k1 = _mm_mul_ps(k1, val1);
            k2 = _mm_mul_ps(k2, val2);
            k3 = _mm_mul_ps(k3, val3);
            k0 = _mm_hadd_ps(k0, k1);
            k2 = _mm_hadd_ps(k2, k3);
            out[j>>2] = _mm_hadd_ps(k0, k2);
        }

        out[0] = _mm_add_ps(out[0], out[1]);
        _mm_store_ps(&dst[i], out[0]);

        frac4 = _mm_add_epi32(frac4, increment4);
        pos4 = _mm_add_epi32(pos4, _mm_srli_epi32(frac4, FRACTIONBITS));
        frac4 = _mm_and_si128(frac4, fracMask4);

        _mm_store_ps(pos_.f, _mm_castsi128_ps(pos4));
        _mm_store_ps(frac_.f, _mm_castsi128_ps(frac4));
    }

    pos = pos_.i[0];
    frac = frac_.i[0];

    for(;i < numsamples;i++)
    {
        dst[i] = resample_fir8(src[pos  ], src[pos+1], src[pos+2], src[pos+3],
                               src[pos+4], src[pos+5], src[pos+6], src[pos+7], frac);

        frac += increment;
        pos  += frac>>FRACTIONBITS;
        frac &= FRACTIONMASK;
    }
    return dst;
}
