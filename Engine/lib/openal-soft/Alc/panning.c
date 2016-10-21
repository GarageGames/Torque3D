/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2010 by authors.
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "alMain.h"
#include "alAuxEffectSlot.h"
#include "alu.h"
#include "bool.h"
#include "ambdec.h"
#include "bformatdec.h"
#include "uhjfilter.h"
#include "bs2b.h"


extern inline void CalcXYZCoeffs(ALfloat x, ALfloat y, ALfloat z, ALfloat spread, ALfloat coeffs[MAX_AMBI_COEFFS]);


#define ZERO_ORDER_SCALE    0.0f
#define FIRST_ORDER_SCALE   1.0f
#define SECOND_ORDER_SCALE  (1.0f / 1.22474f)
#define THIRD_ORDER_SCALE   (1.0f / 1.30657f)


static const ALuint FuMa2ACN[MAX_AMBI_COEFFS] = {
    0,  /* W */
    3,  /* X */
    1,  /* Y */
    2,  /* Z */
    6,  /* R */
    7,  /* S */
    5,  /* T */
    8,  /* U */
    4,  /* V */
    12, /* K */
    13, /* L */
    11, /* M */
    14, /* N */
    10, /* O */
    15, /* P */
    9,  /* Q */
};
static const ALuint ACN2ACN[MAX_AMBI_COEFFS] = {
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9, 10, 11, 12, 13, 14, 15
};

/* NOTE: These are scale factors as applied to Ambisonics content. Decoder
 * coefficients should be divided by these values to get proper N3D scalings.
 */
static const ALfloat UnitScale[MAX_AMBI_COEFFS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};
static const ALfloat SN3D2N3DScale[MAX_AMBI_COEFFS] = {
    1.000000000f, /* ACN  0 (W), sqrt(1) */
    1.732050808f, /* ACN  1 (Y), sqrt(3) */
    1.732050808f, /* ACN  2 (Z), sqrt(3) */
    1.732050808f, /* ACN  3 (X), sqrt(3) */
    2.236067978f, /* ACN  4 (V), sqrt(5) */
    2.236067978f, /* ACN  5 (T), sqrt(5) */
    2.236067978f, /* ACN  6 (R), sqrt(5) */
    2.236067978f, /* ACN  7 (S), sqrt(5) */
    2.236067978f, /* ACN  8 (U), sqrt(5) */
    2.645751311f, /* ACN  9 (Q), sqrt(7) */
    2.645751311f, /* ACN 10 (O), sqrt(7) */
    2.645751311f, /* ACN 11 (M), sqrt(7) */
    2.645751311f, /* ACN 12 (K), sqrt(7) */
    2.645751311f, /* ACN 13 (L), sqrt(7) */
    2.645751311f, /* ACN 14 (N), sqrt(7) */
    2.645751311f, /* ACN 15 (P), sqrt(7) */
};
static const ALfloat FuMa2N3DScale[MAX_AMBI_COEFFS] = {
    1.414213562f, /* ACN  0 (W), sqrt(2) */
    1.732050808f, /* ACN  1 (Y), sqrt(3) */
    1.732050808f, /* ACN  2 (Z), sqrt(3) */
    1.732050808f, /* ACN  3 (X), sqrt(3) */
    1.936491673f, /* ACN  4 (V), sqrt(15)/2 */
    1.936491673f, /* ACN  5 (T), sqrt(15)/2 */
    2.236067978f, /* ACN  6 (R), sqrt(5) */
    1.936491673f, /* ACN  7 (S), sqrt(15)/2 */
    1.936491673f, /* ACN  8 (U), sqrt(15)/2 */
    2.091650066f, /* ACN  9 (Q), sqrt(35/8) */
    1.972026594f, /* ACN 10 (O), sqrt(35)/3 */
    2.231093404f, /* ACN 11 (M), sqrt(224/45) */
    2.645751311f, /* ACN 12 (K), sqrt(7) */
    2.231093404f, /* ACN 13 (L), sqrt(224/45) */
    1.972026594f, /* ACN 14 (N), sqrt(35)/3 */
    2.091650066f, /* ACN 15 (P), sqrt(35/8) */
};


