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
#include "hrtf.h"
#include "alconfig.h"
#include "filters/splitter.h"

#include "compat.h"
#include "almalloc.h"


/* Current data set limits defined by the makehrtf utility. */
#define MIN_IR_SIZE                  (8)
#define MAX_IR_SIZE                  (512)
#define MOD_IR_SIZE                  (8)

#define MIN_FD_COUNT                 (1)
#define MAX_FD_COUNT                 (16)

#define MIN_FD_DISTANCE              (50)
#define MAX_FD_DISTANCE              (2500)

#define MIN_EV_COUNT                 (5)
#define MAX_EV_COUNT                 (128)

#define MIN_AZ_COUNT                 (1)
#define MAX_AZ_COUNT                 (128)

#define MAX_HRIR_DELAY               (HRTF_HISTORY_LENGTH-1)

struct HrtfEntry {
    struct HrtfEntry *next;
    struct Hrtf *handle;
    char filename[];
};

static const ALchar magicMarker00[8] = "MinPHR00";
static const ALchar magicMarker01[8] = "MinPHR01";
static const ALchar magicMarker02[8] = "MinPHR02";

/* First value for pass-through coefficients (remaining are 0), used for omni-
 * directional sounds. */
static const ALfloat PassthruCoeff = 0.707106781187f/*sqrt(0.5)*/;

static ATOMIC_FLAG LoadedHrtfLock = ATOMIC_FLAG_INIT;
static struct HrtfEntry *LoadedHrtfs = NULL;


/* Calculate the elevation index given the polar elevation in radians. This
 * will return an index between 0 and (evcount - 1).
 */
static ALsizei CalcEvIndex(ALsizei evcount, ALfloat ev, ALfloat *mu)
{
    ALsizei idx;
    ev = (F_PI_2+ev) * (evcount-1) / F_PI;
    idx = float2int(ev);

    *mu = ev - idx;
    return mini(idx, evcount-1);
}

/* Calculate the azimuth index given the polar azimuth in radians. This will
 * return an index between 0 and (azcount - 1).
 */
static ALsizei CalcAzIndex(ALsizei azcount, ALfloat az, ALfloat *mu)
{
    ALsizei idx;
    az = (F_TAU+az) * azcount / F_TAU;

    idx = float2int(az);
    *mu = az - idx;
    return idx % azcount;
}

/* Calculates static HRIR coefficients and delays for the given polar elevation
 * and azimuth in radians. The coefficients are normalized.
 */
void GetHrtfCoeffs(const struct Hrtf *Hrtf, ALfloat elevation, ALfloat azimuth, ALfloat spread,
                   ALfloat (*restrict coeffs)[2], ALsizei *delays)
{
    ALsizei evidx, azidx, idx[4];
    ALsizei evoffset;
    ALfloat emu, amu[2];
    ALfloat blend[4];
    ALfloat dirfact;
    ALsizei i, c;

    dirfact = 1.0f - (spread / F_TAU);

    /* Claculate the lower elevation index. */
    evidx = CalcEvIndex(Hrtf->evCount, elevation, &emu);
    evoffset = Hrtf->evOffset[evidx];

    /* Calculate lower azimuth index. */
    azidx= CalcAzIndex(Hrtf->azCount[evidx], azimuth, &amu[0]);

    /* Calculate the lower HRIR indices. */
    idx[0] = evoffset + azidx;
    idx[1] = evoffset + ((azidx+1) % Hrtf->azCount[evidx]);
    if(evidx < Hrtf->evCount-1)
    {
        /* Increment elevation to the next (upper) index. */
        evidx++;
        evoffset = Hrtf->evOffset[evidx];

        /* Calculate upper azimuth index. */
        azidx = CalcAzIndex(Hrtf->azCount[evidx], azimuth, &amu[1]);

        /* Calculate the upper HRIR indices. */
        idx[2] = evoffset + azidx;
        idx[3] = evoffset + ((azidx+1) % Hrtf->azCount[evidx]);
    }
    else
    {
        /* If the lower elevation is the top index, the upper elevation is the
         * same as the lower.
         */
        amu[1] = amu[0];
        idx[2] = idx[0];
        idx[3] = idx[1];
    }

    /* Calculate bilinear blending weights, attenuated according to the
     * directional panning factor.
     */
    blend[0] = (1.0f-emu) * (1.0f-amu[0]) * dirfact;
    blend[1] = (1.0f-emu) * (     amu[0]) * dirfact;
    blend[2] = (     emu) * (1.0f-amu[1]) * dirfact;
    blend[3] = (     emu) * (     amu[1]) * dirfact;

    /* Calculate the blended HRIR delays. */
    delays[0] = float2int(
        Hrtf->delays[idx[0]][0]*blend[0] + Hrtf->delays[idx[1]][0]*blend[1] +
        Hrtf->delays[idx[2]][0]*blend[2] + Hrtf->delays[idx[3]][0]*blend[3] + 0.5f
    );
    delays[1] = float2int(
        Hrtf->delays[idx[0]][1]*blend[0] + Hrtf->delays[idx[1]][1]*blend[1] +
        Hrtf->delays[idx[2]][1]*blend[2] + Hrtf->delays[idx[3]][1]*blend[3] + 0.5f
    );

    /* Calculate the sample offsets for the HRIR indices. */
    idx[0] *= Hrtf->irSize;
    idx[1] *= Hrtf->irSize;
    idx[2] *= Hrtf->irSize;
    idx[3] *= Hrtf->irSize;

    ASSUME(Hrtf->irSize >= MIN_IR_SIZE && (Hrtf->irSize%MOD_IR_SIZE) == 0);
    coeffs = ASSUME_ALIGNED(coeffs, 16);
    /* Calculate the blended HRIR coefficients. */
    coeffs[0][0] = PassthruCoeff * (1.0f-dirfact);
    coeffs[0][1] = PassthruCoeff * (1.0f-dirfact);
    for(i = 1;i < Hrtf->irSize;i++)
    {
        coeffs[i][0] = 0.0f;
        coeffs[i][1] = 0.0f;
    }
    for(c = 0;c < 4;c++)
    {
        const ALfloat (*restrict srccoeffs)[2] = ASSUME_ALIGNED(Hrtf->coeffs+idx[c], 16);
        for(i = 0;i < Hrtf->irSize;i++)
        {
            coeffs[i][0] += srccoeffs[i][0] * blend[c];
            coeffs[i][1] += srccoeffs[i][1] * blend[c];
        }
    }
}


