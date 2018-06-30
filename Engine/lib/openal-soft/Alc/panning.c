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
#include "alconfig.h"
#include "bool.h"
#include "ambdec.h"
#include "bformatdec.h"
#include "filters/splitter.h"
#include "uhjfilter.h"
#include "bs2b.h"


extern inline void CalcAngleCoeffs(ALfloat azimuth, ALfloat elevation, ALfloat spread, ALfloat coeffs[MAX_AMBI_COEFFS]);
extern inline void ComputeDryPanGains(const DryMixParams *dry, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS]);
extern inline void ComputeFirstOrderGains(const BFMixParams *foa, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS]);


static const ALsizei FuMa2ACN[MAX_AMBI_COEFFS] = {
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
static const ALsizei ACN2ACN[MAX_AMBI_COEFFS] = {
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9, 10, 11, 12, 13, 14, 15
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
         * loundness doesn't depend on the spread. Thus:
         *
         * ZH0 = 1.0f;
         * ZH1 = 0.5f * (ca+1.0f);
         * ZH2 = 0.5f * (ca+1.0f)*ca;
         * ZH3 = 0.125f * (ca+1.0f)*(5.0f*ca*ca - 1.0f);
         * ZH4 = 0.125f * (ca+1.0f)*(7.0f*ca*ca - 3.0f)*ca;
         * ZH5 = 0.0625f * (ca+1.0f)*(21.0f*ca*ca*ca*ca - 14.0f*ca*ca + 1.0f);
         */
        ALfloat ca = cosf(spread * 0.5f);
        /* Increase the source volume by up to +3dB for a full spread. */
        ALfloat scale = sqrtf(1.0f + spread/F_TAU);

        ALfloat ZH0_norm = scale;
        ALfloat ZH1_norm = 0.5f * (ca+1.f) * scale;
        ALfloat ZH2_norm = 0.5f * (ca+1.f)*ca * scale;
        ALfloat ZH3_norm = 0.125f * (ca+1.f)*(5.f*ca*ca-1.f) * scale;

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

void CalcAnglePairwiseCoeffs(ALfloat azimuth, ALfloat elevation, ALfloat spread, ALfloat coeffs[MAX_AMBI_COEFFS])
{
    ALfloat sign = (azimuth < 0.0f) ? -1.0f : 1.0f;
    if(!(fabsf(azimuth) > F_PI_2))
        azimuth = minf(fabsf(azimuth) * F_PI_2 / (F_PI/6.0f), F_PI_2) * sign;
    CalcAngleCoeffs(azimuth, elevation, spread, coeffs);
}


void ComputePanningGainsMC(const ChannelConfig *chancoeffs, ALsizei numchans, ALsizei numcoeffs, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALsizei i, j;

    for(i = 0;i < numchans;i++)
    {
        float gain = 0.0f;
        for(j = 0;j < numcoeffs;j++)
            gain += chancoeffs[i][j]*coeffs[j];
        gains[i] = clampf(gain, 0.0f, 1.0f) * ingain;
    }
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputePanningGainsBF(const BFChannelConfig *chanmap, ALsizei numchans, const ALfloat coeffs[MAX_AMBI_COEFFS], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALsizei i;

    for(i = 0;i < numchans;i++)
        gains[i] = chanmap[i].Scale * coeffs[chanmap[i].Index] * ingain;
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputeFirstOrderGainsMC(const ChannelConfig *chancoeffs, ALsizei numchans, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALsizei i, j;

    for(i = 0;i < numchans;i++)
    {
        float gain = 0.0f;
        for(j = 0;j < 4;j++)
            gain += chancoeffs[i][j] * mtx[j];
        gains[i] = clampf(gain, 0.0f, 1.0f) * ingain;
    }
    for(;i < MAX_OUTPUT_CHANNELS;i++)
        gains[i] = 0.0f;
}

void ComputeFirstOrderGainsBF(const BFChannelConfig *chanmap, ALsizei numchans, const ALfloat mtx[4], ALfloat ingain, ALfloat gains[MAX_OUTPUT_CHANNELS])
{
    ALsizei i;

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

static void SetChannelMap(const enum Channel devchans[MAX_OUTPUT_CHANNELS],
                          ChannelConfig *ambicoeffs, const ChannelMap *chanmap,
                          ALsizei count, ALsizei *outcount)
{
    ALsizei maxchans = 0;
    ALsizei i, j;

    for(i = 0;i < count;i++)
    {
        ALint idx = GetChannelIndex(devchans, chanmap[i].ChanName);
        if(idx < 0)
        {
            ERR("Failed to find %s channel in device\n",
                GetLabelFromChannel(chanmap[i].ChanName));
            continue;
        }

        maxchans = maxi(maxchans, idx+1);
        for(j = 0;j < MAX_AMBI_COEFFS;j++)
            ambicoeffs[idx][j] = chanmap[i].Config[j];
    }
    *outcount = mini(maxchans, MAX_OUTPUT_CHANNELS);
}

static bool MakeSpeakerMap(ALCdevice *device, const AmbDecConf *conf, ALsizei speakermap[MAX_OUTPUT_CHANNELS])
{
    ALsizei i;

    for(i = 0;i < conf->NumSpeakers;i++)
    {
        enum Channel ch;
        int chidx = -1;

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
        if(alstr_cmp_cstr(conf->Speakers[i].Name, "LF") == 0)
            ch = FrontLeft;
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "RF") == 0)
            ch = FrontRight;
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "CE") == 0)
            ch = FrontCenter;
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "LS") == 0)
        {
            if(device->FmtChans == DevFmtX51Rear)
                ch = BackLeft;
            else
                ch = SideLeft;
        }
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "RS") == 0)
        {
            if(device->FmtChans == DevFmtX51Rear)
                ch = BackRight;
            else
                ch = SideRight;
        }
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "LB") == 0)
        {
            if(device->FmtChans == DevFmtX51)
                ch = SideLeft;
            else
                ch = BackLeft;
        }
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "RB") == 0)
        {
            if(device->FmtChans == DevFmtX51)
                ch = SideRight;
            else
                ch = BackRight;
        }
        else if(alstr_cmp_cstr(conf->Speakers[i].Name, "CB") == 0)
            ch = BackCenter;
        else
        {
            const char *name = alstr_get_cstr(conf->Speakers[i].Name);
            unsigned int n;
            char c;

            if(sscanf(name, "AUX%u%c", &n, &c) == 1 && n < 16)
                ch = Aux0+n;
            else
            {
                ERR("AmbDec speaker label \"%s\" not recognized\n", name);
                return false;
            }
        }
        chidx = GetChannelIdxByName(&device->RealOut, ch);
        if(chidx == -1)
        {
            ERR("Failed to lookup AmbDec speaker label %s\n",
                alstr_get_cstr(conf->Speakers[i].Name));
            return false;
        }
        speakermap[i] = chidx;
    }

    return true;
}