void CalcDirectionCoeffs(const ALfloat dir[3], ALfloat spread, ALfloat coeffs[MAX_AMBI_COEFFS])
{
    /* Convert from OpenAL coords to Ambisonics. */
    ALfloat x = -dir[2];
    ALfloat y = -dir[0];
    ALfloat z =  dir[1];

    /* Zeroth-order */
    coeffs[0]  = 1.0f; /* ACN 0 = 1 */
    /* First-order */
    coeffs[1]  = 1.732050808f * y; /* ACN 1 = sqrt(3) * Y */
    coeffs[2]  = 1.732050808f * z; /* ACN 2 = sqrt(3) * Z */
    coeffs[3]  = 1.732050808f * x; /* ACN 3 = sqrt(3) * X */
    /* Second-order */
    coeffs[4]  = 3.872983346f * x * y;             /* ACN 4 = sqrt(15) * X * Y */
    coeffs[5]  = 3.872983346f * y * z;             /* ACN 5 = sqrt(15) * Y * Z */
    coeffs[6]  = 1.118033989f * (3.0f*z*z - 1.0f); /* ACN 6 = sqrt(5)/2 * (3*Z*Z - 1) */
    coeffs[7]  = 3.872983346f * x * z;             /* ACN 7 = sqrt(15) * X * Z */
    coeffs[8]  = 1.936491673f * (x*x - y*y);       /* ACN 8 = sqrt(15)/2 * (X*X - Y*Y) */
    /* Third-order */
    coeffs[9]  =  2.091650066f * y * (3.0f*x*x - y*y);  /* ACN  9 = sqrt(35/8) * Y * (3*X*X - Y*Y) */
    coeffs[10] = 10.246950766f * z * x * y;             /* ACN 10 = sqrt(105) * Z * X * Y */
    coeffs[11] =  1.620185175f * y * (5.0f*z*z - 1.0f); /* ACN 11 = sqrt(21/8) * Y * (5*Z*Z - 1) */
    coeffs[12] =  1.322875656f * z * (5.0f*z*z - 3.0f); /* ACN 12 = sqrt(7)/2 * Z * (5*Z*Z - 3) */
    coeffs[13] =  1.620185175f * x * (5.0f*z*z - 1.0f); /* ACN 13 = sqrt(21/8) * X * (5*Z*Z - 1) */
    coeffs[14] =  5.123475383f * z * (x*x - y*y);       /* ACN 14 = sqrt(105)/2 * Z * (X*X - Y*Y) */
    coeffs[15] =  2.091650066f * x * (x*x - 3.0f*y*y);  /* ACN 15 = sqrt(35/8) * X * (X*X - 3*Y*Y) */

    if(spread > 0.0f)
    {
        /* Implement the spread by using a spherical source that subtends the
         * angle spread. See:
         * http://www.ppsloan.org/publications/StupidSH36.pdf - Appendix A3
         *
         * When adjusted for N3D normalization instead of SN3D, these
         * calculations are:
         *
         * ZH0 = -sqrt(pi) * (-1+ca);
         * ZH1 =  0.5*sqrt(pi) * sa*sa;
         * ZH2 = -0.5*sqrt(pi) * ca*(-1+ca)*(ca+1);
         * ZH3 = -0.125*sqrt(pi) * (-1+ca)*(ca+1)*(5*ca*ca - 1);
         * ZH4 = -0.125*sqrt(pi) * ca*(-1+ca)*(ca+1)*(7*ca*ca - 3);
         * ZH5 = -0.0625*sqrt(pi) * (-1+ca)*(ca+1)*(21*ca*ca*ca*ca - 14*ca*ca + 1);
         *
         * The gain of the source is compensated for size, so that the
         * loundness doesn't depend on the spread. That is, the factors are
         * scaled so that ZH0 remains 1 regardless of the spread. Thus:
         *
         * ZH0 = 1.0f;
         * ZH1 = 0.5f * (ca+1.0f);
         * ZH2 = 0.5f * (ca+1.0f)*ca;
         * ZH3 = 0.125f * (ca+1.0f)*(5.0f*ca*ca - 1.0f);
         * ZH4 = 0.125f * (ca+1.0f)*(7.0f*ca*ca - 3.0f)*ca;
         * ZH5 = 0.0625f * (ca+1.0f)*(21.0f*ca*ca*ca*ca - 14.0f*ca*ca + 1.0f);
         */
        ALfloat ca = cosf(spread * 0.5f);

        ALfloat ZH0_norm = 1.0f;
        ALfloat ZH1_norm = 0.5f * (ca+1.f);
        ALfloat ZH2_norm = 0.5f * (ca+1.f)*ca;
        ALfloat ZH3_norm = 0.125f * (ca+1.f)*(5.f*ca*ca-1.f);

        /* Zeroth-order */
        coeffs[0]  *= ZH0_norm;
        /* First-order */
        coeffs[1]  *= ZH1_norm;
        coeffs[2]  *= ZH1_norm;
        coeffs[3]  *= ZH1_norm;
        /* Second-order */
        coeffs[4]  *= ZH2_norm;
        coeffs[5]  *= ZH2_norm;
        coeffs[6]  *= ZH2_norm;
        coeffs[7]  *= ZH2_norm;
        coeffs[8]  *= ZH2_norm;
        /* Third-order */
        coeffs[9]  *= ZH3_norm;
        coeffs[10] *= ZH3_norm;
        coeffs[11] *= ZH3_norm;
        coeffs[12] *= ZH3_norm;
        coeffs[13] *= ZH3_norm;
        coeffs[14] *= ZH3_norm;
        coeffs[15] *= ZH3_norm;
    }
}

void CalcAngleCoeffs(ALfloat azimuth, ALfloat elevation, ALfloat spread, ALfloat coeffs[MAX_AMBI_COEFFS])
{
    ALfloat dir[3] = {
        sinf(azimuth) * cosf(elevation),
        sinf(elevation),
        -cosf(azimuth) * cosf(elevation)
    };
    CalcDirectionCoeffs(dir, spread, coeffs);
}