void BuildBFormatHrtf(const struct Hrtf *Hrtf, DirectHrtfState *state, ALsizei NumChannels, const struct AngularPoint *AmbiPoints, const ALfloat (*restrict AmbiMatrix)[MAX_AMBI_COEFFS], ALsizei AmbiCount, const ALfloat *restrict AmbiOrderHFGain)
{
/* Set this to 2 for dual-band HRTF processing. May require a higher quality
 * band-splitter, or better calculation of the new IR length to deal with the
 * tail generated by the filter.
 */
#define NUM_BANDS 2
    BandSplitter splitter;
    ALdouble (*tmpres)[HRIR_LENGTH][2];
    ALsizei idx[HRTF_AMBI_MAX_CHANNELS];
    ALsizei min_delay = HRTF_HISTORY_LENGTH;
    ALfloat temps[3][HRIR_LENGTH];
    ALsizei max_length = 0;
    ALsizei i, c, b;

    for(c = 0;c < AmbiCount;c++)
    {
        ALuint evidx, azidx;
        ALuint evoffset;
        ALuint azcount;

        /* Calculate elevation index. */
        evidx = (ALsizei)((F_PI_2+AmbiPoints[c].Elev) * (Hrtf->evCount-1) / F_PI + 0.5f);
        evidx = clampi(evidx, 0, Hrtf->evCount-1);

        azcount = Hrtf->azCount[evidx];
        evoffset = Hrtf->evOffset[evidx];

        /* Calculate azimuth index for this elevation. */
        azidx = (ALsizei)((F_TAU+AmbiPoints[c].Azim) * azcount / F_TAU + 0.5f) % azcount;

        /* Calculate indices for left and right channels. */
        idx[c] = evoffset + azidx;

        min_delay = mini(min_delay, mini(Hrtf->delays[idx[c]][0], Hrtf->delays[idx[c]][1]));
    }

    tmpres = al_calloc(16, NumChannels * sizeof(*tmpres));

    memset(temps, 0, sizeof(temps));
    bandsplit_init(&splitter, 400.0f / (ALfloat)Hrtf->sampleRate);
    for(c = 0;c < AmbiCount;c++)
    {
        const ALfloat (*fir)[2] = &Hrtf->coeffs[idx[c] * Hrtf->irSize];
        ALsizei ldelay = Hrtf->delays[idx[c]][0] - min_delay;
        ALsizei rdelay = Hrtf->delays[idx[c]][1] - min_delay;

        if(NUM_BANDS == 1)
        {
            max_length = maxi(max_length,
                mini(maxi(ldelay, rdelay) + Hrtf->irSize, HRIR_LENGTH)
            );

            for(i = 0;i < NumChannels;++i)
            {
                ALdouble mult = (ALdouble)AmbiOrderHFGain[(ALsizei)sqrt(i)] * AmbiMatrix[c][i];
                ALsizei lidx = ldelay, ridx = rdelay;
                ALsizei j = 0;
                while(lidx < HRIR_LENGTH && ridx < HRIR_LENGTH && j < Hrtf->irSize)
                {
                    tmpres[i][lidx++][0] += fir[j][0] * mult;
                    tmpres[i][ridx++][1] += fir[j][1] * mult;
                    j++;
                }
            }
        }
        else
        {
            /* Increase the IR size by 2/3rds to account for the tail generated
             * by the band-split filter.
             */
            const ALsizei irsize = mini(Hrtf->irSize*5/3, HRIR_LENGTH);

            max_length = maxi(max_length,
                mini(maxi(ldelay, rdelay) + irsize, HRIR_LENGTH)
            );

            /* Band-split left HRIR into low and high frequency responses. */
            bandsplit_clear(&splitter);
            for(i = 0;i < Hrtf->irSize;i++)
                temps[2][i] = fir[i][0];
            bandsplit_process(&splitter, temps[0], temps[1], temps[2], HRIR_LENGTH);

            /* Apply left ear response with delay. */
            for(i = 0;i < NumChannels;++i)
            {
                ALfloat hfgain = AmbiOrderHFGain[(ALsizei)sqrt(i)];
                for(b = 0;b < NUM_BANDS;b++)
                {
                    ALdouble mult = AmbiMatrix[c][i] * (ALdouble)((b==0) ? hfgain : 1.0);
                    ALsizei lidx = ldelay;
                    ALsizei j = 0;
                    while(lidx < HRIR_LENGTH)
                        tmpres[i][lidx++][0] += temps[b][j++] * mult;
                }
            }

            /* Band-split right HRIR into low and high frequency responses. */
            bandsplit_clear(&splitter);
            for(i = 0;i < Hrtf->irSize;i++)
                temps[2][i] = fir[i][1];
            bandsplit_process(&splitter, temps[0], temps[1], temps[2], HRIR_LENGTH);

            /* Apply right ear response with delay. */
            for(i = 0;i < NumChannels;++i)
            {
                ALfloat hfgain = AmbiOrderHFGain[(ALsizei)sqrt(i)];
                for(b = 0;b < NUM_BANDS;b++)
                {
                    ALdouble mult = AmbiMatrix[c][i] * (ALdouble)((b==0) ? hfgain : 1.0);
                    ALsizei ridx = rdelay;
                    ALsizei j = 0;
                    while(ridx < HRIR_LENGTH)
                        tmpres[i][ridx++][1] += temps[b][j++] * mult;
                }
            }
        }
    }
    /* Round up to the next IR size multiple. */
    max_length += MOD_IR_SIZE-1;
    max_length -= max_length%MOD_IR_SIZE;

    for(i = 0;i < NumChannels;++i)
    {
        int idx;
        for(idx = 0;idx < HRIR_LENGTH;idx++)
        {
            state->Chan[i].Coeffs[idx][0] = (ALfloat)tmpres[i][idx][0];
            state->Chan[i].Coeffs[idx][1] = (ALfloat)tmpres[i][idx][1];
        }
    }

    al_free(tmpres);
    tmpres = NULL;

    TRACE("Skipped delay: %d, new FIR length: %d\n", min_delay, max_length);
    state->IrSize = max_length;
#undef NUM_BANDS
}