static const ChannelMap MonoCfg[1] = {
    { FrontCenter, { 1.0f } },
}, StereoCfg[2] = {
    { FrontLeft,   { 5.00000000e-1f,  2.88675135e-1f, 0.0f,  5.52305643e-2f } },
    { FrontRight,  { 5.00000000e-1f, -2.88675135e-1f, 0.0f,  5.52305643e-2f } },
}, QuadCfg[4] = {
    { BackLeft,    { 3.53553391e-1f,  2.04124145e-1f, 0.0f, -2.04124145e-1f } },
    { FrontLeft,   { 3.53553391e-1f,  2.04124145e-1f, 0.0f,  2.04124145e-1f } },
    { FrontRight,  { 3.53553391e-1f, -2.04124145e-1f, 0.0f,  2.04124145e-1f } },
    { BackRight,   { 3.53553391e-1f, -2.04124145e-1f, 0.0f, -2.04124145e-1f } },
}, X51SideCfg[4] = {
    { SideLeft,    { 3.33000782e-1f,  1.89084803e-1f, 0.0f, -2.00042375e-1f, -2.12307769e-2f, 0.0f, 0.0f, 0.0f, -1.14579885e-2f } },
    { FrontLeft,   { 1.88542860e-1f,  1.27709292e-1f, 0.0f,  1.66295695e-1f,  7.30571517e-2f, 0.0f, 0.0f, 0.0f,  2.10901184e-2f } },
    { FrontRight,  { 1.88542860e-1f, -1.27709292e-1f, 0.0f,  1.66295695e-1f, -7.30571517e-2f, 0.0f, 0.0f, 0.0f,  2.10901184e-2f } },
    { SideRight,   { 3.33000782e-1f, -1.89084803e-1f, 0.0f, -2.00042375e-1f,  2.12307769e-2f, 0.0f, 0.0f, 0.0f, -1.14579885e-2f } },
}, X51RearCfg[4] = {
    { BackLeft,    { 3.33000782e-1f,  1.89084803e-1f, 0.0f, -2.00042375e-1f, -2.12307769e-2f, 0.0f, 0.0f, 0.0f, -1.14579885e-2f } },
    { FrontLeft,   { 1.88542860e-1f,  1.27709292e-1f, 0.0f,  1.66295695e-1f,  7.30571517e-2f, 0.0f, 0.0f, 0.0f,  2.10901184e-2f } },
    { FrontRight,  { 1.88542860e-1f, -1.27709292e-1f, 0.0f,  1.66295695e-1f, -7.30571517e-2f, 0.0f, 0.0f, 0.0f,  2.10901184e-2f } },
    { BackRight,   { 3.33000782e-1f, -1.89084803e-1f, 0.0f, -2.00042375e-1f,  2.12307769e-2f, 0.0f, 0.0f, 0.0f, -1.14579885e-2f } },
}, X61Cfg[6] = {
    { SideLeft,    { 2.04460341e-1f,  2.17177926e-1f, 0.0f, -4.39996780e-2f, -2.60790269e-2f, 0.0f, 0.0f, 0.0f, -6.87239792e-2f } },
    { FrontLeft,   { 1.58923161e-1f,  9.21772680e-2f, 0.0f,  1.59658796e-1f,  6.66278083e-2f, 0.0f, 0.0f, 0.0f,  3.84686854e-2f } },
    { FrontRight,  { 1.58923161e-1f, -9.21772680e-2f, 0.0f,  1.59658796e-1f, -6.66278083e-2f, 0.0f, 0.0f, 0.0f,  3.84686854e-2f } },
    { SideRight,   { 2.04460341e-1f, -2.17177926e-1f, 0.0f, -4.39996780e-2f,  2.60790269e-2f, 0.0f, 0.0f, 0.0f, -6.87239792e-2f } },
    { BackCenter,  { 2.50001688e-1f,  0.00000000e+0f, 0.0f, -2.50000094e-1f,  0.00000000e+0f, 0.0f, 0.0f, 0.0f,  6.05133395e-2f } },
}, X71Cfg[6] = {
    { BackLeft,    { 2.04124145e-1f,  1.08880247e-1f, 0.0f, -1.88586120e-1f, -1.29099444e-1f, 0.0f, 0.0f, 0.0f,  7.45355993e-2f,  3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
    { SideLeft,    { 2.04124145e-1f,  2.17760495e-1f, 0.0f,  0.00000000e+0f,  0.00000000e+0f, 0.0f, 0.0f, 0.0f, -1.49071198e-1f, -3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
    { FrontLeft,   { 2.04124145e-1f,  1.08880247e-1f, 0.0f,  1.88586120e-1f,  1.29099444e-1f, 0.0f, 0.0f, 0.0f,  7.45355993e-2f,  3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
    { FrontRight,  { 2.04124145e-1f, -1.08880247e-1f, 0.0f,  1.88586120e-1f, -1.29099444e-1f, 0.0f, 0.0f, 0.0f,  7.45355993e-2f, -3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
    { SideRight,   { 2.04124145e-1f, -2.17760495e-1f, 0.0f,  0.00000000e+0f,  0.00000000e+0f, 0.0f, 0.0f, 0.0f, -1.49071198e-1f,  3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
    { BackRight,   { 2.04124145e-1f, -1.08880247e-1f, 0.0f, -1.88586120e-1f,  1.29099444e-1f, 0.0f, 0.0f, 0.0f,  7.45355993e-2f, -3.73460789e-2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.00000000e+0f } },
};

static void InitNearFieldCtrl(ALCdevice *device, ALfloat ctrl_dist, ALsizei order,
                              const ALsizei *restrict chans_per_order)
{
    const char *devname = alstr_get_cstr(device->DeviceName);
    ALsizei i;

    if(GetConfigValueBool(devname, "decoder", "nfc", 1) && ctrl_dist > 0.0f)
    {
        /* NFC is only used when AvgSpeakerDist is greater than 0, and can only
         * be used when rendering to an ambisonic buffer.
         */
        device->AvgSpeakerDist = minf(ctrl_dist, 10.0f);

        for(i = 0;i < order+1;i++)
            device->Dry.NumChannelsPerOrder[i] = chans_per_order[i];
        for(;i < MAX_AMBI_ORDER+1;i++)
            device->Dry.NumChannelsPerOrder[i] = 0;
    }
}

static void InitDistanceComp(ALCdevice *device, const AmbDecConf *conf, const ALsizei speakermap[MAX_OUTPUT_CHANNELS])
{
    const char *devname = alstr_get_cstr(device->DeviceName);
    ALfloat maxdist = 0.0f;
    size_t total = 0;
    ALsizei i;

    for(i = 0;i < conf->NumSpeakers;i++)
        maxdist = maxf(maxdist, conf->Speakers[i].Distance);

    if(GetConfigValueBool(devname, "decoder", "distance-comp", 1) && maxdist > 0.0f)
    {
        ALfloat srate = (ALfloat)device->Frequency;
        for(i = 0;i < conf->NumSpeakers;i++)
        {
            ALsizei chan = speakermap[i];
            ALfloat delay;

            /* Distance compensation only delays in steps of the sample rate.
             * This is a bit less accurate since the delay time falls to the
             * nearest sample time, but it's far simpler as it doesn't have to
             * deal with phase offsets. This means at 48khz, for instance, the
             * distance delay will be in steps of about 7 millimeters.
             */
            delay = floorf((maxdist-conf->Speakers[i].Distance) / SPEEDOFSOUNDMETRESPERSEC *
                           srate + 0.5f);
            if(delay >= (ALfloat)MAX_DELAY_LENGTH)
                ERR("Delay for speaker \"%s\" exceeds buffer length (%f >= %u)\n",
                    alstr_get_cstr(conf->Speakers[i].Name), delay, MAX_DELAY_LENGTH);

            device->ChannelDelay[chan].Length = (ALsizei)clampf(
                delay, 0.0f, (ALfloat)(MAX_DELAY_LENGTH-1)
            );
            device->ChannelDelay[chan].Gain = conf->Speakers[i].Distance / maxdist;
            TRACE("Channel %u \"%s\" distance compensation: %d samples, %f gain\n", chan,
                  alstr_get_cstr(conf->Speakers[i].Name), device->ChannelDelay[chan].Length,
                device->ChannelDelay[chan].Gain
            );

            /* Round up to the next 4th sample, so each channel buffer starts
             * 16-byte aligned.
             */
            total += RoundUp(device->ChannelDelay[chan].Length, 4);
        }
    }

    if(total > 0)
    {
        device->ChannelDelay[0].Buffer = al_calloc(16, total * sizeof(ALfloat));
        for(i = 1;i < MAX_OUTPUT_CHANNELS;i++)
        {
            size_t len = RoundUp(device->ChannelDelay[i-1].Length, 4);
            device->ChannelDelay[i].Buffer = device->ChannelDelay[i-1].Buffer + len;
        }
    }
}

static void InitPanning(ALCdevice *device)
{
    const ChannelMap *chanmap = NULL;
    ALsizei coeffcount = 0;
    ALsizei count = 0;
    ALsizei i, j;

    switch(device->FmtChans)
    {
        case DevFmtMono:
            count = COUNTOF(MonoCfg);
            chanmap = MonoCfg;
            coeffcount = 1;
            break;

        case DevFmtStereo:
            count = COUNTOF(StereoCfg);
            chanmap = StereoCfg;
            coeffcount = 4;
            break;

        case DevFmtQuad:
            count = COUNTOF(QuadCfg);
            chanmap = QuadCfg;
            coeffcount = 4;
            break;

        case DevFmtX51:
            count = COUNTOF(X51SideCfg);
            chanmap = X51SideCfg;
            coeffcount = 9;
            break;

        case DevFmtX51Rear:
            count = COUNTOF(X51RearCfg);
            chanmap = X51RearCfg;
            coeffcount = 9;
            break;

        case DevFmtX61:
            count = COUNTOF(X61Cfg);
            chanmap = X61Cfg;
            coeffcount = 9;
            break;

        case DevFmtX71:
            count = COUNTOF(X71Cfg);
            chanmap = X71Cfg;
            coeffcount = 16;
            break;

        case DevFmtAmbi3D:
            break;
    }

    if(device->FmtChans == DevFmtAmbi3D)
    {
        const char *devname = alstr_get_cstr(device->DeviceName);
        const ALsizei *acnmap = (device->AmbiLayout == AmbiLayout_FuMa) ? FuMa2ACN : ACN2ACN;
        const ALfloat *n3dscale = (device->AmbiScale == AmbiNorm_FuMa) ? FuMa2N3DScale :
                                  (device->AmbiScale == AmbiNorm_SN3D) ? SN3D2N3DScale :
                                  /*(device->AmbiScale == AmbiNorm_N3D) ?*/ N3D2N3DScale;
        ALfloat nfc_delay = 0.0f;

        count = (device->AmbiOrder == 3) ? 16 :
                (device->AmbiOrder == 2) ? 9 :
                (device->AmbiOrder == 1) ? 4 : 1;
        for(i = 0;i < count;i++)
        {
            ALsizei acn = acnmap[i];
            device->Dry.Ambi.Map[i].Scale = 1.0f/n3dscale[acn];
            device->Dry.Ambi.Map[i].Index = acn;
        }
        device->Dry.CoeffCount = 0;
        device->Dry.NumChannels = count;

        if(device->AmbiOrder < 2)
        {
            device->FOAOut.Ambi = device->Dry.Ambi;
            device->FOAOut.CoeffCount = device->Dry.CoeffCount;
            device->FOAOut.NumChannels = 0;
        }
        else
        {
            ALfloat w_scale=1.0f, xyz_scale=1.0f;

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
            device->FOAOut.NumChannels = 4;

            if(device->AmbiOrder >= 3)
            {
                w_scale = W_SCALE_3H3P;
                xyz_scale = XYZ_SCALE_3H3P;
            }
            else
            {
                w_scale = W_SCALE_2H2P;
                xyz_scale = XYZ_SCALE_2H2P;
            }
            ambiup_reset(device->AmbiUp, device, w_scale, xyz_scale);
        }

        if(ConfigValueFloat(devname, "decoder", "nfc-ref-delay", &nfc_delay) && nfc_delay > 0.0f)
        {
            static const ALsizei chans_per_order[MAX_AMBI_ORDER+1] = {
                1, 3, 5, 7
            };
            nfc_delay = clampf(nfc_delay, 0.001f, 1000.0f);
            InitNearFieldCtrl(device, nfc_delay * SPEEDOFSOUNDMETRESPERSEC,
                              device->AmbiOrder, chans_per_order);
        }
    }
    else
    {
        ALfloat w_scale, xyz_scale;

        SetChannelMap(device->RealOut.ChannelName, device->Dry.Ambi.Coeffs,
                      chanmap, count, &device->Dry.NumChannels);
        device->Dry.CoeffCount = coeffcount;

        w_scale = (device->Dry.CoeffCount > 9) ? W_SCALE_3H0P :
                  (device->Dry.CoeffCount > 4) ? W_SCALE_2H0P : 1.0f;
        xyz_scale = (device->Dry.CoeffCount > 9) ? XYZ_SCALE_3H0P :
                    (device->Dry.CoeffCount > 4) ? XYZ_SCALE_2H0P : 1.0f;

        memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
        for(i = 0;i < device->Dry.NumChannels;i++)
        {
            device->FOAOut.Ambi.Coeffs[i][0] = device->Dry.Ambi.Coeffs[i][0] * w_scale;
            for(j = 1;j < 4;j++)
                device->FOAOut.Ambi.Coeffs[i][j] = device->Dry.Ambi.Coeffs[i][j] * xyz_scale;
        }
        device->FOAOut.CoeffCount = 4;
        device->FOAOut.NumChannels = 0;
    }
    device->RealOut.NumChannels = 0;
}

static void InitCustomPanning(ALCdevice *device, const AmbDecConf *conf, const ALsizei speakermap[MAX_OUTPUT_CHANNELS])
{
    ChannelMap chanmap[MAX_OUTPUT_CHANNELS];
    const ALfloat *coeff_scale = N3D2N3DScale;
    ALfloat w_scale = 1.0f;
    ALfloat xyz_scale = 1.0f;
    ALsizei i, j;

    if(conf->FreqBands != 1)
        ERR("Basic renderer uses the high-frequency matrix as single-band (xover_freq = %.0fhz)\n",
            conf->XOverFreq);

    if((conf->ChanMask&AMBI_PERIPHONIC_MASK))
    {
        if(conf->ChanMask > 0x1ff)
        {
            w_scale = W_SCALE_3H3P;
            xyz_scale = XYZ_SCALE_3H3P;
        }
        else if(conf->ChanMask > 0xf)
        {
            w_scale = W_SCALE_2H2P;
            xyz_scale = XYZ_SCALE_2H2P;
        }
    }
    else
    {
        if(conf->ChanMask > 0x1ff)
        {
            w_scale = W_SCALE_3H0P;
            xyz_scale = XYZ_SCALE_3H0P;
        }
        else if(conf->ChanMask > 0xf)
        {
            w_scale = W_SCALE_2H0P;
            xyz_scale = XYZ_SCALE_2H0P;
        }
    }

    if(conf->CoeffScale == ADS_SN3D)
        coeff_scale = SN3D2N3DScale;
    else if(conf->CoeffScale == ADS_FuMa)
        coeff_scale = FuMa2N3DScale;

    for(i = 0;i < conf->NumSpeakers;i++)
    {
        ALsizei chan = speakermap[i];
        ALfloat gain;
        ALsizei k = 0;

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
                  conf->NumSpeakers, &device->Dry.NumChannels);
    device->Dry.CoeffCount = (conf->ChanMask > 0x1ff) ? 16 :
                             (conf->ChanMask > 0xf) ? 9 : 4;

    memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
    for(i = 0;i < device->Dry.NumChannels;i++)
    {
        device->FOAOut.Ambi.Coeffs[i][0] = device->Dry.Ambi.Coeffs[i][0] * w_scale;
        for(j = 1;j < 4;j++)
            device->FOAOut.Ambi.Coeffs[i][j] = device->Dry.Ambi.Coeffs[i][j] * xyz_scale;
    }
    device->FOAOut.CoeffCount = 4;
    device->FOAOut.NumChannels = 0;

    device->RealOut.NumChannels = 0;

    InitDistanceComp(device, conf, speakermap);
}

static void InitHQPanning(ALCdevice *device, const AmbDecConf *conf, const ALsizei speakermap[MAX_OUTPUT_CHANNELS])
{
    static const ALsizei chans_per_order2d[MAX_AMBI_ORDER+1] = { 1, 2, 2, 2 };
    static const ALsizei chans_per_order3d[MAX_AMBI_ORDER+1] = { 1, 3, 5, 7 };
    ALfloat avg_dist;
    ALsizei count;
    ALsizei i;

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
    bformatdec_reset(device->AmbiDecoder, conf, count, device->Frequency, speakermap);

    if(!(conf->ChanMask > 0xf))
    {
        device->FOAOut.Ambi = device->Dry.Ambi;
        device->FOAOut.CoeffCount = device->Dry.CoeffCount;
        device->FOAOut.NumChannels = 0;
    }
    else
    {
        memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
        if((conf->ChanMask&AMBI_PERIPHONIC_MASK))
        {
            count = 4;
            for(i = 0;i < count;i++)
            {
                device->FOAOut.Ambi.Map[i].Scale = 1.0f;
                device->FOAOut.Ambi.Map[i].Index = i;
            }
        }
        else
        {
            static const int map[3] = { 0, 1, 3 };
            count = 3;
            for(i = 0;i < count;i++)
            {
                device->FOAOut.Ambi.Map[i].Scale = 1.0f;
                device->FOAOut.Ambi.Map[i].Index = map[i];
            }
        }
        device->FOAOut.CoeffCount = 0;
        device->FOAOut.NumChannels = count;
    }

    device->RealOut.NumChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);

    avg_dist = 0.0f;
    for(i = 0;i < conf->NumSpeakers;i++)
        avg_dist += conf->Speakers[i].Distance;
    avg_dist /= (ALfloat)conf->NumSpeakers;
    InitNearFieldCtrl(device, avg_dist,
        (conf->ChanMask > 0x1ff) ? 3 : (conf->ChanMask > 0xf) ? 2 : 1,
        (conf->ChanMask&AMBI_PERIPHONIC_MASK) ? chans_per_order3d : chans_per_order2d
    );

    InitDistanceComp(device, conf, speakermap);
}

static void InitHrtfPanning(ALCdevice *device)
{
    /* NOTE: azimuth goes clockwise. */
    static const struct AngularPoint AmbiPoints[] = {
        { DEG2RAD( 90.0f), DEG2RAD(   0.0f) },
        { DEG2RAD( 35.0f), DEG2RAD(  45.0f) },
        { DEG2RAD( 35.0f), DEG2RAD( 135.0f) },
        { DEG2RAD( 35.0f), DEG2RAD(-135.0f) },
        { DEG2RAD( 35.0f), DEG2RAD( -45.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(   0.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(  45.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(  90.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( 135.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( 180.0f) },
        { DEG2RAD(  0.0f), DEG2RAD(-135.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( -90.0f) },
        { DEG2RAD(  0.0f), DEG2RAD( -45.0f) },
        { DEG2RAD(-35.0f), DEG2RAD(  45.0f) },
        { DEG2RAD(-35.0f), DEG2RAD( 135.0f) },
        { DEG2RAD(-35.0f), DEG2RAD(-135.0f) },
        { DEG2RAD(-35.0f), DEG2RAD( -45.0f) },
        { DEG2RAD(-90.0f), DEG2RAD(   0.0f) },
    };
    static const ALfloat AmbiMatrixFOA[][MAX_AMBI_COEFFS] = {
        { 5.55555556e-02f,  0.00000000e+00f,  1.23717915e-01f,  0.00000000e+00f },
        { 5.55555556e-02f, -5.00000000e-02f,  7.14285715e-02f,  5.00000000e-02f },
        { 5.55555556e-02f, -5.00000000e-02f,  7.14285715e-02f, -5.00000000e-02f },
        { 5.55555556e-02f,  5.00000000e-02f,  7.14285715e-02f, -5.00000000e-02f },
        { 5.55555556e-02f,  5.00000000e-02f,  7.14285715e-02f,  5.00000000e-02f },
        { 5.55555556e-02f,  0.00000000e+00f,  0.00000000e+00f,  8.66025404e-02f },
        { 5.55555556e-02f, -6.12372435e-02f,  0.00000000e+00f,  6.12372435e-02f },
        { 5.55555556e-02f, -8.66025404e-02f,  0.00000000e+00f,  0.00000000e+00f },
        { 5.55555556e-02f, -6.12372435e-02f,  0.00000000e+00f, -6.12372435e-02f },
        { 5.55555556e-02f,  0.00000000e+00f,  0.00000000e+00f, -8.66025404e-02f },
        { 5.55555556e-02f,  6.12372435e-02f,  0.00000000e+00f, -6.12372435e-02f },
        { 5.55555556e-02f,  8.66025404e-02f,  0.00000000e+00f,  0.00000000e+00f },
        { 5.55555556e-02f,  6.12372435e-02f,  0.00000000e+00f,  6.12372435e-02f },
        { 5.55555556e-02f, -5.00000000e-02f, -7.14285715e-02f,  5.00000000e-02f },
        { 5.55555556e-02f, -5.00000000e-02f, -7.14285715e-02f, -5.00000000e-02f },
        { 5.55555556e-02f,  5.00000000e-02f, -7.14285715e-02f, -5.00000000e-02f },
        { 5.55555556e-02f,  5.00000000e-02f, -7.14285715e-02f,  5.00000000e-02f },
        { 5.55555556e-02f,  0.00000000e+00f, -1.23717915e-01f,  0.00000000e+00f },
    }, AmbiMatrixHOA[][MAX_AMBI_COEFFS] = {
        { 5.55555556e-02f,  0.00000000e+00f,  1.23717915e-01f,  0.00000000e+00f,  0.00000000e+00f,  0.00000000e+00f },
        { 5.55555556e-02f, -5.00000000e-02f,  7.14285715e-02f,  5.00000000e-02f, -4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f, -5.00000000e-02f,  7.14285715e-02f, -5.00000000e-02f,  4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  5.00000000e-02f,  7.14285715e-02f, -5.00000000e-02f, -4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  5.00000000e-02f,  7.14285715e-02f,  5.00000000e-02f,  4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  0.00000000e+00f,  0.00000000e+00f,  8.66025404e-02f,  0.00000000e+00f,  1.29099445e-01f },
        { 5.55555556e-02f, -6.12372435e-02f,  0.00000000e+00f,  6.12372435e-02f, -6.83467648e-02f,  0.00000000e+00f },
        { 5.55555556e-02f, -8.66025404e-02f,  0.00000000e+00f,  0.00000000e+00f,  0.00000000e+00f, -1.29099445e-01f },
        { 5.55555556e-02f, -6.12372435e-02f,  0.00000000e+00f, -6.12372435e-02f,  6.83467648e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  0.00000000e+00f,  0.00000000e+00f, -8.66025404e-02f,  0.00000000e+00f,  1.29099445e-01f },
        { 5.55555556e-02f,  6.12372435e-02f,  0.00000000e+00f, -6.12372435e-02f, -6.83467648e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  8.66025404e-02f,  0.00000000e+00f,  0.00000000e+00f,  0.00000000e+00f, -1.29099445e-01f },
        { 5.55555556e-02f,  6.12372435e-02f,  0.00000000e+00f,  6.12372435e-02f,  6.83467648e-02f,  0.00000000e+00f },
        { 5.55555556e-02f, -5.00000000e-02f, -7.14285715e-02f,  5.00000000e-02f, -4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f, -5.00000000e-02f, -7.14285715e-02f, -5.00000000e-02f,  4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  5.00000000e-02f, -7.14285715e-02f, -5.00000000e-02f, -4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  5.00000000e-02f, -7.14285715e-02f,  5.00000000e-02f,  4.55645099e-02f,  0.00000000e+00f },
        { 5.55555556e-02f,  0.00000000e+00f, -1.23717915e-01f,  0.00000000e+00f,  0.00000000e+00f,  0.00000000e+00f },
    };
    static const ALfloat AmbiOrderHFGainFOA[MAX_AMBI_ORDER+1] = {
        3.00000000e+00f, 1.73205081e+00f
    }, AmbiOrderHFGainHOA[MAX_AMBI_ORDER+1] = {
        2.40192231e+00f, 1.86052102e+00f, 9.60768923e-01f
    };
    static const ALsizei IndexMap[6] = { 0, 1, 2, 3, 4, 8 };
    static const ALsizei ChansPerOrder[MAX_AMBI_ORDER+1] = { 1, 3, 2, 0 };
    const ALfloat (*restrict AmbiMatrix)[MAX_AMBI_COEFFS] = AmbiMatrixFOA;
    const ALfloat *restrict AmbiOrderHFGain = AmbiOrderHFGainFOA;
    ALsizei count = 4;
    ALsizei i;

    static_assert(COUNTOF(AmbiPoints) == COUNTOF(AmbiMatrixFOA), "FOA Ambisonic HRTF mismatch");
    static_assert(COUNTOF(AmbiPoints) == COUNTOF(AmbiMatrixHOA), "HOA Ambisonic HRTF mismatch");
    static_assert(COUNTOF(AmbiPoints) <= HRTF_AMBI_MAX_CHANNELS, "HRTF_AMBI_MAX_CHANNELS is too small");

    if(device->AmbiUp)
    {
        AmbiMatrix = AmbiMatrixHOA;
        AmbiOrderHFGain = AmbiOrderHFGainHOA;
        count = COUNTOF(IndexMap);
    }

    device->Hrtf = al_calloc(16, FAM_SIZE(DirectHrtfState, Chan, count));

    for(i = 0;i < count;i++)
    {
        device->Dry.Ambi.Map[i].Scale = 1.0f;
        device->Dry.Ambi.Map[i].Index = IndexMap[i];
    }
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = count;

    if(device->AmbiUp)
    {
        memset(&device->FOAOut.Ambi, 0, sizeof(device->FOAOut.Ambi));
        for(i = 0;i < 4;i++)
        {
            device->FOAOut.Ambi.Map[i].Scale = 1.0f;
            device->FOAOut.Ambi.Map[i].Index = i;
        }
        device->FOAOut.CoeffCount = 0;
        device->FOAOut.NumChannels = 4;

        ambiup_reset(device->AmbiUp, device, AmbiOrderHFGainFOA[0] / AmbiOrderHFGain[0],
                     AmbiOrderHFGainFOA[1] / AmbiOrderHFGain[1]);
    }
    else
    {
        device->FOAOut.Ambi = device->Dry.Ambi;
        device->FOAOut.CoeffCount = device->Dry.CoeffCount;
        device->FOAOut.NumChannels = 0;
    }

    device->RealOut.NumChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);

    BuildBFormatHrtf(device->HrtfHandle,
        device->Hrtf, device->Dry.NumChannels, AmbiPoints, AmbiMatrix, COUNTOF(AmbiPoints),
        AmbiOrderHFGain
    );

    InitNearFieldCtrl(device, device->HrtfHandle->distance, device->AmbiUp ? 2 : 1,
                      ChansPerOrder);
}

static void InitUhjPanning(ALCdevice *device)
{
    ALsizei count = 3;
    ALsizei i;

    for(i = 0;i < count;i++)
    {
        ALsizei acn = FuMa2ACN[i];
        device->Dry.Ambi.Map[i].Scale = 1.0f/FuMa2N3DScale[acn];
        device->Dry.Ambi.Map[i].Index = acn;
    }
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = count;

    device->FOAOut.Ambi = device->Dry.Ambi;
    device->FOAOut.CoeffCount = device->Dry.CoeffCount;
    device->FOAOut.NumChannels = 0;

    device->RealOut.NumChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);
}

void aluInitRenderer(ALCdevice *device, ALint hrtf_id, enum HrtfRequestMode hrtf_appreq, enum HrtfRequestMode hrtf_userreq)
{
    /* Hold the HRTF the device last used, in case it's used again. */
    struct Hrtf *old_hrtf = device->HrtfHandle;
    const char *mode;
    bool headphones;
    int bs2blevel;
    size_t i;

    al_free(device->Hrtf);
    device->Hrtf = NULL;
    device->HrtfHandle = NULL;
    alstr_clear(&device->HrtfName);
    device->Render_Mode = NormalRender;

    memset(&device->Dry.Ambi, 0, sizeof(device->Dry.Ambi));
    device->Dry.CoeffCount = 0;
    device->Dry.NumChannels = 0;
    for(i = 0;i < MAX_AMBI_ORDER+1;i++)
        device->Dry.NumChannelsPerOrder[i] = 0;

    device->AvgSpeakerDist = 0.0f;
    memset(device->ChannelDelay, 0, sizeof(device->ChannelDelay));
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        device->ChannelDelay[i].Gain = 1.0f;
        device->ChannelDelay[i].Length = 0;
    }

    al_free(device->Stablizer);
    device->Stablizer = NULL;

    if(device->FmtChans != DevFmtStereo)
    {
        ALsizei speakermap[MAX_OUTPUT_CHANNELS];
        const char *devname, *layout = NULL;
        AmbDecConf conf, *pconf = NULL;

        if(old_hrtf)
            Hrtf_DecRef(old_hrtf);
        old_hrtf = NULL;
        if(hrtf_appreq == Hrtf_Enable)
            device->HrtfStatus = ALC_HRTF_UNSUPPORTED_FORMAT_SOFT;

        ambdec_init(&conf);

        devname = alstr_get_cstr(device->DeviceName);
        switch(device->FmtChans)
        {
            case DevFmtQuad: layout = "quad"; break;
            case DevFmtX51: /* fall-through */
            case DevFmtX51Rear: layout = "surround51"; break;
            case DevFmtX61: layout = "surround61"; break;
            case DevFmtX71: layout = "surround71"; break;
            /* Mono, Stereo, and Ambisonics output don't use custom decoders. */
            case DevFmtMono:
            case DevFmtStereo:
            case DevFmtAmbi3D:
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
            ambiup_free(&device->AmbiUp);
            if(!device->AmbiDecoder)
                device->AmbiDecoder = bformatdec_alloc();
        }
        else
        {
            bformatdec_free(&device->AmbiDecoder);
            if(device->FmtChans != DevFmtAmbi3D || device->AmbiOrder < 2)
                ambiup_free(&device->AmbiUp);
            else
            {
                if(!device->AmbiUp)
                    device->AmbiUp = ambiup_alloc();
            }
        }

        if(!pconf)
            InitPanning(device);
        else if(device->AmbiDecoder)
            InitHQPanning(device, pconf, speakermap);
        else
            InitCustomPanning(device, pconf, speakermap);

        /* Enable the stablizer only for formats that have front-left, front-
         * right, and front-center outputs.
         */
        switch(device->FmtChans)
        {
        case DevFmtX51:
        case DevFmtX51Rear:
        case DevFmtX61:
        case DevFmtX71:
            if(GetConfigValueBool(devname, NULL, "front-stablizer", 0))
            {
                /* Initialize band-splitting filters for the front-left and
                 * front-right channels, with a crossover at 5khz (could be
                 * higher).
                 */
                ALfloat scale = (ALfloat)(5000.0 / device->Frequency);
                FrontStablizer *stablizer = al_calloc(16, sizeof(*stablizer));

                bandsplit_init(&stablizer->LFilter, scale);
                stablizer->RFilter = stablizer->LFilter;

                /* Initialize all-pass filters for all other channels. */
                splitterap_init(&stablizer->APFilter[0], scale);
                for(i = 1;i < (size_t)device->RealOut.NumChannels;i++)
                    stablizer->APFilter[i] = stablizer->APFilter[0];

                device->Stablizer = stablizer;
            }
            break;
        case DevFmtMono:
        case DevFmtStereo:
        case DevFmtQuad:
        case DevFmtAmbi3D:
            break;
        }
        TRACE("Front stablizer %s\n", device->Stablizer ? "enabled" : "disabled");

        ambdec_deinit(&conf);
        return;
    }

    bformatdec_free(&device->AmbiDecoder);

    headphones = device->IsHeadphones;
    if(device->Type != Loopback)
    {
        const char *mode;
        if(ConfigValueStr(alstr_get_cstr(device->DeviceName), NULL, "stereo-mode", &mode))
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

        device->HrtfStatus = ALC_HRTF_ENABLED_SOFT;
        if(headphones && hrtf_appreq != Hrtf_Disable)
            device->HrtfStatus = ALC_HRTF_HEADPHONES_DETECTED_SOFT;
    }
    else
    {
        if(hrtf_userreq != Hrtf_Enable)
        {
            if(hrtf_appreq == Hrtf_Enable)
                device->HrtfStatus = ALC_HRTF_DENIED_SOFT;
            goto no_hrtf;
        }
        device->HrtfStatus = ALC_HRTF_REQUIRED_SOFT;
    }

    if(VECTOR_SIZE(device->HrtfList) == 0)
    {
        VECTOR_DEINIT(device->HrtfList);
        device->HrtfList = EnumerateHrtf(device->DeviceName);
    }

    if(hrtf_id >= 0 && (size_t)hrtf_id < VECTOR_SIZE(device->HrtfList))
    {
        const EnumeratedHrtf *entry = &VECTOR_ELEM(device->HrtfList, hrtf_id);
        struct Hrtf *hrtf = GetLoadedHrtf(entry->hrtf);
        if(hrtf && hrtf->sampleRate == device->Frequency)
        {
            device->HrtfHandle = hrtf;
            alstr_copy(&device->HrtfName, entry->name);
        }
        else if(hrtf)
            Hrtf_DecRef(hrtf);
    }

    for(i = 0;!device->HrtfHandle && i < VECTOR_SIZE(device->HrtfList);i++)
    {
        const EnumeratedHrtf *entry = &VECTOR_ELEM(device->HrtfList, i);
        struct Hrtf *hrtf = GetLoadedHrtf(entry->hrtf);
        if(hrtf && hrtf->sampleRate == device->Frequency)
        {
            device->HrtfHandle = hrtf;
            alstr_copy(&device->HrtfName, entry->name);
        }
        else if(hrtf)
            Hrtf_DecRef(hrtf);
    }

    if(device->HrtfHandle)
    {
        if(old_hrtf)
            Hrtf_DecRef(old_hrtf);
        old_hrtf = NULL;

        device->Render_Mode = HrtfRender;
        if(ConfigValueStr(alstr_get_cstr(device->DeviceName), NULL, "hrtf-mode", &mode))
        {
            if(strcasecmp(mode, "full") == 0)
                device->Render_Mode = HrtfRender;
            else if(strcasecmp(mode, "basic") == 0)
                device->Render_Mode = NormalRender;
            else
                ERR("Unexpected hrtf-mode: %s\n", mode);
        }

        if(device->Render_Mode == HrtfRender)
        {
            /* Don't bother with HOA when using full HRTF rendering. Nothing
             * needs it, and it eases the CPU/memory load.
             */
            ambiup_free(&device->AmbiUp);
        }
        else
        {
            if(!device->AmbiUp)
                device->AmbiUp = ambiup_alloc();
        }

        TRACE("%s HRTF rendering enabled, using \"%s\"\n",
            ((device->Render_Mode == HrtfRender) ? "Full" : "Basic"),
            alstr_get_cstr(device->HrtfName)
        );
        InitHrtfPanning(device);
        return;
    }
    device->HrtfStatus = ALC_HRTF_UNSUPPORTED_FORMAT_SOFT;

no_hrtf:
    if(old_hrtf)
        Hrtf_DecRef(old_hrtf);
    old_hrtf = NULL;
    TRACE("HRTF disabled\n");

    device->Render_Mode = StereoPair;

    ambiup_free(&device->AmbiUp);

    bs2blevel = ((headphones && hrtf_appreq != Hrtf_Disable) ||
                 (hrtf_appreq == Hrtf_Enable)) ? 5 : 0;
    if(device->Type != Loopback)
        ConfigValueInt(alstr_get_cstr(device->DeviceName), NULL, "cf_level", &bs2blevel);
    if(bs2blevel > 0 && bs2blevel <= 6)
    {
        device->Bs2b = al_calloc(16, sizeof(*device->Bs2b));
        bs2b_set_params(device->Bs2b, bs2blevel, device->Frequency);
        TRACE("BS2B enabled\n");
        InitPanning(device);
        return;
    }

    TRACE("BS2B disabled\n");

    if(ConfigValueStr(alstr_get_cstr(device->DeviceName), NULL, "stereo-encoding", &mode))
    {
        if(strcasecmp(mode, "uhj") == 0)
            device->Render_Mode = NormalRender;
        else if(strcasecmp(mode, "panpot") != 0)
            ERR("Unexpected stereo-encoding: %s\n", mode);
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
    ALsizei i;

    memset(slot->ChanMap, 0, sizeof(slot->ChanMap));
    slot->NumChannels = 0;

    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
    {
        slot->ChanMap[i].Scale = 1.0f;
        slot->ChanMap[i].Index = i;
    }
    slot->NumChannels = i;
}
