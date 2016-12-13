/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by Chris Robinson
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

#include <stdlib.h>
#include <ctype.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alSource.h"
#include "alu.h"
#include "bformatdec.h"
#include "hrtf.h"

#include "compat.h"
#include "almalloc.h"


/* Current data set limits defined by the makehrtf utility. */
#define MIN_IR_SIZE                  (8)
#define MAX_IR_SIZE                  (128)
#define MOD_IR_SIZE                  (8)

#define MIN_EV_COUNT                 (5)
#define MAX_EV_COUNT                 (128)

#define MIN_AZ_COUNT                 (1)
#define MAX_AZ_COUNT                 (128)

static const ALchar magicMarker00[8] = "MinPHR00";
static const ALchar magicMarker01[8] = "MinPHR01";

/* First value for pass-through coefficients (remaining are 0), used for omni-
 * directional sounds. */
static const ALfloat PassthruCoeff = 32767.0f * 0.707106781187f/*sqrt(0.5)*/;

static struct Hrtf *LoadedHrtfs = NULL;


/* Calculate the elevation index given the polar elevation in radians. This
 * will return an index between 0 and (evcount - 1). Assumes the FPU is in
 * round-to-zero mode.
 */
static ALuint CalcEvIndex(ALuint evcount, ALfloat ev)
{
    ev = (F_PI_2 + ev) * (evcount-1) / F_PI;
    return minu(fastf2u(ev + 0.5f), evcount-1);
}

/* Calculate the azimuth index given the polar azimuth in radians. This will
 * return an index between 0 and (azcount - 1). Assumes the FPU is in round-to-
 * zero mode.
 */
static ALuint CalcAzIndex(ALuint azcount, ALfloat az)
{
    az = (F_TAU + az) * azcount / F_TAU;
    return fastf2u(az + 0.5f) % azcount;
}

/* Calculates static HRIR coefficients and delays for the given polar elevation
 * and azimuth in radians. The coefficients are normalized and attenuated by
 * the specified gain.
 */
void GetHrtfCoeffs(const struct Hrtf *Hrtf, ALfloat elevation, ALfloat azimuth, ALfloat spread, ALfloat gain, ALfloat (*coeffs)[2], ALuint *delays)
{
    ALuint evidx, azidx, lidx, ridx;
    ALuint azcount, evoffset;
    ALfloat dirfact;
    ALuint i;

    dirfact = 1.0f - (spread / F_TAU);

    /* Claculate elevation index. */
    evidx = CalcEvIndex(Hrtf->evCount, elevation);
    azcount = Hrtf->azCount[evidx];
    evoffset = Hrtf->evOffset[evidx];

    /* Calculate azimuth index. */
    azidx = CalcAzIndex(Hrtf->azCount[evidx], azimuth);

    /* Calculate the HRIR indices for left and right channels. */
    lidx = evoffset + azidx;
    ridx = evoffset + ((azcount-azidx) % azcount);

    /* Calculate the HRIR delays. */
    delays[0] = fastf2u(Hrtf->delays[lidx]*dirfact + 0.5f) << HRTFDELAY_BITS;
    delays[1] = fastf2u(Hrtf->delays[ridx]*dirfact + 0.5f) << HRTFDELAY_BITS;

    /* Calculate the sample offsets for the HRIR indices. */
    lidx *= Hrtf->irSize;
    ridx *= Hrtf->irSize;

    /* Calculate the normalized and attenuated HRIR coefficients. Zero the
     * coefficients if gain is too low.
     */
    if(gain > 0.0001f)
    {
        gain /= 32767.0f;

        i = 0;
        coeffs[i][0] = lerp(PassthruCoeff, Hrtf->coeffs[lidx+i], dirfact)*gain;
        coeffs[i][1] = lerp(PassthruCoeff, Hrtf->coeffs[ridx+i], dirfact)*gain;
        for(i = 1;i < Hrtf->irSize;i++)
        {
            coeffs[i][0] = Hrtf->coeffs[lidx+i]*gain * dirfact;
            coeffs[i][1] = Hrtf->coeffs[ridx+i]*gain * dirfact;
        }
    }
    else
    {
        for(i = 0;i < Hrtf->irSize;i++)
        {
            coeffs[i][0] = 0.0f;
            coeffs[i][1] = 0.0f;
        }
    }
}