static struct Hrtf *CreateHrtfStore(ALuint rate, ALsizei irSize,
  ALfloat distance, ALsizei evCount, ALsizei irCount, const ALubyte *azCount,
  const ALushort *evOffset, const ALfloat (*coeffs)[2], const ALubyte (*delays)[2],
  const char *filename)
{
    struct Hrtf *Hrtf;
    size_t total;

    total  = sizeof(struct Hrtf);
    total += sizeof(Hrtf->azCount[0])*evCount;
    total  = RoundUp(total, sizeof(ALushort)); /* Align for ushort fields */
    total += sizeof(Hrtf->evOffset[0])*evCount;
    total  = RoundUp(total, 16); /* Align for coefficients using SIMD */
    total += sizeof(Hrtf->coeffs[0])*irSize*irCount;
    total += sizeof(Hrtf->delays[0])*irCount;

    Hrtf = al_calloc(16, total);
    if(Hrtf == NULL)
        ERR("Out of memory allocating storage for %s.\n", filename);
    else
    {
        uintptr_t offset = sizeof(struct Hrtf);
        char *base = (char*)Hrtf;
        ALushort *_evOffset;
        ALubyte *_azCount;
        ALubyte (*_delays)[2];
        ALfloat (*_coeffs)[2];
        ALsizei i;

        InitRef(&Hrtf->ref, 0);
        Hrtf->sampleRate = rate;
        Hrtf->irSize = irSize;
        Hrtf->distance = distance;
        Hrtf->evCount = evCount;

        /* Set up pointers to storage following the main HRTF struct. */
        _azCount = (ALubyte*)(base + offset);
        offset += sizeof(_azCount[0])*evCount;

        offset = RoundUp(offset, sizeof(ALushort)); /* Align for ushort fields */
        _evOffset = (ALushort*)(base + offset);
        offset += sizeof(_evOffset[0])*evCount;

        offset = RoundUp(offset, 16); /* Align for coefficients using SIMD */
        _coeffs = (ALfloat(*)[2])(base + offset);
        offset += sizeof(_coeffs[0])*irSize*irCount;

        _delays = (ALubyte(*)[2])(base + offset);
        offset += sizeof(_delays[0])*irCount;

        assert(offset == total);

        /* Copy input data to storage. */
        for(i = 0;i < evCount;i++) _azCount[i] = azCount[i];
        for(i = 0;i < evCount;i++) _evOffset[i] = evOffset[i];
        for(i = 0;i < irSize*irCount;i++)
        {
            _coeffs[i][0] = coeffs[i][0];
            _coeffs[i][1] = coeffs[i][1];
        }
        for(i = 0;i < irCount;i++)
        {
            _delays[i][0] = delays[i][0];
            _delays[i][1] = delays[i][1];
        }

        /* Finally, assign the storage pointers. */
        Hrtf->azCount = _azCount;
        Hrtf->evOffset = _evOffset;
        Hrtf->coeffs = _coeffs;
        Hrtf->delays = _delays;
    }

    return Hrtf;
}

static ALubyte GetLE_ALubyte(const ALubyte **data, size_t *len)
{
    ALubyte ret = (*data)[0];
    *data += 1; *len -= 1;
    return ret;
}

static ALshort GetLE_ALshort(const ALubyte **data, size_t *len)
{
    ALshort ret = (*data)[0] | ((*data)[1]<<8);
    *data += 2; *len -= 2;
    return ret;
}

static ALushort GetLE_ALushort(const ALubyte **data, size_t *len)
{
    ALushort ret = (*data)[0] | ((*data)[1]<<8);
    *data += 2; *len -= 2;
    return ret;
}

static ALint GetLE_ALint24(const ALubyte **data, size_t *len)
{
    ALint ret = (*data)[0] | ((*data)[1]<<8) | ((*data)[2]<<16);
    *data += 3; *len -= 3;
    return (ret^0x800000) - 0x800000;
}

static ALuint GetLE_ALuint(const ALubyte **data, size_t *len)
{
    ALuint ret = (*data)[0] | ((*data)[1]<<8) | ((*data)[2]<<16) | ((*data)[3]<<24);
    *data += 4; *len -= 4;
    return ret;
}