void ComputeAmbientGainsMC(const ChannelConfig *chancoeffs, ALuint numchans, ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALuint i;

    for(i = 0;i < numchans;i++)
    {
        // The W coefficients are based on a mathematical average of the
        // output. The square root of the base average provides for a more
        // perceptual average volume, better suited to non-directional gains.
        gains[i] = sqrtf(chancoeffs[i][0]) * ingain;
    }
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputeAmbientGainsBF(const BFChannelConfig *chanmap, ALuint numchans, ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALfloat gain = 0.0f;
    ALuint i;

    for(i = 0;i < numchans;i++)
    {
        if(chanmap[i].Index == 0)
            gain += chanmap[i].Scale;
    }
    gains[0] = gain * 1.414213562f * ingain;
    for(i = 1;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputePanningGainsMC(const ChannelConfig *chancoeffs, ALuint numchans, ALuint numcoeffs, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALuint i, j;

    for(i = 0;i < numchans;i++)
    {
        float gain = 0.0f;
        for(j = 0;j < numcoeffs;j++)
            gain += chancoeffs[i][j]*coeffs[j];
        gains[i] = gain * ingain;
    }
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputePanningGainsBF(const BFChannelConfig *chanmap, ALuint numchans, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALuint i;

    for(i = 0;i < numchans;i++)
        gains[i] = chanmap[i].Scale * coeffs[chanmap[i].Index] * ingain;
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputeFirstOrderGainsMC(const ChannelConfig *chancoeffs, ALuint numchans, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALuint i, j;

    for(i = 0;i < numchans;i++)
    {
        float gain = 0.0f;
        for(j = 0;j < 4;j++)
            gain += chancoeffs[i][j] * mtx[j];
        gains[i] = gain * ingain;
    }
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputeFirstOrderGainsBF(const BFChannelConfig *chanmap, ALuint numchans, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALuint i;

    for(i = 0;i < numchans;i++)
        gains[i] = chanmap[i].Scale * mtx[chanmap[i].Index] * ingain;
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}


static inline const char *GetLabelFromChannel(enum Channel channel)
{
    switch(channel)
    {
        case FrontLeft: return "front-left";
        case FrontRight: return "front-right";
        case FrontCenter: return "front-center";
        case LFE: return "lfe";
        case BackLeft: return "back-left";
        case BackRight: return "back-right";
        case BackCenter: return "back-center";
        case SideLeft: return "side-left";
        case SideRight: return "side-right";

        case UpperFrontLeft: return "upper-front-left";
        case UpperFrontRight: return "upper-front-right";
        case UpperBackLeft: return "upper-back-left";
        case UpperBackRight: return "upper-back-right";
        case LowerFrontLeft: return "lower-front-left";
        case LowerFrontRight: return "lower-front-right";
        case LowerBackLeft: return "lower-back-left";
        case LowerBackRight: return "lower-back-right";

        case Aux0: return "aux-0";
        case Aux1: return "aux-1";
        case Aux2: return "aux-2";
        case Aux3: return "aux-3";
        case Aux4: return "aux-4";
        case Aux5: return "aux-5";
        case Aux6: return "aux-6";
        case Aux7: return "aux-7";
        case Aux8: return "aux-8";
        case Aux9: return "aux-9";
        case Aux10: return "aux-10";
        case Aux11: return "aux-11";
        case Aux12: return "aux-12";
        case Aux13: return "aux-13";
        case Aux14: return "aux-14";
        case Aux15: return "aux-15";

        case InvalidChannel: break;
    }
    return "(unknown)";
}


typedef struct ChannelMap {
    enum Channel ChanName;
    ChannelConfig Config;
} ChannelMap;

static void SetChannelMap(const enum Channel *devchans, ChannelConfig *ambicoeffs,
                          const ChannelMap *chanmap, size_t count, ALuint *outcount,
                          ALboolean isfuma)
{
    const ALuint *acnmap = isfuma ? FuMa2ACN : ACN2ACN;
    const ALfloat *n3dscale = isfuma ? FuMa2N3DScale : UnitScale;
    size_t j, k;
    ALuint i;

    for(i = 0;i < MAX_OUTPUT_CHANNELS && devchans[i] != InvalidChannel;i++)
    {
        if(devchans[i] == LFE)
        {
            for(j = 0;j < MAX_AMBI_COEFFS;j++)
                ambicoeffs[i][j] = 0.0f;
            continue;
        }

        for(j = 0;j < count;j++)
        {
            if(devchans[i] != chanmap[j].ChanName)
                continue;

            for(k = 0;k < MAX_AMBI_COEFFS;++k)
            {
                ALuint acn = acnmap[k];
                ambicoeffs[i][acn] = chanmap[j].Config[k] / n3dscale[acn];
            }
            break;
        }
        if(j == count)
            ERR("Failed to match %s channel (%u) in channel map\n", GetLabelFromChannel(devchans[i]), i);
    }
    *outcount = i;
}

static bool MakeSpeakerMap(ALCdevice *device, const AmbDecConf *conf, ALuint speakermap[MAX_OUTPUT_CHANNELS])
{
    ALuint i;

    for(i = 0;i < conf->NumSpeakers;i++)
    {
        int c = -1;

        /* NOTE: AmbDec does not define any standard speaker names, however
         * for this to work we have to by able to find the output channel
         * the speaker definition corresponds to. Therefore, OpenAL Soft
         * requires these channel labels to be recognized:
         *
         * LF = Front left
         * RF = Front right
         * LS = Side left
         * RS = Side right
         * LB = Back left
         * RB = Back right
         * CE = Front center
         * CB = Back center
         *
         * Additionally, surround51 will acknowledge back speakers for side
         * channels, and surround51rear will acknowledge side speakers for
         * back channels, to avoid issues with an ambdec expecting 5.1 to
         * use the side channels when the device is configured for back,
         * and vice-versa.
         */
        if(al_string_cmp_cstr(conf->Speakers[i].Name, "LF") == 0)
            c = GetChannelIdxByName(device->RealOut, FrontLeft);
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "RF") == 0)
            c = GetChannelIdxByName(device->RealOut, FrontRight);
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "CE") == 0)
            c = GetChannelIdxByName(device->RealOut, FrontCenter);
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "LS") == 0)
        {
            if(device->FmtChans == DevFmtX51Rear)
                c = GetChannelIdxByName(device->RealOut, BackLeft);
            else
                c = GetChannelIdxByName(device->RealOut, SideLeft);
        }
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "RS") == 0)
        {
            if(device->FmtChans == DevFmtX51Rear)
                c = GetChannelIdxByName(device->RealOut, BackRight);
            else
                c = GetChannelIdxByName(device->RealOut, SideRight);
        }
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "LB") == 0)
        {
            if(device->FmtChans == DevFmtX51)
                c = GetChannelIdxByName(device->RealOut, SideLeft);
            else
                c = GetChannelIdxByName(device->RealOut, BackLeft);
        }
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "RB") == 0)
        {
            if(device->FmtChans == DevFmtX51)
                c = GetChannelIdxByName(device->RealOut, SideRight);
            else
                c = GetChannelIdxByName(device->RealOut, BackRight);
        }
        else if(al_string_cmp_cstr(conf->Speakers[i].Name, "CB") == 0)
            c = GetChannelIdxByName(device->RealOut, BackCenter);
        else
        {
            const char *name = al_string_get_cstr(conf->Speakers[i].Name);
            unsigned int n;
            char ch;

            if(sscanf(name, "AUX%u%c", &n, &ch) == 1 && n < 16)
                c = GetChannelIdxByName(device->RealOut, Aux0+n);
            else
            {
                ERR("AmbDec speaker label \"%s\" not recognized\n", name);
                return false;
            }
        }
        if(c == -1)
        {
            ERR("Failed to lookup AmbDec speaker label %s\n",
                al_string_get_cstr(conf->Speakers[i].Name));
            return false;
        }
        speakermap[i] = c;
    }

    return true;
}