ALuint BuildBFormatHrtf(const struct Hrtf *Hrtf, ALfloat (*coeffs)[HRIR_LENGTH][2], ALuint NumChannels)
{
    static const struct {
        ALfloat elevation;
        ALfloat azimuth;
    } Ambi3DPoints[14] = {
        { DEG2RAD( 90.0f), DEG2RAD(   0.0f) },
        { DEG2RAD( 35.0f), DEG2RAD( -45.0f) },
        { DEG2RAD( 35.0f), DEG2RAD(  45.0f) },
        { DEG2RAD( 35.0f), DEG2RAD( 135.0f) },
        { DEG2RAD( 35.0f), DEG2RAD(-135.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(   0.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(  90.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( 180.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( -90.0f) },
        { DEG2RAD(-35.0f), DEG2RAD( -45.0f) },
        { DEG2RAD(-35.0f), DEG2RAD(  45.0f) },
        { DEG2RAD(-35.0f), DEG2RAD( 135.0f) },
        { DEG2RAD(-35.0f), DEG2RAD(-135.0f) },
        { DEG2RAD(-90.0f), DEG2RAD(   0.0f) },
    };
    static const ALfloat Ambi3DMatrix[14][2][MAX_AMBI_COEFFS] = {
        { { 0.078851598f,  0.000000000f,  0.070561967f,  0.000000000f }, { 0.0269973975f,  0.0000000000f,  0.0467610443f,  0.0000000000f } },
        { { 0.124051278f,  0.059847972f,  0.059847972f,  0.059847972f }, { 0.0269973975f,  0.0269973975f,  0.0269973975f,  0.0269973975f } },
        { { 0.124051278f, -0.059847972f,  0.059847972f,  0.059847972f }, { 0.0269973975f, -0.0269973975f,  0.0269973975f,  0.0269973975f } },
        { { 0.124051278f, -0.059847972f,  0.059847972f, -0.059847972f }, { 0.0269973975f, -0.0269973975f,  0.0269973975f, -0.0269973975f } },
        { { 0.124051278f,  0.059847972f,  0.059847972f, -0.059847972f }, { 0.0269973975f,  0.0269973975f,  0.0269973975f, -0.0269973975f } },
        { { 0.078851598f,  0.000000000f,  0.000000000f,  0.070561967f }, { 0.0269973975f,  0.0000000000f,  0.0000000000f,  0.0467610443f } },
        { { 0.078851598f, -0.070561967f,  0.000000000f,  0.000000000f }, { 0.0269973975f, -0.0467610443f,  0.0000000000f,  0.0000000000f } },
        { { 0.078851598f,  0.000000000f,  0.000000000f, -0.070561967f }, { 0.0269973975f,  0.0000000000f,  0.0000000000f, -0.0467610443f } },
        { { 0.078851598f,  0.070561967f,  0.000000000f,  0.000000000f }, { 0.0269973975f,  0.0467610443f,  0.0000000000f,  0.0000000000f } },
        { { 0.124051278f,  0.059847972f, -0.059847972f,  0.059847972f }, { 0.0269973975f,  0.0269973975f, -0.0269973975f,  0.0269973975f } },
        { { 0.124051278f, -0.059847972f, -0.059847972f,  0.059847972f }, { 0.0269973975f, -0.0269973975f, -0.0269973975f,  0.0269973975f } },
        { { 0.124051278f, -0.059847972f, -0.059847972f, -0.059847972f }, { 0.0269973975f, -0.0269973975f, -0.0269973975f, -0.0269973975f } },
        { { 0.124051278f,  0.059847972f, -0.059847972f, -0.059847972f }, { 0.0269973975f,  0.0269973975f, -0.0269973975f, -0.0269973975f } },
        { { 0.078851598f,  0.000000000f, -0.070561967f,  0.000000000f }, { 0.0269973975f,  0.0000000000f, -0.0467610443f,  0.0000000000f } },
    };
/* Change this to 2 for dual-band HRTF processing. May require a higher quality
 * band-splitter, or better calculation of the new IR length to deal with the
 * tail generated by the filter.
 */
#define NUM_BANDS 2
    BandSplitter splitter;
    ALfloat temps[3][HRIR_LENGTH];
    ALuint lidx[14], ridx[14];
    ALuint min_delay = HRTF_HISTORY_LENGTH;
    ALuint max_length = 0;
    ALuint i, j, c, b;

    assert(NumChannels == 4);

    for(c = 0;c < COUNTOF(Ambi3DPoints);c++)
    {
        ALuint evidx, azidx;
        ALuint evoffset;
        ALuint azcount;

        /* Calculate elevation index. */
        evidx = (ALuint)floorf((F_PI_2 + Ambi3DPoints[c].elevation) *
                               (Hrtf->evCount-1)/F_PI + 0.5f);
        evidx = minu(evidx, Hrtf->evCount-1);

        azcount = Hrtf->azCount[evidx];
        evoffset = Hrtf->evOffset[evidx];

        /* Calculate azimuth index for this elevation. */
        azidx = (ALuint)floorf((F_TAU+Ambi3DPoints[c].azimuth) *
                               azcount/F_TAU + 0.5f) % azcount;

        /* Calculate indices for left and right channels. */
        lidx[c] = evoffset + azidx;
        ridx[c] = evoffset + ((azcount-azidx) % azcount);

        min_delay = minu(min_delay, minu(Hrtf->delays[lidx[c]], Hrtf->delays[ridx[c]]));
    }

    memset(temps, 0, sizeof(temps));
    bandsplit_init(&splitter, 400.0f / (ALfloat)Hrtf->sampleRate);
    for(c = 0;c < COUNTOF(Ambi3DMatrix);c++)
    {
        const ALshort *fir;
        ALuint delay;

        /* Convert the left FIR from shorts to float */
        fir = &Hrtf->coeffs[lidx[c] * Hrtf->irSize];
        if(NUM_BANDS == 1)
        {
            for(i = 0;i < Hrtf->irSize;i++)
                temps[0][i] = fir[i] / 32767.0f;
        }
        else
        {
            /* Band-split left HRIR into low and high frequency responses. */
            bandsplit_clear(&splitter);
            for(i = 0;i < Hrtf->irSize;i++)
                temps[2][i] = fir[i] / 32767.0f;
            bandsplit_process(&splitter, temps[0], temps[1], temps[2], HRIR_LENGTH);
        }

        /* Add to the left output coefficients with the specified delay. */
        delay = Hrtf->delays[lidx[c]] - min_delay;
        for(i = 0;i < NumChannels;++i)
        {
            for(b = 0;b < NUM_BANDS;b++)
            {
                ALuint k = 0;
                for(j = delay;j < HRIR_LENGTH;++j)
                    coeffs[i][j][0] += temps[b][k++] * Ambi3DMatrix[c][b][i];
            }
        }
        max_length = maxu(max_length, minu(delay + Hrtf->irSize, HRIR_LENGTH));

        /* Convert the right FIR from shorts to float */
        fir = &Hrtf->coeffs[ridx[c] * Hrtf->irSize];
        if(NUM_BANDS == 1)
        {
            for(i = 0;i < Hrtf->irSize;i++)
                temps[0][i] = fir[i] / 32767.0f;
        }
        else
        {
            /* Band-split right HRIR into low and high frequency responses. */
            bandsplit_clear(&splitter);
            for(i = 0;i < Hrtf->irSize;i++)
                temps[2][i] = fir[i] / 32767.0f;
            bandsplit_process(&splitter, temps[0], temps[1], temps[2], HRIR_LENGTH);
        }

        /* Add to the right output coefficients with the specified delay. */
        delay = Hrtf->delays[ridx[c]] - min_delay;
        for(i = 0;i < NumChannels;++i)
        {
            for(b = 0;b < NUM_BANDS;b++)
            {
                ALuint k = 0;
                for(j = delay;j < HRIR_LENGTH;++j)
                    coeffs[i][j][1] += temps[b][k++] * Ambi3DMatrix[c][b][i];
            }
        }
        max_length = maxu(max_length, minu(delay + Hrtf->irSize, HRIR_LENGTH));
    }
    TRACE("Skipped min delay: %u, new combined length: %u\n", min_delay, max_length);
#undef NUM_BANDS

    return max_length;
}


static struct Hrtf *LoadHrtf00(const ALubyte *data, size_t datalen, const_al_string filename)
{
    const ALubyte maxDelay = HRTF_HISTORY_LENGTH-1;
    struct Hrtf *Hrtf = NULL;
    ALboolean failed = AL_FALSE;
    ALuint rate = 0, irCount = 0;
    ALushort irSize = 0;
    ALubyte evCount = 0;
    ALubyte *azCount = NULL;
    ALushort *evOffset = NULL;
    ALshort *coeffs = NULL;
    const ALubyte *delays = NULL;
    ALuint i, j;

    if(datalen < 9)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT")\n",
            al_string_get_cstr(filename), 9, datalen);
        return NULL;
    }

    rate  = *(data++);
    rate |= *(data++)<<8;
    rate |= *(data++)<<16;
    rate |= *(data++)<<24;
    datalen -= 4;

    irCount  = *(data++);
    irCount |= *(data++)<<8;
    datalen -= 2;

    irSize  = *(data++);
    irSize |= *(data++)<<8;
    datalen -= 2;

    evCount = *(data++);
    datalen -= 1;

    if(irSize < MIN_IR_SIZE || irSize > MAX_IR_SIZE || (irSize%MOD_IR_SIZE))
    {
        ERR("Unsupported HRIR size: irSize=%d (%d to %d by %d)\n",
            irSize, MIN_IR_SIZE, MAX_IR_SIZE, MOD_IR_SIZE);
        failed = AL_TRUE;
    }
    if(evCount < MIN_EV_COUNT || evCount > MAX_EV_COUNT)
    {
        ERR("Unsupported elevation count: evCount=%d (%d to %d)\n",
            evCount, MIN_EV_COUNT, MAX_EV_COUNT);
        failed = AL_TRUE;
    }
    if(failed)
        return NULL;

    if(datalen < evCount*2)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT")\n",
            al_string_get_cstr(filename), evCount*2, datalen);
        return NULL;
    }

    azCount = malloc(sizeof(azCount[0])*evCount);
    evOffset = malloc(sizeof(evOffset[0])*evCount);
    if(azCount == NULL || evOffset == NULL)
    {
        ERR("Out of memory.\n");
        failed = AL_TRUE;
    }

    if(!failed)
    {
        evOffset[0]  = *(data++);
        evOffset[0] |= *(data++)<<8;
        datalen -= 2;
        for(i = 1;i < evCount;i++)
        {
            evOffset[i]  = *(data++);
            evOffset[i] |= *(data++)<<8;
            datalen -= 2;
            if(evOffset[i] <= evOffset[i-1])
            {
                ERR("Invalid evOffset: evOffset[%d]=%d (last=%d)\n",
                    i, evOffset[i], evOffset[i-1]);
                failed = AL_TRUE;
            }

            azCount[i-1] = evOffset[i] - evOffset[i-1];
            if(azCount[i-1] < MIN_AZ_COUNT || azCount[i-1] > MAX_AZ_COUNT)
            {
                ERR("Unsupported azimuth count: azCount[%d]=%d (%d to %d)\n",
                    i-1, azCount[i-1], MIN_AZ_COUNT, MAX_AZ_COUNT);
                failed = AL_TRUE;
            }
        }
        if(irCount <= evOffset[i-1])
        {
            ERR("Invalid evOffset: evOffset[%d]=%d (irCount=%d)\n",
                i-1, evOffset[i-1], irCount);
            failed = AL_TRUE;
        }

        azCount[i-1] = irCount - evOffset[i-1];
        if(azCount[i-1] < MIN_AZ_COUNT || azCount[i-1] > MAX_AZ_COUNT)
        {
            ERR("Unsupported azimuth count: azCount[%d]=%d (%d to %d)\n",
                i-1, azCount[i-1], MIN_AZ_COUNT, MAX_AZ_COUNT);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        coeffs = malloc(sizeof(coeffs[0])*irSize*irCount);
        if(coeffs == NULL)
        {
            ERR("Out of memory.\n");
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        size_t reqsize = 2*irSize*irCount + irCount;
        if(datalen < reqsize)
        {
            ERR("Unexpected end of %s data (req "SZFMT", rem "SZFMT")\n",
                al_string_get_cstr(filename), reqsize, datalen);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        for(i = 0;i < irCount*irSize;i+=irSize)
        {
            for(j = 0;j < irSize;j++)
            {
                coeffs[i+j]  = *(data++);
                coeffs[i+j] |= *(data++)<<8;
                datalen -= 2;
            }
        }

        delays = data;
        data += irCount;
        datalen -= irCount;
        for(i = 0;i < irCount;i++)
        {
            if(delays[i] > maxDelay)
            {
                ERR("Invalid delays[%d]: %d (%d)\n", i, delays[i], maxDelay);
                failed = AL_TRUE;
            }
        }
    }

    if(!failed)
    {
        size_t total = sizeof(struct Hrtf);
        total += sizeof(azCount[0])*evCount;
        total  = (total+1)&~1; /* Align for (u)short fields */
        total += sizeof(evOffset[0])*evCount;
        total += sizeof(coeffs[0])*irSize*irCount;
        total += sizeof(delays[0])*irCount;
        total += al_string_length(filename)+1;

        Hrtf = al_calloc(16, total);
        if(Hrtf == NULL)
        {
            ERR("Out of memory.\n");
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        char *base = (char*)Hrtf;
        uintptr_t offset = sizeof(*Hrtf);

        Hrtf->sampleRate = rate;
        Hrtf->irSize = irSize;
        Hrtf->evCount = evCount;
        Hrtf->azCount = ((ALubyte*)(base + offset)); offset += evCount*sizeof(Hrtf->azCount[0]);
        offset = (offset+1)&~1; /* Align for (u)short fields */
        Hrtf->evOffset = ((ALushort*)(base + offset)); offset += evCount*sizeof(Hrtf->evOffset[0]);
        Hrtf->coeffs = ((ALshort*)(base + offset)); offset += irSize*irCount*sizeof(Hrtf->coeffs[0]);
        Hrtf->delays = ((ALubyte*)(base + offset)); offset += irCount*sizeof(Hrtf->delays[0]);
        Hrtf->filename = ((char*)(base + offset));
        Hrtf->next = NULL;

        memcpy((void*)Hrtf->azCount, azCount, sizeof(azCount[0])*evCount);
        memcpy((void*)Hrtf->evOffset, evOffset, sizeof(evOffset[0])*evCount);
        memcpy((void*)Hrtf->coeffs, coeffs, sizeof(coeffs[0])*irSize*irCount);
        memcpy((void*)Hrtf->delays, delays, sizeof(delays[0])*irCount);
        memcpy((void*)Hrtf->filename, al_string_get_cstr(filename), al_string_length(filename)+1);
    }

    free(azCount);
    free(evOffset);
    free(coeffs);
    return Hrtf;
}

static struct Hrtf *LoadHrtf01(const ALubyte *data, size_t datalen, const_al_string filename)
{
    const ALubyte maxDelay = HRTF_HISTORY_LENGTH-1;
    struct Hrtf *Hrtf = NULL;
    ALboolean failed = AL_FALSE;
    ALuint rate = 0, irCount = 0;
    ALubyte irSize = 0, evCount = 0;
    const ALubyte *azCount = NULL;
    ALushort *evOffset = NULL;
    ALshort *coeffs = NULL;
    const ALubyte *delays = NULL;
    ALuint i, j;

    if(datalen < 6)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n",
            al_string_get_cstr(filename), 6, datalen);
        return NULL;
    }

    rate  = *(data++);
    rate |= *(data++)<<8;
    rate |= *(data++)<<16;
    rate |= *(data++)<<24;
    datalen -= 4;

    irSize = *(data++);
    datalen -= 1;

    evCount = *(data++);
    datalen -= 1;

    if(irSize < MIN_IR_SIZE || irSize > MAX_IR_SIZE || (irSize%MOD_IR_SIZE))
    {
        ERR("Unsupported HRIR size: irSize=%d (%d to %d by %d)\n",
            irSize, MIN_IR_SIZE, MAX_IR_SIZE, MOD_IR_SIZE);
        failed = AL_TRUE;
    }
    if(evCount < MIN_EV_COUNT || evCount > MAX_EV_COUNT)
    {
        ERR("Unsupported elevation count: evCount=%d (%d to %d)\n",
            evCount, MIN_EV_COUNT, MAX_EV_COUNT);
        failed = AL_TRUE;
    }
    if(failed)
        return NULL;

    if(datalen < evCount)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n",
            al_string_get_cstr(filename), evCount, datalen);
        return NULL;
    }

    azCount = data;
    data += evCount;
    datalen -= evCount;

    evOffset = malloc(sizeof(evOffset[0])*evCount);
    if(azCount == NULL || evOffset == NULL)
    {
        ERR("Out of memory.\n");
        failed = AL_TRUE;
    }

    if(!failed)
    {
        for(i = 0;i < evCount;i++)
        {
            if(azCount[i] < MIN_AZ_COUNT || azCount[i] > MAX_AZ_COUNT)
            {
                ERR("Unsupported azimuth count: azCount[%d]=%d (%d to %d)\n",
                    i, azCount[i], MIN_AZ_COUNT, MAX_AZ_COUNT);
                failed = AL_TRUE;
            }
        }
    }

    if(!failed)
    {
        evOffset[0] = 0;
        irCount = azCount[0];
        for(i = 1;i < evCount;i++)
        {
            evOffset[i] = evOffset[i-1] + azCount[i-1];
            irCount += azCount[i];
        }

        coeffs = malloc(sizeof(coeffs[0])*irSize*irCount);
        if(coeffs == NULL)
        {
            ERR("Out of memory.\n");
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        size_t reqsize = 2*irSize*irCount + irCount;
        if(datalen < reqsize)
        {
            ERR("Unexpected end of %s data (req "SZFMT", rem "SZFMT"\n",
                al_string_get_cstr(filename), reqsize, datalen);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        for(i = 0;i < irCount*irSize;i+=irSize)
        {
            for(j = 0;j < irSize;j++)
            {
                ALshort coeff;
                coeff  = *(data++);
                coeff |= *(data++)<<8;
                datalen -= 2;
                coeffs[i+j] = coeff;
            }
        }

        delays = data;
        data += irCount;
        datalen -= irCount;
        for(i = 0;i < irCount;i++)
        {
            if(delays[i] > maxDelay)
            {
                ERR("Invalid delays[%d]: %d (%d)\n", i, delays[i], maxDelay);
                failed = AL_TRUE;
            }
        }
    }

    if(!failed)
    {
        size_t total = sizeof(struct Hrtf);
        total += sizeof(azCount[0])*evCount;
        total  = (total+1)&~1; /* Align for (u)short fields */
        total += sizeof(evOffset[0])*evCount;
        total += sizeof(coeffs[0])*irSize*irCount;
        total += sizeof(delays[0])*irCount;
        total += al_string_length(filename)+1;

        Hrtf = al_calloc(16, total);
        if(Hrtf == NULL)
        {
            ERR("Out of memory.\n");
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        char *base = (char*)Hrtf;
        uintptr_t offset = sizeof(*Hrtf);

        Hrtf->sampleRate = rate;
        Hrtf->irSize = irSize;
        Hrtf->evCount = evCount;
        Hrtf->azCount = ((ALubyte*)(base + offset)); offset += evCount*sizeof(Hrtf->azCount[0]);
        offset = (offset+1)&~1; /* Align for (u)short fields */
        Hrtf->evOffset = ((ALushort*)(base + offset)); offset += evCount*sizeof(Hrtf->evOffset[0]);
        Hrtf->coeffs = ((ALshort*)(base + offset)); offset += irSize*irCount*sizeof(Hrtf->coeffs[0]);
        Hrtf->delays = ((ALubyte*)(base + offset)); offset += irCount*sizeof(Hrtf->delays[0]);
        Hrtf->filename = ((char*)(base + offset));
        Hrtf->next = NULL;

        memcpy((void*)Hrtf->azCount, azCount, sizeof(azCount[0])*evCount);
        memcpy((void*)Hrtf->evOffset, evOffset, sizeof(evOffset[0])*evCount);
        memcpy((void*)Hrtf->coeffs, coeffs, sizeof(coeffs[0])*irSize*irCount);
        memcpy((void*)Hrtf->delays, delays, sizeof(delays[0])*irCount);
        memcpy((void*)Hrtf->filename, al_string_get_cstr(filename), al_string_length(filename)+1);
    }

    free(evOffset);
    free(coeffs);
    return Hrtf;
}

static void AddFileEntry(vector_HrtfEntry *list, al_string *filename)
{
    HrtfEntry entry = { AL_STRING_INIT_STATIC(), NULL };
    struct Hrtf *hrtf = NULL;
    const HrtfEntry *iter;
    struct FileMapping fmap;
    const char *name;
    const char *ext;
    int i;

#define MATCH_FNAME(i) (al_string_cmp_cstr(*filename, (i)->hrtf->filename) == 0)
    VECTOR_FIND_IF(iter, const HrtfEntry, *list, MATCH_FNAME);
    if(iter != VECTOR_END(*list))
    {
        TRACE("Skipping duplicate file entry %s\n", al_string_get_cstr(*filename));
        goto done;
    }
#undef MATCH_FNAME

    entry.hrtf = LoadedHrtfs;
    while(entry.hrtf)
    {
        if(al_string_cmp_cstr(*filename, entry.hrtf->filename) == 0)
        {
            TRACE("Skipping load of already-loaded file %s\n", al_string_get_cstr(*filename));
            goto skip_load;
        }
        entry.hrtf = entry.hrtf->next;
    }

    TRACE("Loading %s...\n", al_string_get_cstr(*filename));
    fmap = MapFileToMem(al_string_get_cstr(*filename));
    if(fmap.ptr == NULL)
    {
        ERR("Could not open %s\n", al_string_get_cstr(*filename));
        goto done;
    }

    if(fmap.len < sizeof(magicMarker01))
        ERR("%s data is too short ("SZFMT" bytes)\n", al_string_get_cstr(*filename), fmap.len);
    else if(memcmp(fmap.ptr, magicMarker01, sizeof(magicMarker01)) == 0)
    {
        TRACE("Detected data set format v1\n");
        hrtf = LoadHrtf01((const ALubyte*)fmap.ptr+sizeof(magicMarker01),
            fmap.len-sizeof(magicMarker01), *filename
        );
    }
    else if(memcmp(fmap.ptr, magicMarker00, sizeof(magicMarker00)) == 0)
    {
        TRACE("Detected data set format v0\n");
        hrtf = LoadHrtf00((const ALubyte*)fmap.ptr+sizeof(magicMarker00),
            fmap.len-sizeof(magicMarker00), *filename
        );
    }
    else
        ERR("Invalid header in %s: \"%.8s\"\n", al_string_get_cstr(*filename), (const char*)fmap.ptr);
    UnmapFileMem(&fmap);

    if(!hrtf)
    {
        ERR("Failed to load %s\n", al_string_get_cstr(*filename));
        goto done;
    }

    hrtf->next = LoadedHrtfs;
    LoadedHrtfs = hrtf;
    TRACE("Loaded HRTF support for format: %s %uhz\n",
            DevFmtChannelsString(DevFmtStereo), hrtf->sampleRate);
    entry.hrtf = hrtf;

skip_load:
    /* TODO: Get a human-readable name from the HRTF data (possibly coming in a
     * format update). */
    name = strrchr(al_string_get_cstr(*filename), '/');
    if(!name) name = strrchr(al_string_get_cstr(*filename), '\\');
    if(!name) name = al_string_get_cstr(*filename);
    else ++name;

    ext = strrchr(name, '.');

    i = 0;
    do {
        if(!ext)
            al_string_copy_cstr(&entry.name, name);
        else
            al_string_copy_range(&entry.name, name, ext);
        if(i != 0)
        {
            char str[64];
            snprintf(str, sizeof(str), " #%d", i+1);
            al_string_append_cstr(&entry.name, str);
        }
        ++i;

#define MATCH_NAME(i)  (al_string_cmp(entry.name, (i)->name) == 0)
        VECTOR_FIND_IF(iter, const HrtfEntry, *list, MATCH_NAME);
#undef MATCH_NAME
    } while(iter != VECTOR_END(*list));

    TRACE("Adding entry \"%s\" from file \"%s\"\n", al_string_get_cstr(entry.name),
          al_string_get_cstr(*filename));
    VECTOR_PUSH_BACK(*list, entry);

done:
    al_string_deinit(filename);
}

/* Unfortunate that we have to duplicate AddFileEntry to take a memory buffer
 * for input instead of opening the given filename.
 */
static void AddBuiltInEntry(vector_HrtfEntry *list, const ALubyte *data, size_t datalen, al_string *filename)
{
    HrtfEntry entry = { AL_STRING_INIT_STATIC(), NULL };
    struct Hrtf *hrtf = NULL;
    const HrtfEntry *iter;
    int i;

#define MATCH_FNAME(i) (al_string_cmp_cstr(*filename, (i)->hrtf->filename) == 0)
    VECTOR_FIND_IF(iter, const HrtfEntry, *list, MATCH_FNAME);
    if(iter != VECTOR_END(*list))
    {
        TRACE("Skipping duplicate file entry %s\n", al_string_get_cstr(*filename));
        goto done;
    }
#undef MATCH_FNAME

    entry.hrtf = LoadedHrtfs;
    while(entry.hrtf)
    {
        if(al_string_cmp_cstr(*filename, entry.hrtf->filename) == 0)
        {
            TRACE("Skipping load of already-loaded file %s\n", al_string_get_cstr(*filename));
            goto skip_load;
        }
        entry.hrtf = entry.hrtf->next;
    }

    TRACE("Loading %s...\n", al_string_get_cstr(*filename));
    if(datalen < sizeof(magicMarker01))
    {
        ERR("%s data is too short ("SZFMT" bytes)\n", al_string_get_cstr(*filename), datalen);
        goto done;
    }

    if(memcmp(data, magicMarker01, sizeof(magicMarker01)) == 0)
    {
        TRACE("Detected data set format v1\n");
        hrtf = LoadHrtf01(data+sizeof(magicMarker01),
            datalen-sizeof(magicMarker01), *filename
        );
    }
    else if(memcmp(data, magicMarker00, sizeof(magicMarker00)) == 0)
    {
        TRACE("Detected data set format v0\n");
        hrtf = LoadHrtf00(data+sizeof(magicMarker00),
            datalen-sizeof(magicMarker00), *filename
        );
    }
    else
        ERR("Invalid header in %s: \"%.8s\"\n", al_string_get_cstr(*filename), data);

    if(!hrtf)
    {
        ERR("Failed to load %s\n", al_string_get_cstr(*filename));
        goto done;
    }

    hrtf->next = LoadedHrtfs;
    LoadedHrtfs = hrtf;
    TRACE("Loaded HRTF support for format: %s %uhz\n",
            DevFmtChannelsString(DevFmtStereo), hrtf->sampleRate);
    entry.hrtf = hrtf;

skip_load:
    i = 0;
    do {
        al_string_copy(&entry.name, *filename);
        if(i != 0)
        {
            char str[64];
            snprintf(str, sizeof(str), " #%d", i+1);
            al_string_append_cstr(&entry.name, str);
        }
        ++i;

#define MATCH_NAME(i)  (al_string_cmp(entry.name, (i)->name) == 0)
        VECTOR_FIND_IF(iter, const HrtfEntry, *list, MATCH_NAME);
#undef MATCH_NAME
    } while(iter != VECTOR_END(*list));

    TRACE("Adding built-in entry \"%s\"\n", al_string_get_cstr(entry.name));
    VECTOR_PUSH_BACK(*list, entry);

done:
    al_string_deinit(filename);
}


#ifndef ALSOFT_EMBED_HRTF_DATA
#define IDR_DEFAULT_44100_MHR 0
#define IDR_DEFAULT_48000_MHR 1

static const ALubyte *GetResource(int UNUSED(name), size_t *size)
{
    *size = 0;
    return NULL;
}

#else
#include "hrtf_res.h"

#ifdef _WIN32
static const ALubyte *GetResource(int name, size_t *size)
{
    HMODULE handle;
    HGLOBAL res;
    HRSRC rc;

    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCWSTR)GetResource, &handle
    );
    rc = FindResourceW(handle, MAKEINTRESOURCEW(name), MAKEINTRESOURCEW(MHRTYPE));
    res = LoadResource(handle, rc);

    *size = SizeofResource(handle, rc);
    return LockResource(res);
}

#else

extern const ALubyte _binary_default_44100_mhr_start[] HIDDEN_DECL;
extern const ALubyte _binary_default_44100_mhr_end[] HIDDEN_DECL;
extern const ALubyte _binary_default_44100_mhr_size[] HIDDEN_DECL;

extern const ALubyte _binary_default_48000_mhr_start[] HIDDEN_DECL;
extern const ALubyte _binary_default_48000_mhr_end[] HIDDEN_DECL;
extern const ALubyte _binary_default_48000_mhr_size[] HIDDEN_DECL;

static const ALubyte *GetResource(int name, size_t *size)
{
    if(name == IDR_DEFAULT_44100_MHR)
    {
        /* Make sure all symbols are referenced, to ensure the compiler won't
         * ignore the declarations and lose the visibility attribute used to
         * hide them (would be nice if ld or objcopy could automatically mark
         * them as hidden when generating them, but apparently they can't).
         */
        const void *volatile ptr =_binary_default_44100_mhr_size;
        (void)ptr;
        *size = _binary_default_44100_mhr_end - _binary_default_44100_mhr_start;
        return _binary_default_44100_mhr_start;
    }
    if(name == IDR_DEFAULT_48000_MHR)
    {
        const void *volatile ptr =_binary_default_48000_mhr_size;
        (void)ptr;
        *size = _binary_default_48000_mhr_end - _binary_default_48000_mhr_start;
        return _binary_default_48000_mhr_start;
    }
    *size = 0;
    return NULL;
}
#endif
#endif

vector_HrtfEntry EnumerateHrtf(const_al_string devname)
{
    vector_HrtfEntry list = VECTOR_INIT_STATIC();
    const char *defaulthrtf = "";
    const char *pathlist = "";
    bool usedefaults = true;

    if(ConfigValueStr(al_string_get_cstr(devname), NULL, "hrtf-paths", &pathlist))
    {
        while(pathlist && *pathlist)
        {
            const char *next, *end;

            while(isspace(*pathlist) || *pathlist == ',')
                pathlist++;
            if(*pathlist == '\0')
                continue;

            next = strchr(pathlist, ',');
            if(next)
                end = next++;
            else
            {
                end = pathlist + strlen(pathlist);
                usedefaults = false;
            }

            while(end != pathlist && isspace(*(end-1)))
                --end;
            if(end != pathlist)
            {
                al_string pname = AL_STRING_INIT_STATIC();
                vector_al_string flist;

                al_string_append_range(&pname, pathlist, end);

                flist = SearchDataFiles(".mhr", al_string_get_cstr(pname));
                VECTOR_FOR_EACH_PARAMS(al_string, flist, AddFileEntry, &list);
                VECTOR_DEINIT(flist);

                al_string_deinit(&pname);
            }

            pathlist = next;
        }
    }
    else if(ConfigValueExists(al_string_get_cstr(devname), NULL, "hrtf_tables"))
        ERR("The hrtf_tables option is deprecated, please use hrtf-paths instead.\n");

    if(usedefaults)
    {
        vector_al_string flist;
        const ALubyte *rdata;
        size_t rsize;

        flist = SearchDataFiles(".mhr", "openal/hrtf");
        VECTOR_FOR_EACH_PARAMS(al_string, flist, AddFileEntry, &list);
        VECTOR_DEINIT(flist);

        rdata = GetResource(IDR_DEFAULT_44100_MHR, &rsize);
        if(rdata != NULL && rsize > 0)
        {
            al_string ename = AL_STRING_INIT_STATIC();
            al_string_copy_cstr(&ename, "Built-In 44100hz");
            AddBuiltInEntry(&list, rdata, rsize, &ename);
        }

        rdata = GetResource(IDR_DEFAULT_48000_MHR, &rsize);
        if(rdata != NULL && rsize > 0)
        {
            al_string ename = AL_STRING_INIT_STATIC();
            al_string_copy_cstr(&ename, "Built-In 48000hz");
            AddBuiltInEntry(&list, rdata, rsize, &ename);
        }
    }

    if(VECTOR_SIZE(list) > 1 && ConfigValueStr(al_string_get_cstr(devname), NULL, "default-hrtf", &defaulthrtf))
    {
        const HrtfEntry *iter;
        /* Find the preferred HRTF and move it to the front of the list. */
#define FIND_ENTRY(i)  (al_string_cmp_cstr((i)->name, defaulthrtf) == 0)
        VECTOR_FIND_IF(iter, const HrtfEntry, list, FIND_ENTRY);
#undef FIND_ENTRY
        if(iter == VECTOR_END(list))
            WARN("Failed to find default HRTF \"%s\"\n", defaulthrtf);
        else if(iter != VECTOR_BEGIN(list))
        {
            HrtfEntry entry = *iter;
            memmove(&VECTOR_ELEM(list,1), &VECTOR_ELEM(list,0),
                    (iter-VECTOR_BEGIN(list))*sizeof(HrtfEntry));
            VECTOR_ELEM(list,0) = entry;
        }
    }

    return list;
}

void FreeHrtfList(vector_HrtfEntry *list)
{
#define CLEAR_ENTRY(i) do {           \
    al_string_deinit(&(i)->name);     \
} while(0)
    VECTOR_FOR_EACH(HrtfEntry, *list, CLEAR_ENTRY);
    VECTOR_DEINIT(*list);
#undef CLEAR_ENTRY
}


void FreeHrtfs(void)
{
    struct Hrtf *Hrtf = LoadedHrtfs;
    LoadedHrtfs = NULL;

    while(Hrtf != NULL)
    {
        struct Hrtf *next = Hrtf->next;
        al_free(Hrtf);
        Hrtf = next;
    }
}