static const ALubyte *Get_ALubytePtr(const ALubyte **data, size_t *len, size_t size)
{
    const ALubyte *ret = *data;
    *data += size; *len -= size;
    return ret;
}

static struct Hrtf *LoadHrtf00(const ALubyte *data, size_t datalen, const char *filename)
{
    struct Hrtf *Hrtf = NULL;
    ALboolean failed = AL_FALSE;
    ALuint rate = 0;
    ALushort irCount = 0;
    ALushort irSize = 0;
    ALubyte evCount = 0;
    ALubyte *azCount = NULL;
    ALushort *evOffset = NULL;
    ALfloat (*coeffs)[2] = NULL;
    ALubyte (*delays)[2] = NULL;
    ALsizei i, j;

    if(datalen < 9)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT")\n", filename, 9, datalen);
        return NULL;
    }

    rate = GetLE_ALuint(&data, &datalen);

    irCount = GetLE_ALushort(&data, &datalen);

    irSize = GetLE_ALushort(&data, &datalen);

    evCount = GetLE_ALubyte(&data, &datalen);

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

    if(datalen < evCount*2u)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT")\n", filename, evCount*2, datalen);
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
        evOffset[0] = GetLE_ALushort(&data, &datalen);
        for(i = 1;i < evCount;i++)
        {
            evOffset[i] = GetLE_ALushort(&data, &datalen);
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
        delays = malloc(sizeof(delays[0])*irCount);
        if(coeffs == NULL || delays == NULL)
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
                filename, reqsize, datalen);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        for(i = 0;i < irCount;i++)
        {
            for(j = 0;j < irSize;j++)
                coeffs[i*irSize + j][0] = GetLE_ALshort(&data, &datalen) / 32768.0f;
        }

        for(i = 0;i < irCount;i++)
        {
            delays[i][0] = GetLE_ALubyte(&data, &datalen);
            if(delays[i][0] > MAX_HRIR_DELAY)
            {
                ERR("Invalid delays[%d]: %d (%d)\n", i, delays[i][0], MAX_HRIR_DELAY);
                failed = AL_TRUE;
            }
        }
    }

    if(!failed)
    {
        /* Mirror the left ear responses to the right ear. */
        for(i = 0;i < evCount;i++)
        {
            ALushort evoffset = evOffset[i];
            ALubyte azcount = azCount[i];
            for(j = 0;j < azcount;j++)
            {
                ALsizei lidx = evoffset + j;
                ALsizei ridx = evoffset + ((azcount-j) % azcount);
                ALsizei k;

                for(k = 0;k < irSize;k++)
                    coeffs[ridx*irSize + k][1] = coeffs[lidx*irSize + k][0];
                delays[ridx][1] = delays[lidx][0];
            }
        }

        Hrtf = CreateHrtfStore(rate, irSize, 0.0f, evCount, irCount, azCount,
                               evOffset, coeffs, delays, filename);
    }

    free(azCount);
    free(evOffset);
    free(coeffs);
    free(delays);
    return Hrtf;
}

static struct Hrtf *LoadHrtf01(const ALubyte *data, size_t datalen, const char *filename)
{
    struct Hrtf *Hrtf = NULL;
    ALboolean failed = AL_FALSE;
    ALuint rate = 0;
    ALushort irCount = 0;
    ALushort irSize = 0;
    ALubyte evCount = 0;
    const ALubyte *azCount = NULL;
    ALushort *evOffset = NULL;
    ALfloat (*coeffs)[2] = NULL;
    ALubyte (*delays)[2] = NULL;
    ALsizei i, j;

    if(datalen < 6)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n", filename, 6, datalen);
        return NULL;
    }

    rate = GetLE_ALuint(&data, &datalen);

    irSize = GetLE_ALubyte(&data, &datalen);

    evCount = GetLE_ALubyte(&data, &datalen);

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
        ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n", filename, evCount, datalen);
        return NULL;
    }

    azCount = Get_ALubytePtr(&data, &datalen, evCount);

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
        delays = malloc(sizeof(delays[0])*irCount);
        if(coeffs == NULL || delays == NULL)
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
                filename, reqsize, datalen);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        for(i = 0;i < irCount;i++)
        {
            for(j = 0;j < irSize;j++)
                coeffs[i*irSize + j][0] = GetLE_ALshort(&data, &datalen) / 32768.0f;
        }

        for(i = 0;i < irCount;i++)
        {
            delays[i][0] = GetLE_ALubyte(&data, &datalen);
            if(delays[i][0] > MAX_HRIR_DELAY)
            {
                ERR("Invalid delays[%d]: %d (%d)\n", i, delays[i][0], MAX_HRIR_DELAY);
                failed = AL_TRUE;
            }
        }
    }

    if(!failed)
    {
        /* Mirror the left ear responses to the right ear. */
        for(i = 0;i < evCount;i++)
        {
            ALushort evoffset = evOffset[i];
            ALubyte azcount = azCount[i];
            for(j = 0;j < azcount;j++)
            {
                ALsizei lidx = evoffset + j;
                ALsizei ridx = evoffset + ((azcount-j) % azcount);
                ALsizei k;

                for(k = 0;k < irSize;k++)
                    coeffs[ridx*irSize + k][1] = coeffs[lidx*irSize + k][0];
                delays[ridx][1] = delays[lidx][0];
            }
        }

        Hrtf = CreateHrtfStore(rate, irSize, 0.0f, evCount, irCount, azCount,
                               evOffset, coeffs, delays, filename);
    }

    free(evOffset);
    free(coeffs);
    free(delays);
    return Hrtf;
}