/* NOTE: These decoder coefficients are using FuMa channel ordering and
 * normalization, since that's what was produced by the Ambisonic Decoder
 * Toolbox. SetChannelMap will convert them to N3D.
 */
static const ChannelMap MonoCfg[1] = {
    { FrontCenter, { 1.414213562f } },
}, StereoCfg[2] = {
    { FrontLeft,   { 0.707106781f, 0.0f,  0.5f, 0.0f } },
    { FrontRight,  { 0.707106781f, 0.0f, -0.5f, 0.0f } },
}, QuadCfg[4] = {
    { FrontLeft,   { 0.353553f,  0.306186f,  0.306186f, 0.0f,  0.0f, 0.0f, 0.0f,  0.000000f,  0.125000f } },
    { FrontRight,  { 0.353553f,  0.306186f, -0.306186f, 0.0f,  0.0f, 0.0f, 0.0f,  0.000000f, -0.125000f } },
    { BackLeft,    { 0.353553f, -0.306186f,  0.306186f, 0.0f,  0.0f, 0.0f, 0.0f,  0.000000f, -0.125000f } },
    { BackRight,   { 0.353553f, -0.306186f, -0.306186f, 0.0f,  0.0f, 0.0f, 0.0f,  0.000000f,  0.125000f } },
}, X51SideCfg[5] = {
    { FrontLeft,   { 0.208954f,  0.199518f,  0.223424f, 0.0f,  0.0f, 0.0f, 0.0f, -0.012543f,  0.144260f } },
    { FrontRight,  { 0.208950f,  0.199514f, -0.223425f, 0.0f,  0.0f, 0.0f, 0.0f, -0.012544f, -0.144258f } },
    { FrontCenter, { 0.109403f,  0.168250f, -0.000002f, 0.0f,  0.0f, 0.0f, 0.0f,  0.100431f, -0.000001f } },
    { SideLeft,    { 0.470934f, -0.346484f,  0.327504f, 0.0f,  0.0f, 0.0f, 0.0f, -0.022188f, -0.041113f } },
    { SideRight,   { 0.470936f, -0.346480f, -0.327507f, 0.0f,  0.0f, 0.0f, 0.0f, -0.022186f,  0.041114f } },
}, X51RearCfg[5] = {
    { FrontLeft,   { 0.208954f,  0.199518f,  0.223424f, 0.0f,  0.0f, 0.0f, 0.0f, -0.012543f,  0.144260f } },
    { FrontRight,  { 0.208950f,  0.199514f, -0.223425f, 0.0f,  0.0f, 0.0f, 0.0f, -0.012544f, -0.144258f } },
    { FrontCenter, { 0.109403f,  0.168250f, -0.000002f, 0.0f,  0.0f, 0.0f, 0.0f,  0.100431f, -0.000001f } },
    { BackLeft,    { 0.470934f, -0.346484f,  0.327504f, 0.0f,  0.0f, 0.0f, 0.0f, -0.022188f, -0.041113f } },
    { BackRight,   { 0.470936f, -0.346480f, -0.327507f, 0.0f,  0.0f, 0.0f, 0.0f, -0.022186f,  0.041114f } },
}, X61Cfg[6] = {
    { FrontLeft,   { 0.167065f,  0.200583f,  0.172695f, 0.0f, 0.0f, 0.0f, 0.0f,  0.029855f,  0.186407f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.039241f,  0.068910f } },
    { FrontRight,  { 0.167065f,  0.200583f, -0.172695f, 0.0f, 0.0f, 0.0f, 0.0f,  0.029855f, -0.186407f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.039241f, -0.068910f } },
    { FrontCenter, { 0.109403f,  0.179490f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f,  0.142031f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.072024f,  0.000000f } },
    { BackCenter,  { 0.353556f, -0.461940f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f,  0.165723f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000f,  0.000000f } },
    { SideLeft,    { 0.289151f, -0.081301f,  0.401292f, 0.0f, 0.0f, 0.0f, 0.0f, -0.188208f, -0.071420f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.010099f, -0.032897f } },
    { SideRight,   { 0.289151f, -0.081301f, -0.401292f, 0.0f, 0.0f, 0.0f, 0.0f, -0.188208f,  0.071420f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.010099f,  0.032897f } },
}, X71Cfg[7] = {
    { FrontLeft,   { 0.167065f,  0.200583f,  0.172695f, 0.0f, 0.0f, 0.0f, 0.0f,  0.029855f,  0.186407f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.039241f,  0.068910f } },
    { FrontRight,  { 0.167065f,  0.200583f, -0.172695f, 0.0f, 0.0f, 0.0f, 0.0f,  0.029855f, -0.186407f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.039241f, -0.068910f } },
    { FrontCenter, { 0.109403f,  0.179490f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f,  0.142031f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.072024f,  0.000000f } },
    { BackLeft,    { 0.224752f, -0.295009f,  0.170325f, 0.0f, 0.0f, 0.0f, 0.0f,  0.105349f, -0.182473f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000f,  0.065799f } },
    { BackRight,   { 0.224752f, -0.295009f, -0.170325f, 0.0f, 0.0f, 0.0f, 0.0f,  0.105349f,  0.182473f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000f, -0.065799f } },
    { SideLeft,    { 0.224739f,  0.000000f,  0.340644f, 0.0f, 0.0f, 0.0f, 0.0f, -0.210697f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000f, -0.065795f } },
    { SideRight,   { 0.224739f,  0.000000f, -0.340644f, 0.0f, 0.0f, 0.0f, 0.0f, -0.210697f,  0.000000f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000f,  0.065795f } },
};