#define SAMPLETYPE_S16 0
#define SAMPLETYPE_S24 1

#define CHANTYPE_LEFTONLY  0
#define CHANTYPE_LEFTRIGHT 1

static struct Hrtf *LoadHrtf02(const ALubyte *data, size_t datalen, const char *filename)
{
    struct Hrtf *Hrtf = NULL;
    ALboolean failed = AL_FALSE;
    ALuint rate = 0;
    ALubyte sampleType;
    ALubyte channelType;
    ALushort irCount = 0;
    ALushort irSize = 0;
    ALubyte fdCount = 0;
    ALushort distance = 0;
    ALubyte evCount = 0;
    const ALubyte *azCount = NULL;
    ALushort *evOffset = NULL;
    ALfloat (*coeffs)[2] = NULL;
    ALubyte (*delays)[2] = NULL;
    ALsizei i, j;

    if(datalen < 8)
    {
        ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n", filename, 8, datalen);
        return NULL;
    }

    rate = GetLE_ALuint(&data, &datalen);
    sampleType = GetLE_ALubyte(&data, &datalen);
    channelType = GetLE_ALubyte(&data, &datalen);

    irSize = GetLE_ALubyte(&data, &datalen);

    fdCount = GetLE_ALubyte(&data, &datalen);

    if(sampleType > SAMPLETYPE_S24)
    {
        ERR("Unsupported sample type: %d\n", sampleType);
        failed = AL_TRUE;
    }
    if(channelType > CHANTYPE_LEFTRIGHT)
    {
        ERR("Unsupported channel type: %d\n", channelType);
        failed = AL_TRUE;
    }

    if(irSize < MIN_IR_SIZE || irSize > MAX_IR_SIZE || (irSize%MOD_IR_SIZE))
    {
        ERR("Unsupported HRIR size: irSize=%d (%d to %d by %d)\n",
            irSize, MIN_IR_SIZE, MAX_IR_SIZE, MOD_IR_SIZE);
        failed = AL_TRUE;
    }
    if(fdCount != 1)
    {
        ERR("Multiple field-depths not supported: fdCount=%d (%d to %d)\n",
            evCount, MIN_FD_COUNT, MAX_FD_COUNT);
        failed = AL_TRUE;
    }
    if(failed)
        return NULL;

    for(i = 0;i < fdCount;i++)
    {
        if(datalen < 3)
        {
            ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n", filename, 3, datalen);
            return NULL;
        }

        distance = GetLE_ALushort(&data, &datalen);
        if(distance < MIN_FD_DISTANCE || distance > MAX_FD_DISTANCE)
        {
            ERR("Unsupported field distance: distance=%d (%dmm to %dmm)\n",
                distance, MIN_FD_DISTANCE, MAX_FD_DISTANCE);
            failed = AL_TRUE;
        }

        evCount = GetLE_ALubyte(&data, &datalen);
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
            ERR("Unexpected end of %s data (req %d, rem "SZFMT"\n", filename, evCount, datalen);
            return NULL;
        }

        azCount = Get_ALubytePtr(&data, &datalen, evCount);
        for(j = 0;j < evCount;j++)
        {
            if(azCount[j] < MIN_AZ_COUNT || azCount[j] > MAX_AZ_COUNT)
            {
                ERR("Unsupported azimuth count: azCount[%d]=%d (%d to %d)\n",
                    j, azCount[j], MIN_AZ_COUNT, MAX_AZ_COUNT);
                failed = AL_TRUE;
            }
        }
    }
    if(failed)
        return NULL;

    evOffset = malloc(sizeof(evOffset[0])*evCount);
    if(azCount == NULL || evOffset == NULL)
    {
        ERR("Out of memory.\n");
        failed = AL_TRUE;
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
        delays = malloc(sizeof(delays[0])*irCount);
        if(coeffs == NULL || delays == NULL)
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
                filename, reqsize, datalen);
            failed = AL_TRUE;
        }
    }

    if(!failed)
    {
        if(channelType == CHANTYPE_LEFTONLY)
        {
            if(sampleType == SAMPLETYPE_S16)
                for(i = 0;i < irCount;i++)
                {
                    for(j = 0;j < irSize;j++)
                        coeffs[i*irSize + j][0] = GetLE_ALshort(&data, &datalen) / 32768.0f;
                }
            else if(sampleType == SAMPLETYPE_S24)
                for(i = 0;i < irCount;i++)
                {
                    for(j = 0;j < irSize;j++)
                        coeffs[i*irSize + j][0] = GetLE_ALint24(&data, &datalen) / 8388608.0f;
                }

            for(i = 0;i < irCount;i++)
            {
                delays[i][0] = GetLE_ALubyte(&data, &datalen);
                if(delays[i][0] > MAX_HRIR_DELAY)
                {
                    ERR("Invalid delays[%d][0]: %d (%d)\n", i, delays[i][0], MAX_HRIR_DELAY);
                    failed = AL_TRUE;
                }
            }
        }
        else if(channelType == CHANTYPE_LEFTRIGHT)
        {
            if(sampleType == SAMPLETYPE_S16)
                for(i = 0;i < irCount;i++)
                {
                    for(j = 0;j < irSize;j++)
                    {
                        coeffs[i*irSize + j][0] = GetLE_ALshort(&data, &datalen) / 32768.0f;
                        coeffs[i*irSize + j][1] = GetLE_ALshort(&data, &datalen) / 32768.0f;
                    }
                }
            else if(sampleType == SAMPLETYPE_S24)
                for(i = 0;i < irCount;i++)
                {
                    for(j = 0;j < irSize;j++)
                    {
                        coeffs[i*irSize + j][0] = GetLE_ALint24(&data, &datalen) / 8388608.0f;
                        coeffs[i*irSize + j][1] = GetLE_ALint24(&data, &datalen) / 8388608.0f;
                    }
                }

            for(i = 0;i < irCount;i++)
            {
                delays[i][0] = GetLE_ALubyte(&data, &datalen);
                if(delays[i][0] > MAX_HRIR_DELAY)
                {
                    ERR("Invalid delays[%d][0]: %d (%d)\n", i, delays[i][0], MAX_HRIR_DELAY);
                    failed = AL_TRUE;
                }
                delays[i][1] = GetLE_ALubyte(&data, &datalen);
                if(delays[i][1] > MAX_HRIR_DELAY)
                {
                    ERR("Invalid delays[%d][1]: %d (%d)\n", i, delays[i][1], MAX_HRIR_DELAY);
                    failed = AL_TRUE;
                }
            }
        }
    }

    if(!failed)
    {
        if(channelType == CHANTYPE_LEFTONLY)
        {
            /* Mirror the left ear responses to the right ear. */
            for(i = 0;i < evCount;i++)
            {
                ALushort evoffset = evOffset[i];
                ALubyte azcount = azCount[i];
                for(j = 0;j < azcount;j++)
                {
                    ALsizei lidx = evoffset + j;
                    ALsizei ridx = evoffset + ((azcount-j) % azcount);
                    ALsizei k;

                    for(k = 0;k < irSize;k++)
                        coeffs[ridx*irSize + k][1] = coeffs[lidx*irSize + k][0];
                    delays[ridx][1] = delays[lidx][0];
                }
            }
        }

        Hrtf = CreateHrtfStore(rate, irSize,
            (ALfloat)distance / 1000.0f, evCount, irCount, azCount, evOffset,
            coeffs, delays, filename
        );
    }

    free(evOffset);
    free(coeffs);
    free(delays);
    return Hrtf;
}


static void AddFileEntry(vector_EnumeratedHrtf *list, const_al_string filename)
{
    EnumeratedHrtf entry = { AL_STRING_INIT_STATIC(), NULL };
    struct HrtfEntry *loaded_entry;
    const EnumeratedHrtf *iter;
    const char *name;
    const char *ext;
    int i;

    /* Check if this file has already been loaded globally. */
    loaded_entry = LoadedHrtfs;
    while(loaded_entry)
    {
        if(alstr_cmp_cstr(filename, loaded_entry->filename) == 0)
        {
            /* Check if this entry has already been added to the list. */
#define MATCH_ENTRY(i) (loaded_entry == (i)->hrtf)
            VECTOR_FIND_IF(iter, const EnumeratedHrtf, *list, MATCH_ENTRY);
            if(iter != VECTOR_END(*list))
            {
                TRACE("Skipping duplicate file entry %s\n", alstr_get_cstr(filename));
                return;
            }
#undef MATCH_FNAME

            break;
        }
        loaded_entry = loaded_entry->next;
    }

    if(!loaded_entry)
    {
        TRACE("Got new file \"%s\"\n", alstr_get_cstr(filename));

        loaded_entry = al_calloc(DEF_ALIGN,
            FAM_SIZE(struct HrtfEntry, filename, alstr_length(filename)+1)
        );
        loaded_entry->next = LoadedHrtfs;
        loaded_entry->handle = NULL;
        strcpy(loaded_entry->filename, alstr_get_cstr(filename));
        LoadedHrtfs = loaded_entry;
    }

    /* TODO: Get a human-readable name from the HRTF data (possibly coming in a
     * format update). */
    name = strrchr(alstr_get_cstr(filename), '/');
    if(!name) name = strrchr(alstr_get_cstr(filename), '\\');
    if(!name) name = alstr_get_cstr(filename);
    else ++name;

    ext = strrchr(name, '.');

    i = 0;
    do {
        if(!ext)
            alstr_copy_cstr(&entry.name, name);
        else
            alstr_copy_range(&entry.name, name, ext);
        if(i != 0)
        {
            char str[64];
            snprintf(str, sizeof(str), " #%d", i+1);
            alstr_append_cstr(&entry.name, str);
        }
        ++i;

#define MATCH_NAME(i)  (alstr_cmp(entry.name, (i)->name) == 0)
        VECTOR_FIND_IF(iter, const EnumeratedHrtf, *list, MATCH_NAME);
#undef MATCH_NAME
    } while(iter != VECTOR_END(*list));
    entry.hrtf = loaded_entry;

    TRACE("Adding entry \"%s\" from file \"%s\"\n", alstr_get_cstr(entry.name),
          alstr_get_cstr(filename));
    VECTOR_PUSH_BACK(*list, entry);
}

/* Unfortunate that we have to duplicate AddFileEntry to take a memory buffer
 * for input instead of opening the given filename.
 */