static void InitPanning(ALCdevice *device)
{
    const ChannelMap *chanmap = NULL;
    ALuint coeffcount = 0;
    ALfloat ambiscale;
    size_t count = 0;
    ALuint i, j;

    ambiscale = 1.0f;
    switch(device->FmtChans)
    {
        case DevFmtMono:
            count = COUNTOF(MonoCfg);
            chanmap = MonoCfg;
            ambiscale = ZERO_ORDER_SCALE;
            coeffcount = 1;
            break;

        case DevFmtStereo:
            count = COUNTOF(StereoCfg);
            chanmap = StereoCfg;
            ambiscale = FIRST_ORDER_SCALE;
            coeffcount = 4;
            break;

        case DevFmtQuad:
            count = COUNTOF(QuadCfg);
            chanmap = QuadCfg;
            ambiscale = SECOND_ORDER_SCALE;
            coeffcount = 9;
            break;

        case DevFmtX51:
            count = COUNTOF(X51SideCfg);
            chanmap = X51SideCfg;
            ambiscale = SECOND_ORDER_SCALE;
            coeffcount = 9;
            break;

        case DevFmtX51Rear:
            count = COUNTOF(X51RearCfg);
            chanmap = X51RearCfg;
            ambiscale = SECOND_ORDER_SCALE;
            coeffcount = 9;
            break;

        case DevFmtX61:
            count = COUNTOF(X61Cfg);
            chanmap = X61Cfg;
            ambiscale = THIRD_ORDER_SCALE;
            coeffcount = 16;
            break;

        case DevFmtX71:
            count = COUNTOF(X71Cfg);
            chanmap = X71Cfg;
            ambiscale = THIRD_ORDER_SCALE;
            coeffcount = 16;
            break;

        case DevFmtAmbi1:
        case DevFmtAmbi2:
        case DevFmtAmbi3:
            break;
    }

    if(device->FmtChans >= DevFmtAmbi1 && device->FmtChans <= DevFmtAmbi3)
    {
        const ALuint *acnmap = (device->AmbiFmt == AmbiFormat_FuMa) ? FuMa2ACN : ACN2ACN;
        const ALfloat *n3dscale = (device->AmbiFmt == AmbiFormat_FuMa) ? FuMa2N3DScale :
                                  (device->AmbiFmt == AmbiFormat_ACN_SN3D) ? SN3D2N3DScale :
                                  /*(device->AmbiFmt == AmbiFormat_ACN_N3D) ?*/ UnitScale;

        count = (device->FmtChans == DevFmtAmbi3) ? 16 :
                (device->FmtChans == DevFmtAmbi2) ? 9 :
                (device->FmtChans == DevFmtAmbi1) ? 4 : 1;
        for(i = 0;i < count;i++)
        {
            ALuint acn = acnmap[i];
            device->Dry.Ambi.Map[i].Scale = 1.0f/n3dscale[acn];
            device->Dry.Ambi.Map[i].Index = acn;
        }
        device->Dry.CoeffCount = 0;
        device->Dry.NumChannels = count;

        if(device->FmtChans == DevFmtAmbi1)
        {
            device->FOAOut.Ambi = device->Dry.Ambi;
            device->FOAOut.CoeffCount = device->Dry.CoeffCount;
        }
        else
        {
            /* FOA output is always ACN+N3D for higher-order ambisonic output.
             * The upsampler expects this and will convert it for output.
             */
            memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
            for(i = 0;i < 4;i++)
            {
                device->FOAOut.Ambi.Map[i].Scale = 1.0f;
                device->FOAOut.Ambi.Map[i].Index = i;
            }
            device->FOAOut.CoeffCount = 0;

            ambiup_reset(device->AmbiUp, device);
        }
    }
    else
    {
        SetChannelMap(device->RealOut.ChannelName, device->Dry.Ambi.Coeffs,
                      chanmap, count, &device->Dry.NumChannels, AL_TRUE);
        device->Dry.CoeffCount = coeffcount;

        memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
        for(i = 0;i < device->Dry.NumChannels;i++)
        {
            device->FOAOut.Ambi.Coeffs[i][0] = device->Dry.Ambi.Coeffs[i][0];
            for(j = 1;j < 4;j++)
                device->FOAOut.Ambi.Coeffs[i][j] = device->Dry.Ambi.Coeffs[i][j] * ambiscale;
        }
        device->FOAOut.CoeffCount = 4;
    }
}