static void AddBuiltInEntry(vector_EnumeratedHrtf *list, const_al_string filename, ALuint residx)
{
    EnumeratedHrtf entry = { AL_STRING_INIT_STATIC(), NULL };
    struct HrtfEntry *loaded_entry;
    struct Hrtf *hrtf = NULL;
    const EnumeratedHrtf *iter;
    const char *name;
    const char *ext;
    int i;

    loaded_entry = LoadedHrtfs;
    while(loaded_entry)
    {
        if(alstr_cmp_cstr(filename, loaded_entry->filename) == 0)
        {
#define MATCH_ENTRY(i) (loaded_entry == (i)->hrtf)
            VECTOR_FIND_IF(iter, const EnumeratedHrtf, *list, MATCH_ENTRY);
            if(iter != VECTOR_END(*list))
            {
                TRACE("Skipping duplicate file entry %s\n", alstr_get_cstr(filename));
                return;
            }
#undef MATCH_FNAME

            break;
        }
        loaded_entry = loaded_entry->next;
    }

    if(!loaded_entry)
    {
        size_t namelen = alstr_length(filename)+32;

        TRACE("Got new file \"%s\"\n", alstr_get_cstr(filename));

        loaded_entry = al_calloc(DEF_ALIGN,
            FAM_SIZE(struct HrtfEntry, filename, namelen)
        );
        loaded_entry->next = LoadedHrtfs;
        loaded_entry->handle = hrtf;
        snprintf(loaded_entry->filename, namelen,  "!%u_%s",
                 residx, alstr_get_cstr(filename));
        LoadedHrtfs = loaded_entry;
    }

    /* TODO: Get a human-readable name from the HRTF data (possibly coming in a
     * format update). */
    name = strrchr(alstr_get_cstr(filename), '/');
    if(!name) name = strrchr(alstr_get_cstr(filename), '\\');
    if(!name) name = alstr_get_cstr(filename);
    else ++name;

    ext = strrchr(name, '.');

    i = 0;
    do {
        if(!ext)
            alstr_copy_cstr(&entry.name, name);
        else
            alstr_copy_range(&entry.name, name, ext);
        if(i != 0)
        {
            char str[64];
            snprintf(str, sizeof(str), " #%d", i+1);
            alstr_append_cstr(&entry.name, str);
        }
        ++i;

#define MATCH_NAME(i)  (alstr_cmp(entry.name, (i)->name) == 0)
        VECTOR_FIND_IF(iter, const EnumeratedHrtf, *list, MATCH_NAME);
#undef MATCH_NAME
    } while(iter != VECTOR_END(*list));
    entry.hrtf = loaded_entry;

    TRACE("Adding built-in entry \"%s\"\n", alstr_get_cstr(entry.name));
    VECTOR_PUSH_BACK(*list, entry);
}


#define IDR_DEFAULT_44100_MHR 1
#define IDR_DEFAULT_48000_MHR 2

#ifndef ALSOFT_EMBED_HRTF_DATA

static const ALubyte *GetResource(int UNUSED(name), size_t *size)
{
    *size = 0;
    return NULL;
}

#else

#include "default-44100.mhr.h"
#include "default-48000.mhr.h"

static const ALubyte *GetResource(int name, size_t *size)
{
    if(name == IDR_DEFAULT_44100_MHR)
    {
        *size = sizeof(hrtf_default_44100);
        return hrtf_default_44100;
    }
    if(name == IDR_DEFAULT_48000_MHR)
    {
        *size = sizeof(hrtf_default_48000);
        return hrtf_default_48000;
    }
    *size = 0;
    return NULL;
}
#endif

vector_EnumeratedHrtf EnumerateHrtf(const_al_string devname)
{
    vector_EnumeratedHrtf list = VECTOR_INIT_STATIC();
    const char *defaulthrtf = "";
    const char *pathlist = "";
    bool usedefaults = true;

    if(ConfigValueStr(alstr_get_cstr(devname), NULL, "hrtf-paths", &pathlist))
    {
        al_string pname = AL_STRING_INIT_STATIC();
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
                vector_al_string flist;
                size_t i;

                alstr_copy_range(&pname, pathlist, end);

                flist = SearchDataFiles(".mhr", alstr_get_cstr(pname));
                for(i = 0;i < VECTOR_SIZE(flist);i++)
                    AddFileEntry(&list, VECTOR_ELEM(flist, i));
                VECTOR_FOR_EACH(al_string, flist, alstr_reset);
                VECTOR_DEINIT(flist);
            }

            pathlist = next;
        }

        alstr_reset(&pname);
    }
    else if(ConfigValueExists(alstr_get_cstr(devname), NULL, "hrtf_tables"))
        ERR("The hrtf_tables option is deprecated, please use hrtf-paths instead.\n");

    if(usedefaults)
    {
        al_string ename = AL_STRING_INIT_STATIC();
        vector_al_string flist;
        const ALubyte *rdata;
        size_t rsize, i;

        flist = SearchDataFiles(".mhr", "openal/hrtf");
        for(i = 0;i < VECTOR_SIZE(flist);i++)
            AddFileEntry(&list, VECTOR_ELEM(flist, i));
        VECTOR_FOR_EACH(al_string, flist, alstr_reset);
        VECTOR_DEINIT(flist);

        rdata = GetResource(IDR_DEFAULT_44100_MHR, &rsize);
        if(rdata != NULL && rsize > 0)
        {
            alstr_copy_cstr(&ename, "Built-In 44100hz");
            AddBuiltInEntry(&list, ename, IDR_DEFAULT_44100_MHR);
        }

        rdata = GetResource(IDR_DEFAULT_48000_MHR, &rsize);
        if(rdata != NULL && rsize > 0)
        {
            alstr_copy_cstr(&ename, "Built-In 48000hz");
            AddBuiltInEntry(&list, ename, IDR_DEFAULT_48000_MHR);
        }
        alstr_reset(&ename);
    }

    if(VECTOR_SIZE(list) > 1 && ConfigValueStr(alstr_get_cstr(devname), NULL, "default-hrtf", &defaulthrtf))
    {
        const EnumeratedHrtf *iter;
        /* Find the preferred HRTF and move it to the front of the list. */
#define FIND_ENTRY(i)  (alstr_cmp_cstr((i)->name, defaulthrtf) == 0)
        VECTOR_FIND_IF(iter, const EnumeratedHrtf, list, FIND_ENTRY);
#undef FIND_ENTRY
        if(iter == VECTOR_END(list))
            WARN("Failed to find default HRTF \"%s\"\n", defaulthrtf);
        else if(iter != VECTOR_BEGIN(list))
        {
            EnumeratedHrtf entry = *iter;
            memmove(&VECTOR_ELEM(list,1), &VECTOR_ELEM(list,0),
                    (iter-VECTOR_BEGIN(list))*sizeof(EnumeratedHrtf));
            VECTOR_ELEM(list,0) = entry;
        }
    }

    return list;
}