static void InitCustomPanning(ALCdevice *device, const AmbDecConf *conf, const ALuint speakermap[MAX_OUTPUT_CHANNELS])
{
    ChannelMap chanmap[MAX_OUTPUT_CHANNELS];
    const ALfloat *coeff_scale = UnitScale;
    ALfloat ambiscale = 1.0f;
    ALuint i, j;

    if(conf->FreqBands != 1)
        ERR("Basic renderer uses the high-frequency matrix as single-band (xover_freq = %.0fhz)\n",
            conf->XOverFreq);

    if(conf->ChanMask > 0x1ff)
        ambiscale = THIRD_ORDER_SCALE;
    else if(conf->ChanMask > 0xf)
        ambiscale = SECOND_ORDER_SCALE;
    else if(conf->ChanMask > 0x1)
        ambiscale = FIRST_ORDER_SCALE;
    else
        ambiscale = 0.0f;

    if(conf->CoeffScale == ADS_SN3D)
        coeff_scale = SN3D2N3DScale;
    else if(conf->CoeffScale == ADS_FuMa)
        coeff_scale = FuMa2N3DScale;

    for(i = 0;i < conf->NumSpeakers;i++)
    {
        ALuint chan = speakermap[i];
        ALfloat gain;
        ALuint k = 0;

        for(j = 0;j < MAX_AMBI_COEFFS;j++)
            chanmap[i].Config[j] = 0.0f;

        chanmap[i].ChanName = device->RealOut.ChannelName[chan];
        for(j = 0;j < MAX_AMBI_COEFFS;j++)
        {
            if(j == 0) gain = conf->HFOrderGain[0];
            else if(j == 1) gain = conf->HFOrderGain[1];
            else if(j == 4) gain = conf->HFOrderGain[2];
            else if(j == 9) gain = conf->HFOrderGain[3];
            if((conf->ChanMask&(1<<j)))
                chanmap[i].Config[j] = conf->HFMatrix[i][k++] / coeff_scale[j] * gain;
        }
    }

    SetChannelMap(device->RealOut.ChannelName, device->Dry.Ambi.Coeffs, chanmap,
                  conf->NumSpeakers, &device->Dry.NumChannels, AL_FALSE);
    device->Dry.CoeffCount = (conf->ChanMask > 0x1ff) ? 16 :
                             (conf->ChanMask > 0xf) ? 9 : 4;

    memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
    for(i = 0;i < device->Dry.NumChannels;i++)
    {
        device->FOAOut.Ambi.Coeffs[i][0] = device->Dry.Ambi.Coeffs[i][0];
        for(j = 1;j < 4;j++)
            device->FOAOut.Ambi.Coeffs[i][j] = device->Dry.Ambi.Coeffs[i][j] * ambiscale;
    }
    device->FOAOut.CoeffCount = 4;
}

static void InitHQPanning(ALCdevice *device, const AmbDecConf *conf, const ALuint speakermap[MAX_OUTPUT_CHANNELS])
{
    const char *devname;
    int decflags = 0;
    size_t count;
    ALuint i;

    devname = al_string_get_cstr(device->DeviceName);
    if(GetConfigValueBool(devname, "decoder", "distance-comp", 1))
        decflags |= BFDF_DistanceComp;

    if((conf->ChanMask&AMBI_PERIPHONIC_MASK))
    {
        count = (conf->ChanMask > 0x1ff) ? 16 :
                (conf->ChanMask > 0xf) ? 9 : 4;
        for(i = 0;i < count;i++)
        {
            device->Dry.Ambi.Map[i].Scale = 1.0f;
            device->Dry.Ambi.Map[i].Index = i;
        }
    }
    else
    {
        static const int map[MAX_AMBI2D_COEFFS] = { 0, 1, 3, 4, 8, 9, 15 };

        count = (conf->ChanMask > 0x1ff) ? 7 :
                (conf->ChanMask > 0xf) ? 5 : 3;
        for(i = 0;i < count;i++)
        {
            device->Dry.Ambi.Map[i].Scale = 1.0f;
            device->Dry.Ambi.Map[i].Index = map[i];
        }
    }
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = count;

    TRACE("Enabling %s-band %s-order%s ambisonic decoder\n",
        (conf->FreqBands == 1) ? "single" : "dual",
        (conf->ChanMask > 0xf) ? (conf->ChanMask > 0x1ff) ? "third" : "second" : "first",
        (conf->ChanMask&AMBI_PERIPHONIC_MASK) ? " periphonic" : ""
    );
    bformatdec_reset(device->AmbiDecoder, conf, count, device->Frequency,
                     speakermap, decflags);

    if(bformatdec_getOrder(device->AmbiDecoder) < 2)
    {
        device->FOAOut.Ambi = device->Dry.Ambi;
        device->FOAOut.CoeffCount = device->Dry.CoeffCount;
    }
    else
    {
        memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
        for(i = 0;i < 4;i++)
        {
            device->FOAOut.Ambi.Map[i].Scale = 1.0f;
            device->FOAOut.Ambi.Map[i].Index = i;
        }
        device->FOAOut.CoeffCount = 0;
    }
}

static void InitHrtfPanning(ALCdevice *device)
{
    size_t count = 4;
    ALuint i;

    for(i = 0;i < count;i++)
    {
        device->Dry.Ambi.Map[i].Scale = 1.0f;
        device->Dry.Ambi.Map[i].Index = i;
    }
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = count;

    device->FOAOut.Ambi = device->Dry.Ambi;
    device->FOAOut.CoeffCount = device->Dry.CoeffCount;

    memset(device->Hrtf.Coeffs, 0, sizeof(device->Hrtf.Coeffs));
    device->Hrtf.IrSize = BuildBFormatHrtf(device->Hrtf.Handle,
        device->Hrtf.Coeffs, device->Dry.NumChannels
    );

    /* Round up to the nearest multiple of 8 */
    device->Hrtf.IrSize = (device->Hrtf.IrSize+7)&~7;
}

static void InitUhjPanning(ALCdevice *device)
{
    size_t count = 3;
    ALuint i;

    for(i = 0;i < count;i++)
    {
        ALuint acn = FuMa2ACN[i];
        device->Dry.Ambi.Map[i].Scale = 1.0f/FuMa2N3DScale[acn];
        device->Dry.Ambi.Map[i].Index = acn;
    }
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = count;

    device->FOAOut.Ambi = device->Dry.Ambi;
    device->FOAOut.CoeffCount = device->Dry.CoeffCount;
}