void FreeHrtfList(vector_EnumeratedHrtf *list)
{
#define CLEAR_ENTRY(i) alstr_reset(&(i)->name)
    VECTOR_FOR_EACH(EnumeratedHrtf, *list, CLEAR_ENTRY);
    VECTOR_DEINIT(*list);
#undef CLEAR_ENTRY
}

struct Hrtf *GetLoadedHrtf(struct HrtfEntry *entry)
{
    struct Hrtf *hrtf = NULL;
    struct FileMapping fmap;
    const ALubyte *rdata;
    const char *name;
    ALuint residx;
    size_t rsize;
    char ch;

    while(ATOMIC_FLAG_TEST_AND_SET(&LoadedHrtfLock, almemory_order_seq_cst))
        althrd_yield();

    if(entry->handle)
    {
        hrtf = entry->handle;
        Hrtf_IncRef(hrtf);
        goto done;
    }

    fmap.ptr = NULL;
    fmap.len = 0;
    if(sscanf(entry->filename, "!%u%c", &residx, &ch) == 2 && ch == '_')
    {
        name = strchr(entry->filename, ch)+1;

        TRACE("Loading %s...\n", name);
        rdata = GetResource(residx, &rsize);
        if(rdata == NULL || rsize == 0)
        {
            ERR("Could not get resource %u, %s\n", residx, name);
            goto done;
        }
    }
    else
    {
        name = entry->filename;

        TRACE("Loading %s...\n", entry->filename);
        fmap = MapFileToMem(entry->filename);
        if(fmap.ptr == NULL)
        {
            ERR("Could not open %s\n", entry->filename);
            goto done;
        }

        rdata = fmap.ptr;
        rsize = fmap.len;
    }

    if(rsize < sizeof(magicMarker02))
        ERR("%s data is too short ("SZFMT" bytes)\n", name, rsize);
    else if(memcmp(rdata, magicMarker02, sizeof(magicMarker02)) == 0)
    {
        TRACE("Detected data set format v2\n");
        hrtf = LoadHrtf02(rdata+sizeof(magicMarker02),
            rsize-sizeof(magicMarker02), name
        );
    }
    else if(memcmp(rdata, magicMarker01, sizeof(magicMarker01)) == 0)
    {
        TRACE("Detected data set format v1\n");
        hrtf = LoadHrtf01(rdata+sizeof(magicMarker01),
            rsize-sizeof(magicMarker01), name
        );
    }
    else if(memcmp(rdata, magicMarker00, sizeof(magicMarker00)) == 0)
    {
        TRACE("Detected data set format v0\n");
        hrtf = LoadHrtf00(rdata+sizeof(magicMarker00),
            rsize-sizeof(magicMarker00), name
        );
    }
    else
        ERR("Invalid header in %s: \"%.8s\"\n", name, (const char*)rdata);
    if(fmap.ptr)
        UnmapFileMem(&fmap);

    if(!hrtf)
    {
        ERR("Failed to load %s\n", name);
        goto done;
    }
    entry->handle = hrtf;
    Hrtf_IncRef(hrtf);

    TRACE("Loaded HRTF support for format: %s %uhz\n",
          DevFmtChannelsString(DevFmtStereo), hrtf->sampleRate);

done:
    ATOMIC_FLAG_CLEAR(&LoadedHrtfLock, almemory_order_seq_cst);
    return hrtf;
}


void Hrtf_IncRef(struct Hrtf *hrtf)
{
    uint ref = IncrementRef(&hrtf->ref);
    TRACEREF("%p increasing refcount to %u\n", hrtf, ref);
}

void Hrtf_DecRef(struct Hrtf *hrtf)
{
    struct HrtfEntry *Hrtf;
    uint ref = DecrementRef(&hrtf->ref);
    TRACEREF("%p decreasing refcount to %u\n", hrtf, ref);
    if(ref == 0)
    {
        while(ATOMIC_FLAG_TEST_AND_SET(&LoadedHrtfLock, almemory_order_seq_cst))
            althrd_yield();

        Hrtf = LoadedHrtfs;
        while(Hrtf != NULL)
        {
            /* Need to double-check that it's still unused, as another device
             * could've reacquired this HRTF after its reference went to 0 and
             * before the lock was taken.
             */
            if(hrtf == Hrtf->handle && ReadRef(&hrtf->ref) == 0)
            {
                al_free(Hrtf->handle);
                Hrtf->handle = NULL;
                TRACE("Unloaded unused HRTF %s\n", Hrtf->filename);
            }
            Hrtf = Hrtf->next;
        }

        ATOMIC_FLAG_CLEAR(&LoadedHrtfLock, almemory_order_seq_cst);
    }
}


void FreeHrtfs(void)
{
    struct HrtfEntry *Hrtf = LoadedHrtfs;
    LoadedHrtfs = NULL;

    while(Hrtf != NULL)
    {
        struct HrtfEntry *next = Hrtf->next;
        al_free(Hrtf->handle);
        al_free(Hrtf);
        Hrtf = next;
    }
}