void aluInitRenderer(ALCdevice *device, ALint hrtf_id, enum HrtfRequestMode hrtf_appreq, enum HrtfRequestMode hrtf_userreq)
{
    const char *mode;
    bool headphones;
    int bs2blevel;
    size_t i;

    device->Hrtf.Handle = NULL;
    al_string_clear(&device->Hrtf.Name);
    device->Render_Mode = NormalRender;

    memset(&device->Dry.Ambi, 0, sizeof(device->Dry.Ambi));
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = 0;

    if(device->FmtChans != DevFmtStereo)
    {
        ALuint speakermap[MAX_OUTPUT_CHANNELS];
        const char *devname, *layout = NULL;
        AmbDecConf conf, *pconf = NULL;

        if(hrtf_appreq == Hrtf_Enable)
            device->Hrtf.Status = ALC_HRTF_UNSUPPORTED_FORMAT_SOFT;

        ambdec_init(&conf);

        devname = al_string_get_cstr(device->DeviceName);
        switch(device->FmtChans)
        {
            case DevFmtQuad: layout = "quad"; break;
            case DevFmtX51: layout = "surround51"; break;
            case DevFmtX51Rear: layout = "surround51rear"; break;
            case DevFmtX61: layout = "surround61"; break;
            case DevFmtX71: layout = "surround71"; break;
            /* Mono, Stereo, and Ambisonics output don't use custom decoders. */
            case DevFmtMono:
            case DevFmtStereo:
            case DevFmtAmbi1:
            case DevFmtAmbi2:
            case DevFmtAmbi3:
                break;
        }
        if(layout)
        {
            const char *fname;
            if(ConfigValueStr(devname, "decoder", layout, &fname))
            {
                if(!ambdec_load(&conf, fname))
                    ERR("Failed to load layout file %s\n", fname);
                else
                {
                    if(conf.ChanMask > 0xffff)
                        ERR("Unsupported channel mask 0x%04x (max 0xffff)\n", conf.ChanMask);
                    else
                    {
                        if(MakeSpeakerMap(device, &conf, speakermap))
                            pconf = &conf;
                    }
                }
            }
        }

        if(pconf && GetConfigValueBool(devname, "decoder", "hq-mode", 0))
        {
            ambiup_free(device->AmbiUp);
            device->AmbiUp = NULL;
            if(!device->AmbiDecoder)
                device->AmbiDecoder = bformatdec_alloc();
        }
        else
        {
            bformatdec_free(device->AmbiDecoder);
            device->AmbiDecoder = NULL;
            if(device->FmtChans > DevFmtAmbi1 && device->FmtChans <= DevFmtAmbi3)
            {
                if(!device->AmbiUp)
                    device->AmbiUp = ambiup_alloc();
            }
            else
            {
                ambiup_free(device->AmbiUp);
                device->AmbiUp = NULL;
            }
        }

        if(!pconf)
            InitPanning(device);
        else if(device->AmbiDecoder)
            InitHQPanning(device, pconf, speakermap);
        else
            InitCustomPanning(device, pconf, speakermap);

        ambdec_deinit(&conf);
        return;
    }

    ambiup_free(device->AmbiUp);
    device->AmbiUp = NULL;
    bformatdec_free(device->AmbiDecoder);
    device->AmbiDecoder = NULL;

    headphones = device->IsHeadphones;
    if(device->Type != Loopback)
    {
        const char *mode;
        if(ConfigValueStr(al_string_get_cstr(device->DeviceName), NULL, "stereo-mode", &mode))
        {
            if(strcasecmp(mode, "headphones") == 0)
                headphones = true;
            else if(strcasecmp(mode, "speakers") == 0)
                headphones = false;
            else if(strcasecmp(mode, "auto") != 0)
                ERR("Unexpected stereo-mode: %s\n", mode);
        }
    }

    if(hrtf_userreq == Hrtf_Default)
    {
        bool usehrtf = (headphones && hrtf_appreq != Hrtf_Disable) ||
                       (hrtf_appreq == Hrtf_Enable);
        if(!usehrtf) goto no_hrtf;

        device->Hrtf.Status = ALC_HRTF_ENABLED_SOFT;
        if(headphones && hrtf_appreq != Hrtf_Disable)
            device->Hrtf.Status = ALC_HRTF_HEADPHONES_DETECTED_SOFT;
    }
    else
    {
        if(hrtf_userreq != Hrtf_Enable)
        {
            if(hrtf_appreq == Hrtf_Enable)
                device->Hrtf.Status = ALC_HRTF_DENIED_SOFT;
            goto no_hrtf;
        }
        device->Hrtf.Status = ALC_HRTF_REQUIRED_SOFT;
    }

    if(VECTOR_SIZE(device->Hrtf.List) == 0)
    {
        VECTOR_DEINIT(device->Hrtf.List);
        device->Hrtf.List = EnumerateHrtf(device->DeviceName);
    }

    if(hrtf_id >= 0 && (size_t)hrtf_id < VECTOR_SIZE(device->Hrtf.List))
    {
        const HrtfEntry *entry = &VECTOR_ELEM(device->Hrtf.List, hrtf_id);
        if(entry->hrtf->sampleRate == device->Frequency)
        {
            device->Hrtf.Handle = entry->hrtf;
            al_string_copy(&device->Hrtf.Name, entry->name);
        }
    }

    for(i = 0;!device->Hrtf.Handle && i < VECTOR_SIZE(device->Hrtf.List);i++)
    {
        const HrtfEntry *entry = &VECTOR_ELEM(device->Hrtf.List, i);
        if(entry->hrtf->sampleRate == device->Frequency)
        {
            device->Hrtf.Handle = entry->hrtf;
            al_string_copy(&device->Hrtf.Name, entry->name);
        }
    }

    if(device->Hrtf.Handle)
    {
        device->Render_Mode = HrtfRender;
        if(ConfigValueStr(al_string_get_cstr(device->DeviceName), NULL, "hrtf-mode", &mode))
        {
            if(strcasecmp(mode, "full") == 0)
                device->Render_Mode = HrtfRender;
            else if(strcasecmp(mode, "basic") == 0)
                device->Render_Mode = NormalRender;
            else
                ERR("Unexpected hrtf-mode: %s\n", mode);
        }

        TRACE("HRTF enabled, \"%s\"\n", al_string_get_cstr(device->Hrtf.Name));
        InitHrtfPanning(device);
        return;
    }
    device->Hrtf.Status = ALC_HRTF_UNSUPPORTED_FORMAT_SOFT;

no_hrtf:
    TRACE("HRTF disabled\n");

    bs2blevel = ((headphones && hrtf_appreq != Hrtf_Disable) ||
                 (hrtf_appreq == Hrtf_Enable)) ? 5 : 0;
    if(device->Type != Loopback)
        ConfigValueInt(al_string_get_cstr(device->DeviceName), NULL, "cf_level", &bs2blevel);
    if(bs2blevel > 0 && bs2blevel <= 6)
    {
        device->Bs2b = al_calloc(16, sizeof(*device->Bs2b));
        bs2b_set_params(device->Bs2b, bs2blevel, device->Frequency);
        device->Render_Mode = StereoPair;
        TRACE("BS2B enabled\n");
        InitPanning(device);
        return;
    }

    TRACE("BS2B disabled\n");

    device->Render_Mode = NormalRender;
    if(ConfigValueStr(al_string_get_cstr(device->DeviceName), NULL, "stereo-panning", &mode))
    {
        if(strcasecmp(mode, "paired") == 0)
            device->Render_Mode = StereoPair;
        else if(strcasecmp(mode, "uhj") != 0)
            ERR("Unexpected stereo-panning: %s\n", mode);
    }
    if(device->Render_Mode == NormalRender)
    {
        device->Uhj_Encoder = al_calloc(16, sizeof(Uhj2Encoder));
        TRACE("UHJ enabled\n");
        InitUhjPanning(device);
        return;
    }

    TRACE("UHJ disabled\n");
    InitPanning(device);
}


void aluInitEffectPanning(ALeffectslot *slot)
{
    ALuint i;

    memset(slot->ChanMap, 0, sizeof(slot->ChanMap));
    slot->NumChannels = 0;

    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
    {
        slot->ChanMap[i].Scale = 1.0f;
        slot->ChanMap[i].Index = i;
    }
    slot->NumChannels = i;
}
