/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
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
#include "alSource.h"
#include "alBuffer.h"
#include "alListener.h"
#include "alAuxEffectSlot.h"
#include "alu.h"
#include "bs2b.h"
#include "hrtf.h"
#include "uhjfilter.h"
#include "bformatdec.h"
#include "static_assert.h"

#include "mixer_defs.h"

#include "backends/base.h"


struct ChanMap {
    enum Channel channel;
    ALfloat angle;
    ALfloat elevation;
};

/* Cone scalar */
ALfloat ConeScale = 1.0f;

/* Localized Z scalar for mono sources */
ALfloat ZScale = 1.0f;

extern inline ALfloat minf(ALfloat a, ALfloat b);
extern inline ALfloat maxf(ALfloat a, ALfloat b);
extern inline ALfloat clampf(ALfloat val, ALfloat min, ALfloat max);

extern inline ALdouble mind(ALdouble a, ALdouble b);
extern inline ALdouble maxd(ALdouble a, ALdouble b);
extern inline ALdouble clampd(ALdouble val, ALdouble min, ALdouble max);

extern inline ALuint minu(ALuint a, ALuint b);
extern inline ALuint maxu(ALuint a, ALuint b);
extern inline ALuint clampu(ALuint val, ALuint min, ALuint max);

extern inline ALint mini(ALint a, ALint b);
extern inline ALint maxi(ALint a, ALint b);
extern inline ALint clampi(ALint val, ALint min, ALint max);

extern inline ALint64 mini64(ALint64 a, ALint64 b);
extern inline ALint64 maxi64(ALint64 a, ALint64 b);
extern inline ALint64 clampi64(ALint64 val, ALint64 min, ALint64 max);

extern inline ALuint64 minu64(ALuint64 a, ALuint64 b);
extern inline ALuint64 maxu64(ALuint64 a, ALuint64 b);
extern inline ALuint64 clampu64(ALuint64 val, ALuint64 min, ALuint64 max);

extern inline ALfloat lerp(ALfloat val1, ALfloat val2, ALfloat mu);
extern inline ALfloat resample_fir4(ALfloat val0, ALfloat val1, ALfloat val2, ALfloat val3, ALuint frac);
extern inline ALfloat resample_fir8(ALfloat val0, ALfloat val1, ALfloat val2, ALfloat val3, ALfloat val4, ALfloat val5, ALfloat val6, ALfloat val7, ALuint frac);

extern inline void aluVectorSet(aluVector *restrict vector, ALfloat x, ALfloat y, ALfloat z, ALfloat w);

extern inline void aluMatrixfSetRow(aluMatrixf *matrix, ALuint row,
                                    ALfloat m0, ALfloat m1, ALfloat m2, ALfloat m3);
extern inline void aluMatrixfSet(aluMatrixf *matrix,
                                 ALfloat m00, ALfloat m01, ALfloat m02, ALfloat m03,
                                 ALfloat m10, ALfloat m11, ALfloat m12, ALfloat m13,
                                 ALfloat m20, ALfloat m21, ALfloat m22, ALfloat m23,
                                 ALfloat m30, ALfloat m31, ALfloat m32, ALfloat m33);

const aluMatrixf IdentityMatrixf = {{
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
}};


static inline HrtfDirectMixerFunc SelectHrtfMixer(void)
{
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
        return MixDirectHrtf_SSE;
#endif
#ifdef HAVE_NEON
    if((CPUCapFlags&CPU_CAP_NEON))
        return MixDirectHrtf_Neon;
#endif

    return MixDirectHrtf_C;
}


static inline void aluCrossproduct(const ALfloat *inVector1, const ALfloat *inVector2, ALfloat *outVector)
{
    outVector[0] = inVector1[1]*inVector2[2] - inVector1[2]*inVector2[1];
    outVector[1] = inVector1[2]*inVector2[0] - inVector1[0]*inVector2[2];
    outVector[2] = inVector1[0]*inVector2[1] - inVector1[1]*inVector2[0];
}

static inline ALfloat aluDotproduct(const aluVector *vec1, const aluVector *vec2)
{
    return vec1->v[0]*vec2->v[0] + vec1->v[1]*vec2->v[1] + vec1->v[2]*vec2->v[2];
}

static ALfloat aluNormalize(ALfloat *vec)
{
    ALfloat length = sqrtf(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
    if(length > 0.0f)
    {
        ALfloat inv_length = 1.0f/length;
        vec[0] *= inv_length;
        vec[1] *= inv_length;
        vec[2] *= inv_length;
    }
    return length;
}

static void aluMatrixfFloat3(ALfloat *vec, ALfloat w, const aluMatrixf *mtx)
{
    ALfloat v[4] = { vec[0], vec[1], vec[2], w };

    vec[0] = v[0]*mtx->m[0][0] + v[1]*mtx->m[1][0] + v[2]*mtx->m[2][0] + v[3]*mtx->m[3][0];
    vec[1] = v[0]*mtx->m[0][1] + v[1]*mtx->m[1][1] + v[2]*mtx->m[2][1] + v[3]*mtx->m[3][1];
    vec[2] = v[0]*mtx->m[0][2] + v[1]*mtx->m[1][2] + v[2]*mtx->m[2][2] + v[3]*mtx->m[3][2];
}

static aluVector aluMatrixfVector(const aluMatrixf *mtx, const aluVector *vec)
{
    aluVector v;
    v.v[0] = vec->v[0]*mtx->m[0][0] + vec->v[1]*mtx->m[1][0] + vec->v[2]*mtx->m[2][0] + vec->v[3]*mtx->m[3][0];
    v.v[1] = vec->v[0]*mtx->m[0][1] + vec->v[1]*mtx->m[1][1] + vec->v[2]*mtx->m[2][1] + vec->v[3]*mtx->m[3][1];
    v.v[2] = vec->v[0]*mtx->m[0][2] + vec->v[1]*mtx->m[1][2] + vec->v[2]*mtx->m[2][2] + vec->v[3]*mtx->m[3][2];
    v.v[3] = vec->v[0]*mtx->m[0][3] + vec->v[1]*mtx->m[1][3] + vec->v[2]*mtx->m[2][3] + vec->v[3]*mtx->m[3][3];
    return v;
}


/* Prepares the interpolator for a given rate (determined by increment).  A
 * result of AL_FALSE indicates that the filter output will completely cut
 * the input signal.
 *
 * With a bit of work, and a trade of memory for CPU cost, this could be
 * modified for use with an interpolated increment for buttery-smooth pitch
 * changes.
 */
static ALboolean BsincPrepare(const ALuint increment, BsincState *state)
{
    static const ALfloat scaleBase = 1.510578918e-01f, scaleRange = 1.177936623e+00f;
    static const ALuint m[BSINC_SCALE_COUNT] = { 24, 24, 24, 24, 24, 24, 24, 20, 20, 20, 16, 16, 16, 12, 12, 12 };
    static const ALuint to[4][BSINC_SCALE_COUNT] =
    {
        { 0, 24, 408, 792, 1176, 1560, 1944, 2328, 2648, 2968, 3288, 3544, 3800, 4056, 4248, 4440 },
        { 4632, 5016, 5400, 5784, 6168, 6552, 6936, 7320, 7640, 7960, 8280, 8536, 8792, 9048, 9240, 0 },
        { 0, 9432, 9816, 10200, 10584, 10968, 11352, 11736, 12056, 12376, 12696, 12952, 13208, 13464, 13656, 13848 },
        { 14040, 14424, 14808, 15192, 15576, 15960, 16344, 16728, 17048, 17368, 17688, 17944, 18200, 18456, 18648, 0 }
    };
    static const ALuint tm[2][BSINC_SCALE_COUNT] =
    {
        { 0, 24, 24, 24, 24, 24, 24, 20, 20, 20, 16, 16, 16, 12, 12, 12 },
        { 24, 24, 24, 24, 24, 24, 24, 20, 20, 20, 16, 16, 16, 12, 12, 0 }
    };
    ALfloat sf;
    ALuint si, pi;
    ALboolean uncut = AL_TRUE;

    if(increment > FRACTIONONE)
    {
        sf = (ALfloat)FRACTIONONE / increment;
        if(sf < scaleBase)
        {
            /* Signal has been completely cut.  The return result can be used
             * to skip the filter (and output zeros) as an optimization.
             */
            sf = 0.0f;
            si = 0;
            uncut = AL_FALSE;
        }
        else
        {
            sf = (BSINC_SCALE_COUNT - 1) * (sf - scaleBase) * scaleRange;
            si = fastf2u(sf);
            /* The interpolation factor is fit to this diagonally-symmetric
             * curve to reduce the transition ripple caused by interpolating
             * different scales of the sinc function.
             */
            sf = 1.0f - cosf(asinf(sf - si));
        }
    }
    else
    {
        sf = 0.0f;
        si = BSINC_SCALE_COUNT - 1;
    }

    state->sf = sf;
    state->m = m[si];
    state->l = -(ALint)((m[si] / 2) - 1);
    /* The CPU cost of this table re-mapping could be traded for the memory
     * cost of a complete table map (1024 elements large).
     */
    for(pi = 0;pi < BSINC_PHASE_COUNT;pi++)
    {
        state->coeffs[pi].filter  = &bsincTab[to[0][si] + tm[0][si]*pi];
        state->coeffs[pi].scDelta = &bsincTab[to[1][si] + tm[1][si]*pi];
        state->coeffs[pi].phDelta = &bsincTab[to[2][si] + tm[0][si]*pi];
        state->coeffs[pi].spDelta = &bsincTab[to[3][si] + tm[1][si]*pi];
    }
    return uncut;
}


static ALboolean CalcListenerParams(ALCcontext *Context)
{
    ALlistener *Listener = Context->Listener;
    ALfloat N[3], V[3], U[3], P[3];
    struct ALlistenerProps *first;
    struct ALlistenerProps *props;
    aluVector vel;

    props = ATOMIC_EXCHANGE(struct ALlistenerProps*, &Listener->Update, NULL, almemory_order_acq_rel);
    if(!props) return AL_FALSE;

    /* AT then UP */
    N[0] = ATOMIC_LOAD(&props->Forward[0], almemory_order_relaxed);
    N[1] = ATOMIC_LOAD(&props->Forward[1], almemory_order_relaxed);
    N[2] = ATOMIC_LOAD(&props->Forward[2], almemory_order_relaxed);
    aluNormalize(N);
    V[0] = ATOMIC_LOAD(&props->Up[0], almemory_order_relaxed);
    V[1] = ATOMIC_LOAD(&props->Up[1], almemory_order_relaxed);
    V[2] = ATOMIC_LOAD(&props->Up[2], almemory_order_relaxed);
    aluNormalize(V);
    /* Build and normalize right-vector */
    aluCrossproduct(N, V, U);
    aluNormalize(U);

    aluMatrixfSet(&Listener->Params.Matrix,
        U[0], V[0], -N[0], 0.0,
        U[1], V[1], -N[1], 0.0,
        U[2], V[2], -N[2], 0.0,
         0.0,  0.0,   0.0, 1.0
    );

    P[0] = ATOMIC_LOAD(&props->Position[0], almemory_order_relaxed);
    P[1] = ATOMIC_LOAD(&props->Position[1], almemory_order_relaxed);
    P[2] = ATOMIC_LOAD(&props->Position[2], almemory_order_relaxed);
    aluMatrixfFloat3(P, 1.0, &Listener->Params.Matrix);
    aluMatrixfSetRow(&Listener->Params.Matrix, 3, -P[0], -P[1], -P[2], 1.0f);

    aluVectorSet(&vel, ATOMIC_LOAD(&props->Velocity[0], almemory_order_relaxed),
                       ATOMIC_LOAD(&props->Velocity[1], almemory_order_relaxed),
                       ATOMIC_LOAD(&props->Velocity[2], almemory_order_relaxed),
                       0.0f);
    Listener->Params.Velocity = aluMatrixfVector(&Listener->Params.Matrix, &vel);

    Listener->Params.Gain = ATOMIC_LOAD(&props->Gain, almemory_order_relaxed) * Context->GainBoost;
    Listener->Params.MetersPerUnit = ATOMIC_LOAD(&props->MetersPerUnit, almemory_order_relaxed);

    Listener->Params.DopplerFactor = ATOMIC_LOAD(&props->DopplerFactor, almemory_order_relaxed);
    Listener->Params.SpeedOfSound = ATOMIC_LOAD(&props->SpeedOfSound, almemory_order_relaxed) *
                                    ATOMIC_LOAD(&props->DopplerVelocity, almemory_order_relaxed);

    Listener->Params.SourceDistanceModel = ATOMIC_LOAD(&props->SourceDistanceModel, almemory_order_relaxed);
    Listener->Params.DistanceModel = ATOMIC_LOAD(&props->DistanceModel, almemory_order_relaxed);

    /* WARNING: A livelock is theoretically possible if another thread keeps
     * changing the freelist head without giving this a chance to actually swap
     * in the old container (practically impossible with this little code,
     * but...).
     */
    first = ATOMIC_LOAD(&Listener->FreeList);
    do {
        ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
    } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALlistenerProps*,
            &Listener->FreeList, &first, props) == 0);

    return AL_TRUE;
}

static ALboolean CalcEffectSlotParams(ALeffectslot *slot, ALCdevice *device)
{
    struct ALeffectslotProps *first;
    struct ALeffectslotProps *props;
    ALeffectState *state;

    props = ATOMIC_EXCHANGE(struct ALeffectslotProps*, &slot->Update, NULL, almemory_order_acq_rel);
    if(!props) return AL_FALSE;

    slot->Params.Gain = ATOMIC_LOAD(&props->Gain, almemory_order_relaxed);
    slot->Params.AuxSendAuto = ATOMIC_LOAD(&props->AuxSendAuto, almemory_order_relaxed);
    slot->Params.EffectType = ATOMIC_LOAD(&props->Type, almemory_order_relaxed);
    if(IsReverbEffect(slot->Params.EffectType))
    {
        slot->Params.RoomRolloff = props->Props.Reverb.RoomRolloffFactor;
        slot->Params.DecayTime = props->Props.Reverb.DecayTime;
        slot->Params.AirAbsorptionGainHF = props->Props.Reverb.AirAbsorptionGainHF;
    }
    else
    {
        slot->Params.RoomRolloff = 0.0f;
        slot->Params.DecayTime = 0.0f;
        slot->Params.AirAbsorptionGainHF = 1.0f;
    }

    /* Swap effect states. No need to play with the ref counts since they keep
     * the same number of refs.
     */
    state = ATOMIC_EXCHANGE(ALeffectState*, &props->State, slot->Params.EffectState,
                            almemory_order_relaxed);
    slot->Params.EffectState = state;

    V(state,update)(device, slot, &props->Props);

    /* WARNING: A livelock is theoretically possible if another thread keeps
     * changing the freelist head without giving this a chance to actually swap
     * in the old container (practically impossible with this little code,
     * but...).
     */
    first = ATOMIC_LOAD(&slot->FreeList);
    do {
        ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
    } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALeffectslotProps*,
            &slot->FreeList, &first, props) == 0);

    return AL_TRUE;
}


static void CalcNonAttnSourceParams(ALvoice *voice, const struct ALsourceProps *props, const ALbuffer *ALBuffer, const ALCcontext *ALContext)
{
    static const struct ChanMap MonoMap[1] = {
        { FrontCenter, 0.0f, 0.0f }
    }, RearMap[2] = {
        { BackLeft,  DEG2RAD(-150.0f), DEG2RAD(0.0f) },
        { BackRight, DEG2RAD( 150.0f), DEG2RAD(0.0f) }
    }, QuadMap[4] = {
        { FrontLeft,  DEG2RAD( -45.0f), DEG2RAD(0.0f) },
        { FrontRight, DEG2RAD(  45.0f), DEG2RAD(0.0f) },
        { BackLeft,   DEG2RAD(-135.0f), DEG2RAD(0.0f) },
        { BackRight,  DEG2RAD( 135.0f), DEG2RAD(0.0f) }
    }, X51Map[6] = {
        { FrontLeft,   DEG2RAD( -30.0f), DEG2RAD(0.0f) },
        { FrontRight,  DEG2RAD(  30.0f), DEG2RAD(0.0f) },
        { FrontCenter, DEG2RAD(   0.0f), DEG2RAD(0.0f) },
        { LFE, 0.0f, 0.0f },
        { SideLeft,    DEG2RAD(-110.0f), DEG2RAD(0.0f) },
        { SideRight,   DEG2RAD( 110.0f), DEG2RAD(0.0f) }
    }, X61Map[7] = {
        { FrontLeft,    DEG2RAD(-30.0f), DEG2RAD(0.0f) },
        { FrontRight,   DEG2RAD( 30.0f), DEG2RAD(0.0f) },
        { FrontCenter,  DEG2RAD(  0.0f), DEG2RAD(0.0f) },
        { LFE, 0.0f, 0.0f },
        { BackCenter,   DEG2RAD(180.0f), DEG2RAD(0.0f) },
        { SideLeft,     DEG2RAD(-90.0f), DEG2RAD(0.0f) },
        { SideRight,    DEG2RAD( 90.0f), DEG2RAD(0.0f) }
    }, X71Map[8] = {
        { FrontLeft,   DEG2RAD( -30.0f), DEG2RAD(0.0f) },
        { FrontRight,  DEG2RAD(  30.0f), DEG2RAD(0.0f) },
        { FrontCenter, DEG2RAD(   0.0f), DEG2RAD(0.0f) },
        { LFE, 0.0f, 0.0f },
        { BackLeft,    DEG2RAD(-150.0f), DEG2RAD(0.0f) },
        { BackRight,   DEG2RAD( 150.0f), DEG2RAD(0.0f) },
        { SideLeft,    DEG2RAD( -90.0f), DEG2RAD(0.0f) },
        { SideRight,   DEG2RAD(  90.0f), DEG2RAD(0.0f) }
    };

    const ALCdevice *Device = ALContext->Device;
    const ALlistener *Listener = ALContext->Listener;
    ALfloat SourceVolume,ListenerGain,MinVolume,MaxVolume;
    ALfloat DryGain, DryGainHF, DryGainLF;
    ALfloat WetGain[MAX_SENDS];
    ALfloat WetGainHF[MAX_SENDS];
    ALfloat WetGainLF[MAX_SENDS];
    ALeffectslot *SendSlots[MAX_SENDS];
    ALuint NumSends, Frequency;
    ALboolean Relative;
    const struct ChanMap *chans = NULL;
    struct ChanMap StereoMap[2] = {
        { FrontLeft,  DEG2RAD(-30.0f), DEG2RAD(0.0f) },
        { FrontRight, DEG2RAD( 30.0f), DEG2RAD(0.0f) }
    };
    ALuint num_channels = 0;
    ALboolean DirectChannels;
    ALboolean isbformat = AL_FALSE;
    ALfloat Pitch;
    ALuint i, j, c;

    /* Get device properties */
    NumSends  = Device->NumAuxSends;
    Frequency = Device->Frequency;

    /* Get listener properties */
    ListenerGain = Listener->Params.Gain;

    /* Get source properties */
    SourceVolume   = ATOMIC_LOAD(&props->Gain, almemory_order_relaxed);
    MinVolume      = ATOMIC_LOAD(&props->MinGain, almemory_order_relaxed);
    MaxVolume      = ATOMIC_LOAD(&props->MaxGain, almemory_order_relaxed);
    Pitch          = ATOMIC_LOAD(&props->Pitch, almemory_order_relaxed);
    Relative       = ATOMIC_LOAD(&props->HeadRelative, almemory_order_relaxed);
    DirectChannels = ATOMIC_LOAD(&props->DirectChannels, almemory_order_relaxed);

    /* Convert counter-clockwise to clockwise. */
    StereoMap[0].angle = -ATOMIC_LOAD(&props->StereoPan[0], almemory_order_relaxed);
    StereoMap[1].angle = -ATOMIC_LOAD(&props->StereoPan[1], almemory_order_relaxed);

    voice->DirectOut.Buffer = Device->Dry.Buffer;
    voice->DirectOut.Channels = Device->Dry.NumChannels;
    for(i = 0;i < NumSends;i++)
    {
        SendSlots[i] = ATOMIC_LOAD(&props->Send[i].Slot, almemory_order_relaxed);
        if(!SendSlots[i] && i == 0)
            SendSlots[i] = Device->DefaultSlot;
        if(!SendSlots[i] || SendSlots[i]->Params.EffectType == AL_EFFECT_NULL)
        {
            SendSlots[i] = NULL;
            voice->SendOut[i].Buffer = NULL;
            voice->SendOut[i].Channels = 0;
        }
        else
        {
            voice->SendOut[i].Buffer = SendSlots[i]->WetBuffer;
            voice->SendOut[i].Channels = SendSlots[i]->NumChannels;
        }
    }

    /* Calculate the stepping value */
    Pitch *= (ALfloat)ALBuffer->Frequency / Frequency;
    if(Pitch > (ALfloat)MAX_PITCH)
        voice->Step = MAX_PITCH<<FRACTIONBITS;
    else
        voice->Step = maxi(fastf2i(Pitch*FRACTIONONE + 0.5f), 1);
    BsincPrepare(voice->Step, &voice->SincState);

    /* Calculate gains */
    DryGain  = clampf(SourceVolume, MinVolume, MaxVolume);
    DryGain *= ATOMIC_LOAD(&props->Direct.Gain, almemory_order_relaxed) * ListenerGain;
    DryGain  = minf(DryGain, GAIN_MIX_MAX);
    DryGainHF = ATOMIC_LOAD(&props->Direct.GainHF, almemory_order_relaxed);
    DryGainLF = ATOMIC_LOAD(&props->Direct.GainLF, almemory_order_relaxed);
    for(i = 0;i < NumSends;i++)
    {
        WetGain[i]  = clampf(SourceVolume, MinVolume, MaxVolume);
        WetGain[i] *= ATOMIC_LOAD(&props->Send[i].Gain, almemory_order_relaxed) * ListenerGain;
        WetGain[i]  = minf(WetGain[i], GAIN_MIX_MAX);
        WetGainHF[i] = ATOMIC_LOAD(&props->Send[i].GainHF, almemory_order_relaxed);
        WetGainLF[i] = ATOMIC_LOAD(&props->Send[i].GainLF, almemory_order_relaxed);
    }

    switch(ALBuffer->FmtChannels)
    {
    case FmtMono:
        chans = MonoMap;
        num_channels = 1;
        break;

    case FmtStereo:
        chans = StereoMap;
        num_channels = 2;
        break;

    case FmtRear:
        chans = RearMap;
        num_channels = 2;
        break;

    case FmtQuad:
        chans = QuadMap;
        num_channels = 4;
        break;

    case FmtX51:
        chans = X51Map;
        num_channels = 6;
        break;

    case FmtX61:
        chans = X61Map;
        num_channels = 7;
        break;

    case FmtX71:
        chans = X71Map;
        num_channels = 8;
        break;

    case FmtBFormat2D:
        num_channels = 3;
        isbformat = AL_TRUE;
        DirectChannels = AL_FALSE;
        break;

    case FmtBFormat3D:
        num_channels = 4;
        isbformat = AL_TRUE;
        DirectChannels = AL_FALSE;
        break;
    }

    if(isbformat)
    {
        ALfloat N[3], V[3], U[3];
        aluMatrixf matrix;
        ALfloat scale;

        /* AT then UP */
        N[0] = ATOMIC_LOAD(&props->Orientation[0][0], almemory_order_relaxed);
        N[1] = ATOMIC_LOAD(&props->Orientation[0][1], almemory_order_relaxed);
        N[2] = ATOMIC_LOAD(&props->Orientation[0][2], almemory_order_relaxed);
        aluNormalize(N);
        V[0] = ATOMIC_LOAD(&props->Orientation[1][0], almemory_order_relaxed);
        V[1] = ATOMIC_LOAD(&props->Orientation[1][1], almemory_order_relaxed);
        V[2] = ATOMIC_LOAD(&props->Orientation[1][2], almemory_order_relaxed);
        aluNormalize(V);
        if(!Relative)
        {
            const aluMatrixf *lmatrix = &Listener->Params.Matrix;
            aluMatrixfFloat3(N, 0.0f, lmatrix);
            aluMatrixfFloat3(V, 0.0f, lmatrix);
        }
        /* Build and normalize right-vector */
        aluCrossproduct(N, V, U);
        aluNormalize(U);

        /* Build a rotate + conversion matrix (FuMa -> ACN+N3D). */
        scale = 1.732050808f;
        aluMatrixfSet(&matrix,
            1.414213562f,        0.0f,        0.0f,        0.0f,
                    0.0f, -N[0]*scale,  N[1]*scale, -N[2]*scale,
                    0.0f,  U[0]*scale, -U[1]*scale,  U[2]*scale,
                    0.0f, -V[0]*scale,  V[1]*scale, -V[2]*scale
        );

        voice->DirectOut.Buffer = Device->FOAOut.Buffer;
        voice->DirectOut.Channels = Device->FOAOut.NumChannels;
        for(c = 0;c < num_channels;c++)
            ComputeFirstOrderGains(Device->FOAOut, matrix.m[c], DryGain,
                                   voice->Chan[c].Direct.Gains.Target);

        for(i = 0;i < NumSends;i++)
        {
            if(!SendSlots[i])
            {
                for(c = 0;c < num_channels;c++)
                {
                    for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                        voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                }
            }
            else
            {
                for(c = 0;c < num_channels;c++)
                {
                    const ALeffectslot *Slot = SendSlots[i];
                    ComputeFirstOrderGainsBF(Slot->ChanMap, Slot->NumChannels, matrix.m[c],
                                             WetGain[i], voice->Chan[c].Send[i].Gains.Target);
                }
            }
        }

        voice->IsHrtf = AL_FALSE;
    }
    else
    {
        ALfloat coeffs[MAX_AMBI_COEFFS];

        if(DirectChannels)
        {
            /* Skip the virtual channels and write inputs to the real output. */
            voice->DirectOut.Buffer = Device->RealOut.Buffer;
            voice->DirectOut.Channels = Device->RealOut.NumChannels;
            for(c = 0;c < num_channels;c++)
            {
                int idx;
                for(j = 0;j < MAX_OUTPUT_CHANNELS;j++)
                    voice->Chan[c].Direct.Gains.Target[j] = 0.0f;
                if((idx=GetChannelIdxByName(Device->RealOut, chans[c].channel)) != -1)
                    voice->Chan[c].Direct.Gains.Target[idx] = DryGain;
            }

            /* Auxiliary sends still use normal panning since they mix to B-Format, which can't
             * channel-match. */
            for(c = 0;c < num_channels;c++)
            {
                CalcAngleCoeffs(chans[c].angle, chans[c].elevation, 0.0f, coeffs);

                for(i = 0;i < NumSends;i++)
                {
                    if(!SendSlots[i])
                    {
                        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                            voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                    }
                    else
                    {
                        const ALeffectslot *Slot = SendSlots[i];
                        ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels, coeffs,
                                              WetGain[i], voice->Chan[c].Send[i].Gains.Target);
                    }
                }
            }

            voice->IsHrtf = AL_FALSE;
        }
        else if(Device->Render_Mode == HrtfRender)
        {
            /* Full HRTF rendering. Skip the virtual channels and render each
             * input channel to the real outputs.
             */
            voice->DirectOut.Buffer = Device->RealOut.Buffer;
            voice->DirectOut.Channels = Device->RealOut.NumChannels;
            for(c = 0;c < num_channels;c++)
            {
                if(chans[c].channel == LFE)
                {
                    /* Skip LFE */
                    voice->Chan[c].Direct.Hrtf.Target.Delay[0] = 0;
                    voice->Chan[c].Direct.Hrtf.Target.Delay[1] = 0;
                    for(i = 0;i < HRIR_LENGTH;i++)
                    {
                        voice->Chan[c].Direct.Hrtf.Target.Coeffs[i][0] = 0.0f;
                        voice->Chan[c].Direct.Hrtf.Target.Coeffs[i][1] = 0.0f;
                    }

                    for(i = 0;i < NumSends;i++)
                    {
                        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                            voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                    }

                    continue;
                }

                /* Get the static HRIR coefficients and delays for this channel. */
                GetHrtfCoeffs(Device->Hrtf.Handle,
                    chans[c].elevation, chans[c].angle, 0.0f, DryGain,
                    voice->Chan[c].Direct.Hrtf.Target.Coeffs,
                    voice->Chan[c].Direct.Hrtf.Target.Delay
                );

                /* Normal panning for auxiliary sends. */
                CalcAngleCoeffs(chans[c].angle, chans[c].elevation, 0.0f, coeffs);

                for(i = 0;i < NumSends;i++)
                {
                    if(!SendSlots[i])
                    {
                        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                            voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                    }
                    else
                    {
                        const ALeffectslot *Slot = SendSlots[i];
                        ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels, coeffs,
                                              WetGain[i], voice->Chan[c].Send[i].Gains.Target);
                    }
                }
            }

            voice->IsHrtf = AL_TRUE;
        }
        else
        {
            /* Non-HRTF rendering. Use normal panning to the output. */
            for(c = 0;c < num_channels;c++)
            {
                /* Special-case LFE */
                if(chans[c].channel == LFE)
                {
                    for(j = 0;j < MAX_OUTPUT_CHANNELS;j++)
                        voice->Chan[c].Direct.Gains.Target[j] = 0.0f;
                    if(Device->Dry.Buffer == Device->RealOut.Buffer)
                    {
                        int idx;
                        if((idx=GetChannelIdxByName(Device->RealOut, chans[c].channel)) != -1)
                            voice->Chan[c].Direct.Gains.Target[idx] = DryGain;
                    }

                    for(i = 0;i < NumSends;i++)
                    {
                        ALuint j;
                        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                            voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                    }
                    continue;
                }

                if(Device->Render_Mode == StereoPair)
                {
                    /* Clamp X so it remains within 30 degrees of 0 or 180 degree azimuth. */
                    ALfloat x = sinf(chans[c].angle) * cosf(chans[c].elevation);
                    coeffs[0] = clampf(-x, -0.5f, 0.5f) + 0.5f;
                    voice->Chan[c].Direct.Gains.Target[0] = coeffs[0] * DryGain;
                    voice->Chan[c].Direct.Gains.Target[1] = (1.0f-coeffs[0]) * DryGain;
                    for(j = 2;j < MAX_OUTPUT_CHANNELS;j++)
                        voice->Chan[c].Direct.Gains.Target[j] = 0.0f;

                    CalcAngleCoeffs(chans[c].angle, chans[c].elevation, 0.0f, coeffs);
                }
                else
                {
                    CalcAngleCoeffs(chans[c].angle, chans[c].elevation, 0.0f, coeffs);
                    ComputePanningGains(Device->Dry, coeffs, DryGain,
                                        voice->Chan[c].Direct.Gains.Target);
                }

                for(i = 0;i < NumSends;i++)
                {
                    if(!SendSlots[i])
                    {
                        ALuint j;
                        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                            voice->Chan[c].Send[i].Gains.Target[j] = 0.0f;
                    }
                    else
                    {
                        const ALeffectslot *Slot = SendSlots[i];
                        ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels, coeffs,
                                              WetGain[i], voice->Chan[c].Send[i].Gains.Target);
                    }
                }
            }

            voice->IsHrtf = AL_FALSE;
        }
    }

    {
        ALfloat hfscale = ATOMIC_LOAD(&props->Direct.HFReference, almemory_order_relaxed) /
                          Frequency;
        ALfloat lfscale = ATOMIC_LOAD(&props->Direct.LFReference, almemory_order_relaxed) /
                          Frequency;
        DryGainHF = maxf(DryGainHF, 0.0001f);
        DryGainLF = maxf(DryGainLF, 0.0001f);
        for(c = 0;c < num_channels;c++)
        {
            voice->Chan[c].Direct.FilterType = AF_None;
            if(DryGainHF != 1.0f) voice->Chan[c].Direct.FilterType |= AF_LowPass;
            if(DryGainLF != 1.0f) voice->Chan[c].Direct.FilterType |= AF_HighPass;
            ALfilterState_setParams(
                &voice->Chan[c].Direct.LowPass, ALfilterType_HighShelf,
                DryGainHF, hfscale, calc_rcpQ_from_slope(DryGainHF, 0.75f)
            );
            ALfilterState_setParams(
                &voice->Chan[c].Direct.HighPass, ALfilterType_LowShelf,
                DryGainLF, lfscale, calc_rcpQ_from_slope(DryGainLF, 0.75f)
            );
        }
    }
    for(i = 0;i < NumSends;i++)
    {
        ALfloat hfscale = ATOMIC_LOAD(&props->Send[i].HFReference, almemory_order_relaxed) /
                          Frequency;
        ALfloat lfscale = ATOMIC_LOAD(&props->Send[i].LFReference, almemory_order_relaxed) /
                          Frequency;
        WetGainHF[i] = maxf(WetGainHF[i], 0.0001f);
        WetGainLF[i] = maxf(WetGainLF[i], 0.0001f);
        for(c = 0;c < num_channels;c++)
        {
            voice->Chan[c].Send[i].FilterType = AF_None;
            if(WetGainHF[i] != 1.0f) voice->Chan[c].Send[i].FilterType |= AF_LowPass;
            if(WetGainLF[i] != 1.0f) voice->Chan[c].Send[i].FilterType |= AF_HighPass;
            ALfilterState_setParams(
                &voice->Chan[c].Send[i].LowPass, ALfilterType_HighShelf,
                WetGainHF[i], hfscale, calc_rcpQ_from_slope(WetGainHF[i], 0.75f)
            );
            ALfilterState_setParams(
                &voice->Chan[c].Send[i].HighPass, ALfilterType_LowShelf,
                WetGainLF[i], lfscale, calc_rcpQ_from_slope(WetGainLF[i], 0.75f)
            );
        }
    }
}

static void CalcAttnSourceParams(ALvoice *voice, const struct ALsourceProps *props, const ALbuffer *ALBuffer, const ALCcontext *ALContext)
{
    const ALCdevice *Device = ALContext->Device;
    const ALlistener *Listener = ALContext->Listener;
    aluVector Position, Velocity, Direction, SourceToListener;
    ALfloat InnerAngle,OuterAngle,Distance,ClampedDist;
    ALfloat MinVolume,MaxVolume,MinDist,MaxDist,Rolloff;
    ALfloat SourceVolume,ListenerGain;
    ALfloat DopplerFactor, SpeedOfSound;
    ALfloat AirAbsorptionFactor;
    ALfloat RoomAirAbsorption[MAX_SENDS];
    ALeffectslot *SendSlots[MAX_SENDS];
    ALfloat Attenuation;
    ALfloat RoomAttenuation[MAX_SENDS];
    ALfloat MetersPerUnit;
    ALfloat RoomRolloffBase;
    ALfloat RoomRolloff[MAX_SENDS];
    ALfloat DecayDistance[MAX_SENDS];
    ALfloat DryGain;
    ALfloat DryGainHF;
    ALfloat DryGainLF;
    ALboolean DryGainHFAuto;
    ALfloat WetGain[MAX_SENDS];
    ALfloat WetGainHF[MAX_SENDS];
    ALfloat WetGainLF[MAX_SENDS];
    ALboolean WetGainAuto;
    ALboolean WetGainHFAuto;
    ALfloat Pitch;
    ALuint Frequency;
    ALint NumSends;
    ALint i;

    DryGainHF = 1.0f;
    DryGainLF = 1.0f;
    for(i = 0;i < MAX_SENDS;i++)
    {
        WetGainHF[i] = 1.0f;
        WetGainLF[i] = 1.0f;
    }

    /* Get context/device properties */
    DopplerFactor = Listener->Params.DopplerFactor;
    SpeedOfSound  = Listener->Params.SpeedOfSound;
    NumSends      = Device->NumAuxSends;
    Frequency     = Device->Frequency;

    /* Get listener properties */
    ListenerGain  = Listener->Params.Gain;
    MetersPerUnit = Listener->Params.MetersPerUnit;

    /* Get source properties */
    SourceVolume   = ATOMIC_LOAD(&props->Gain, almemory_order_relaxed);
    MinVolume      = ATOMIC_LOAD(&props->MinGain, almemory_order_relaxed);
    MaxVolume      = ATOMIC_LOAD(&props->MaxGain, almemory_order_relaxed);
    Pitch          = ATOMIC_LOAD(&props->Pitch, almemory_order_relaxed);
    aluVectorSet(&Position, ATOMIC_LOAD(&props->Position[0], almemory_order_relaxed),
                            ATOMIC_LOAD(&props->Position[1], almemory_order_relaxed),
                            ATOMIC_LOAD(&props->Position[2], almemory_order_relaxed),
                            1.0f);
    aluVectorSet(&Direction, ATOMIC_LOAD(&props->Direction[0], almemory_order_relaxed),
                             ATOMIC_LOAD(&props->Direction[1], almemory_order_relaxed),
                             ATOMIC_LOAD(&props->Direction[2], almemory_order_relaxed),
                             0.0f);
    aluVectorSet(&Velocity, ATOMIC_LOAD(&props->Velocity[0], almemory_order_relaxed),
                            ATOMIC_LOAD(&props->Velocity[1], almemory_order_relaxed),
                            ATOMIC_LOAD(&props->Velocity[2], almemory_order_relaxed),
                            0.0f);
    MinDist        = ATOMIC_LOAD(&props->RefDistance, almemory_order_relaxed);
    MaxDist        = ATOMIC_LOAD(&props->MaxDistance, almemory_order_relaxed);
    Rolloff        = ATOMIC_LOAD(&props->RollOffFactor, almemory_order_relaxed);
    DopplerFactor *= ATOMIC_LOAD(&props->DopplerFactor, almemory_order_relaxed);
    InnerAngle     = ATOMIC_LOAD(&props->InnerAngle, almemory_order_relaxed);
    OuterAngle     = ATOMIC_LOAD(&props->OuterAngle, almemory_order_relaxed);
    AirAbsorptionFactor = ATOMIC_LOAD(&props->AirAbsorptionFactor, almemory_order_relaxed);
    DryGainHFAuto   = ATOMIC_LOAD(&props->DryGainHFAuto, almemory_order_relaxed);
    WetGainAuto     = ATOMIC_LOAD(&props->WetGainAuto, almemory_order_relaxed);
    WetGainHFAuto   = ATOMIC_LOAD(&props->WetGainHFAuto, almemory_order_relaxed);
    RoomRolloffBase = ATOMIC_LOAD(&props->RoomRolloffFactor, almemory_order_relaxed);

    voice->DirectOut.Buffer = Device->Dry.Buffer;
    voice->DirectOut.Channels = Device->Dry.NumChannels;
    for(i = 0;i < NumSends;i++)
    {
        SendSlots[i] = ATOMIC_LOAD(&props->Send[i].Slot, almemory_order_relaxed);

        if(!SendSlots[i] && i == 0)
            SendSlots[i] = Device->DefaultSlot;
        if(!SendSlots[i] || SendSlots[i]->Params.EffectType == AL_EFFECT_NULL)
        {
            SendSlots[i] = NULL;
            RoomRolloff[i] = 0.0f;
            DecayDistance[i] = 0.0f;
            RoomAirAbsorption[i] = 1.0f;
        }
        else if(SendSlots[i]->Params.AuxSendAuto)
        {
            RoomRolloff[i] = SendSlots[i]->Params.RoomRolloff + RoomRolloffBase;
            DecayDistance[i] = SendSlots[i]->Params.DecayTime *
                               SPEEDOFSOUNDMETRESPERSEC;
            RoomAirAbsorption[i] = SendSlots[i]->Params.AirAbsorptionGainHF;
        }
        else
        {
            /* If the slot's auxiliary send auto is off, the data sent to the
             * effect slot is the same as the dry path, sans filter effects */
            RoomRolloff[i] = Rolloff;
            DecayDistance[i] = 0.0f;
            RoomAirAbsorption[i] = AIRABSORBGAINHF;
        }

        if(!SendSlots[i])
        {
            voice->SendOut[i].Buffer = NULL;
            voice->SendOut[i].Channels = 0;
        }
        else
        {
            voice->SendOut[i].Buffer = SendSlots[i]->WetBuffer;
            voice->SendOut[i].Channels = SendSlots[i]->NumChannels;
        }
    }

    /* Transform source to listener space (convert to head relative) */
    if(ATOMIC_LOAD(&props->HeadRelative, almemory_order_relaxed) == AL_FALSE)
    {
        const aluMatrixf *Matrix = &Listener->Params.Matrix;
        /* Transform source vectors */
        Position = aluMatrixfVector(Matrix, &Position);
        Velocity = aluMatrixfVector(Matrix, &Velocity);
        Direction = aluMatrixfVector(Matrix, &Direction);
    }
    else
    {
        const aluVector *lvelocity = &Listener->Params.Velocity;
        /* Offset the source velocity to be relative of the listener velocity */
        Velocity.v[0] += lvelocity->v[0];
        Velocity.v[1] += lvelocity->v[1];
        Velocity.v[2] += lvelocity->v[2];
    }

    aluNormalize(Direction.v);
    SourceToListener.v[0] = -Position.v[0];
    SourceToListener.v[1] = -Position.v[1];
    SourceToListener.v[2] = -Position.v[2];
    SourceToListener.v[3] = 0.0f;
    Distance = aluNormalize(SourceToListener.v);

    /* Calculate distance attenuation */
    ClampedDist = Distance;

    Attenuation = 1.0f;
    for(i = 0;i < NumSends;i++)
        RoomAttenuation[i] = 1.0f;
    switch(Listener->Params.SourceDistanceModel ?
           ATOMIC_LOAD(&props->DistanceModel, almemory_order_relaxed) :
           Listener->Params.DistanceModel)
    {
        case InverseDistanceClamped:
            ClampedDist = clampf(ClampedDist, MinDist, MaxDist);
            if(MaxDist < MinDist)
                break;
            /*fall-through*/
        case InverseDistance:
            if(MinDist > 0.0f)
            {
                ALfloat dist = lerp(MinDist, ClampedDist, Rolloff);
                if(dist > 0.0f) Attenuation = MinDist / dist;
                for(i = 0;i < NumSends;i++)
                {
                    dist = lerp(MinDist, ClampedDist, RoomRolloff[i]);
                    if(dist > 0.0f) RoomAttenuation[i] = MinDist / dist;
                }
            }
            break;

        case LinearDistanceClamped:
            ClampedDist = clampf(ClampedDist, MinDist, MaxDist);
            if(MaxDist < MinDist)
                break;
            /*fall-through*/
        case LinearDistance:
            if(MaxDist != MinDist)
            {
                Attenuation = 1.0f - (Rolloff*(ClampedDist-MinDist)/(MaxDist - MinDist));
                Attenuation = maxf(Attenuation, 0.0f);
                for(i = 0;i < NumSends;i++)
                {
                    RoomAttenuation[i] = 1.0f - (RoomRolloff[i]*(ClampedDist-MinDist)/(MaxDist - MinDist));
                    RoomAttenuation[i] = maxf(RoomAttenuation[i], 0.0f);
                }
            }
            break;

        case ExponentDistanceClamped:
            ClampedDist = clampf(ClampedDist, MinDist, MaxDist);
            if(MaxDist < MinDist)
                break;
            /*fall-through*/
        case ExponentDistance:
            if(ClampedDist > 0.0f && MinDist > 0.0f)
            {
                Attenuation = powf(ClampedDist/MinDist, -Rolloff);
                for(i = 0;i < NumSends;i++)
                    RoomAttenuation[i] = powf(ClampedDist/MinDist, -RoomRolloff[i]);
            }
            break;

        case DisableDistance:
            ClampedDist = MinDist;
            break;
    }

    /* Source Gain + Attenuation */
    DryGain = SourceVolume * Attenuation;
    for(i = 0;i < NumSends;i++)
        WetGain[i] = SourceVolume * RoomAttenuation[i];

    /* Distance-based air absorption */
    if(AirAbsorptionFactor > 0.0f && ClampedDist > MinDist)
    {
        ALfloat meters = (ClampedDist-MinDist) * MetersPerUnit;
        DryGainHF *= powf(AIRABSORBGAINHF, AirAbsorptionFactor*meters);
        for(i = 0;i < NumSends;i++)
            WetGainHF[i] *= powf(RoomAirAbsorption[i], AirAbsorptionFactor*meters);
    }

    if(WetGainAuto)
    {
        ALfloat ApparentDist = 1.0f/maxf(Attenuation, 0.00001f) - 1.0f;

        /* Apply a decay-time transformation to the wet path, based on the
         * attenuation of the dry path.
         *
         * Using the apparent distance, based on the distance attenuation, the
         * initial decay of the reverb effect is calculated and applied to the
         * wet path.
         */
        for(i = 0;i < NumSends;i++)
        {
            if(DecayDistance[i] > 0.0f)
                WetGain[i] *= powf(0.001f/*-60dB*/, ApparentDist/DecayDistance[i]);
        }
    }

    /* Calculate directional soundcones */
    if(InnerAngle < 360.0f)
    {
        ALfloat ConeVolume;
        ALfloat ConeHF;
        ALfloat Angle;
        ALfloat scale;

        Angle = RAD2DEG(acosf(aluDotproduct(&Direction, &SourceToListener)) * ConeScale) * 2.0f;
        if(Angle > InnerAngle)
        {
            if(Angle < OuterAngle)
            {
                scale = (Angle-InnerAngle) / (OuterAngle-InnerAngle);
                ConeVolume = lerp(
                    1.0f, ATOMIC_LOAD(&props->OuterGain, almemory_order_relaxed), scale
                );
                ConeHF = lerp(
                    1.0f, ATOMIC_LOAD(&props->OuterGainHF, almemory_order_relaxed), scale
                );
            }
            else
            {
                ConeVolume = ATOMIC_LOAD(&props->OuterGain, almemory_order_relaxed);
                ConeHF = ATOMIC_LOAD(&props->OuterGainHF, almemory_order_relaxed);
            }
            DryGain *= ConeVolume;
            if(DryGainHFAuto)
                DryGainHF *= ConeHF;
        }

        /* Wet path uses the total area of the cone emitter (the room will
         * receive the same amount of sound regardless of its direction).
         */
        scale = (asinf(maxf((OuterAngle-InnerAngle)/360.0f, 0.0f)) / F_PI) +
                (InnerAngle/360.0f);
        if(WetGainAuto)
        {
            ConeVolume = lerp(
                1.0f, ATOMIC_LOAD(&props->OuterGain, almemory_order_relaxed), scale
            );
            for(i = 0;i < NumSends;i++)
                WetGain[i] *= ConeVolume;
        }
        if(WetGainHFAuto)
        {
            ConeHF = lerp(
                1.0f, ATOMIC_LOAD(&props->OuterGainHF, almemory_order_relaxed), scale
            );
            for(i = 0;i < NumSends;i++)
                WetGainHF[i] *= ConeHF;
        }
    }

    /* Apply gain and frequency filters */
    DryGain  = clampf(DryGain, MinVolume, MaxVolume);
    DryGain *= ATOMIC_LOAD(&props->Direct.Gain, almemory_order_relaxed) * ListenerGain;
    DryGain  = minf(DryGain, GAIN_MIX_MAX);
    DryGainHF *= ATOMIC_LOAD(&props->Direct.GainHF, almemory_order_relaxed);
    DryGainLF *= ATOMIC_LOAD(&props->Direct.GainLF, almemory_order_relaxed);
    for(i = 0;i < NumSends;i++)
    {
        WetGain[i]  = clampf(WetGain[i], MinVolume, MaxVolume);
        WetGain[i] *= ATOMIC_LOAD(&props->Send[i].Gain, almemory_order_relaxed) * ListenerGain;
        WetGain[i]  = minf(WetGain[i], GAIN_MIX_MAX);
        WetGainHF[i] *= ATOMIC_LOAD(&props->Send[i].GainHF, almemory_order_relaxed);
        WetGainLF[i] *= ATOMIC_LOAD(&props->Send[i].GainLF, almemory_order_relaxed);
    }

    /* Calculate velocity-based doppler effect */
    if(DopplerFactor > 0.0f)
    {
        const aluVector *lvelocity = &Listener->Params.Velocity;
        ALfloat VSS, VLS;

        if(SpeedOfSound < 1.0f)
        {
            DopplerFactor *= 1.0f/SpeedOfSound;
            SpeedOfSound   = 1.0f;
        }

        VSS = aluDotproduct(&Velocity, &SourceToListener) * DopplerFactor;
        VLS = aluDotproduct(lvelocity, &SourceToListener) * DopplerFactor;

        Pitch *= clampf(SpeedOfSound-VLS, 1.0f, SpeedOfSound*2.0f - 1.0f) /
                 clampf(SpeedOfSound-VSS, 1.0f, SpeedOfSound*2.0f - 1.0f);
    }

    /* Calculate fixed-point stepping value, based on the pitch, buffer
     * frequency, and output frequency.
     */
    Pitch *= (ALfloat)ALBuffer->Frequency / Frequency;
    if(Pitch > (ALfloat)MAX_PITCH)
        voice->Step = MAX_PITCH<<FRACTIONBITS;
    else
        voice->Step = maxi(fastf2i(Pitch*FRACTIONONE + 0.5f), 1);
    BsincPrepare(voice->Step, &voice->SincState);

    if(Device->Render_Mode == HrtfRender)
    {
        /* Full HRTF rendering. Skip the virtual channels and render to the
         * real outputs.
         */
        ALfloat dir[3] = { 0.0f, 0.0f, -1.0f };
        ALfloat ev = 0.0f, az = 0.0f;
        ALfloat radius = ATOMIC_LOAD(&props->Radius, almemory_order_relaxed);
        ALfloat coeffs[MAX_AMBI_COEFFS];
        ALfloat spread = 0.0f;

        voice->DirectOut.Buffer = Device->RealOut.Buffer;
        voice->DirectOut.Channels = Device->RealOut.NumChannels;

        if(Distance > FLT_EPSILON)
        {
            dir[0] = -SourceToListener.v[0];
            dir[1] = -SourceToListener.v[1];
            dir[2] = -SourceToListener.v[2] * ZScale;

            /* Calculate elevation and azimuth only when the source is not at
             * the listener. This prevents +0 and -0 Z from producing
             * inconsistent panning. Also, clamp Y in case FP precision errors
             * cause it to land outside of -1..+1. */
            ev = asinf(clampf(dir[1], -1.0f, 1.0f));
            az = atan2f(dir[0], -dir[2]);
        }
        if(radius > Distance)
            spread = F_TAU - Distance/radius*F_PI;
        else if(Distance > FLT_EPSILON)
            spread = asinf(radius / Distance) * 2.0f;

        /* Get the HRIR coefficients and delays. */
        GetHrtfCoeffs(Device->Hrtf.Handle, ev, az, spread, DryGain,
                      voice->Chan[0].Direct.Hrtf.Target.Coeffs,
                      voice->Chan[0].Direct.Hrtf.Target.Delay);

        CalcDirectionCoeffs(dir, spread, coeffs);

        for(i = 0;i < NumSends;i++)
        {
            if(!SendSlots[i])
            {
                ALuint j;
                for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                    voice->Chan[0].Send[i].Gains.Target[j] = 0.0f;
            }
            else
            {
                const ALeffectslot *Slot = SendSlots[i];
                ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels, coeffs,
                                      WetGain[i], voice->Chan[0].Send[i].Gains.Target);
            }
        }

        voice->IsHrtf = AL_TRUE;
    }
    else
    {
        /* Non-HRTF rendering. */
        ALfloat dir[3] = { 0.0f, 0.0f, -1.0f };
        ALfloat radius = ATOMIC_LOAD(&props->Radius, almemory_order_relaxed);
        ALfloat coeffs[MAX_AMBI_COEFFS];
        ALfloat spread = 0.0f;

        /* Get the localized direction, and compute panned gains. */
        if(Distance > FLT_EPSILON)
        {
            dir[0] = -SourceToListener.v[0];
            dir[1] = -SourceToListener.v[1];
            dir[2] = -SourceToListener.v[2] * ZScale;
        }
        if(radius > Distance)
            spread = F_TAU - Distance/radius*F_PI;
        else if(Distance > FLT_EPSILON)
            spread = asinf(radius / Distance) * 2.0f;

        if(Device->Render_Mode == StereoPair)
        {
            /* Clamp X so it remains within 30 degrees of 0 or 180 degree azimuth. */
            ALfloat x = -dir[0] * (0.5f * (cosf(spread*0.5f) + 1.0f));
            x = clampf(x, -0.5f, 0.5f) + 0.5f;
            voice->Chan[0].Direct.Gains.Target[0] = x * DryGain;
            voice->Chan[0].Direct.Gains.Target[1] = (1.0f-x) * DryGain;
            for(i = 2;i < MAX_OUTPUT_CHANNELS;i++)
                voice->Chan[0].Direct.Gains.Target[i] = 0.0f;

            CalcDirectionCoeffs(dir, spread, coeffs);
        }
        else
        {
            CalcDirectionCoeffs(dir, spread, coeffs);
            ComputePanningGains(Device->Dry, coeffs, DryGain,
                                voice->Chan[0].Direct.Gains.Target);
        }

        for(i = 0;i < NumSends;i++)
        {
            if(!SendSlots[i])
            {
                ALuint j;
                for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
                    voice->Chan[0].Send[i].Gains.Target[j] = 0.0f;
            }
            else
            {
                const ALeffectslot *Slot = SendSlots[i];
                ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels, coeffs,
                                      WetGain[i], voice->Chan[0].Send[i].Gains.Target);
            }
        }

        voice->IsHrtf = AL_FALSE;
    }

    {
        ALfloat hfscale = ATOMIC_LOAD(&props->Direct.HFReference, almemory_order_relaxed) /
                          Frequency;
        ALfloat lfscale = ATOMIC_LOAD(&props->Direct.LFReference, almemory_order_relaxed) /
                          Frequency;
        DryGainHF = maxf(DryGainHF, 0.0001f);
        DryGainLF = maxf(DryGainLF, 0.0001f);
        voice->Chan[0].Direct.FilterType = AF_None;
        if(DryGainHF != 1.0f) voice->Chan[0].Direct.FilterType |= AF_LowPass;
        if(DryGainLF != 1.0f) voice->Chan[0].Direct.FilterType |= AF_HighPass;
        ALfilterState_setParams(
            &voice->Chan[0].Direct.LowPass, ALfilterType_HighShelf,
            DryGainHF, hfscale, calc_rcpQ_from_slope(DryGainHF, 0.75f)
        );
        ALfilterState_setParams(
            &voice->Chan[0].Direct.HighPass, ALfilterType_LowShelf,
            DryGainLF, lfscale, calc_rcpQ_from_slope(DryGainLF, 0.75f)
        );
    }
    for(i = 0;i < NumSends;i++)
    {
        ALfloat hfscale = ATOMIC_LOAD(&props->Send[i].HFReference, almemory_order_relaxed) /
                          Frequency;
        ALfloat lfscale = ATOMIC_LOAD(&props->Send[i].LFReference, almemory_order_relaxed) /
                          Frequency;
        WetGainHF[i] = maxf(WetGainHF[i], 0.0001f);
        WetGainLF[i] = maxf(WetGainLF[i], 0.0001f);
        voice->Chan[0].Send[i].FilterType = AF_None;
        if(WetGainHF[i] != 1.0f) voice->Chan[0].Send[i].FilterType |= AF_LowPass;
        if(WetGainLF[i] != 1.0f) voice->Chan[0].Send[i].FilterType |= AF_HighPass;
        ALfilterState_setParams(
            &voice->Chan[0].Send[i].LowPass, ALfilterType_HighShelf,
            WetGainHF[i], hfscale, calc_rcpQ_from_slope(WetGainHF[i], 0.75f)
        );
        ALfilterState_setParams(
            &voice->Chan[0].Send[i].HighPass, ALfilterType_LowShelf,
            WetGainLF[i], lfscale, calc_rcpQ_from_slope(WetGainLF[i], 0.75f)
        );
    }
}

static void CalcSourceParams(ALvoice *voice, ALCcontext *context, ALboolean force)
{
    ALsource *source = voice->Source;
    const ALbufferlistitem *BufferListItem;
    struct ALsourceProps *first;
    struct ALsourceProps *props;

    props = ATOMIC_EXCHANGE(struct ALsourceProps*, &source->Update, NULL, almemory_order_acq_rel);
    if(!props && !force) return;

    if(props)
    {
        voice->Props = *props;

        /* WARNING: A livelock is theoretically possible if another thread
         * keeps changing the freelist head without giving this a chance to
         * actually swap in the old container (practically impossible with this
         * little code, but...).
         */
        first = ATOMIC_LOAD(&source->FreeList);
        do {
            ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALsourceProps*,
                &source->FreeList, &first, props) == 0);
    }

    BufferListItem = ATOMIC_LOAD(&source->queue, almemory_order_relaxed);
    while(BufferListItem != NULL)
    {
        const ALbuffer *buffer;
        if((buffer=BufferListItem->buffer) != NULL)
        {
            if(buffer->FmtChannels == FmtMono)
                CalcAttnSourceParams(voice, &voice->Props, buffer, context);
            else
                CalcNonAttnSourceParams(voice, &voice->Props, buffer, context);
            break;
        }
        BufferListItem = BufferListItem->next;
    }
}


static void UpdateContextSources(ALCcontext *ctx, ALeffectslot *slot)
{
    ALvoice *voice, *voice_end;
    ALsource *source;

    IncrementRef(&ctx->UpdateCount);
    if(!ATOMIC_LOAD(&ctx->HoldUpdates))
    {
        ALboolean force = CalcListenerParams(ctx);
        while(slot)
        {
            force |= CalcEffectSlotParams(slot, ctx->Device);
            slot = ATOMIC_LOAD(&slot->next, almemory_order_relaxed);
        }

        voice = ctx->Voices;
        voice_end = voice + ctx->VoiceCount;
        for(;voice != voice_end;++voice)
        {
            if(!(source=voice->Source)) continue;
            if(source->state != AL_PLAYING && source->state != AL_PAUSED)
                voice->Source = NULL;
            else
                CalcSourceParams(voice, ctx, force);
        }
    }
    IncrementRef(&ctx->UpdateCount);
}


/* Specialized function to clamp to [-1, +1] with only one branch. This also
 * converts NaN to 0. */
static inline ALfloat aluClampf(ALfloat val)
{
    if(fabsf(val) <= 1.0f) return val;
    return (ALfloat)((0.0f < val) - (val < 0.0f));
}

static inline ALfloat aluF2F(ALfloat val)
{ return val; }

static inline ALint aluF2I(ALfloat val)
{
    /* Floats only have a 24-bit mantissa, so [-16777215, +16777215] is the max
     * integer range normalized floats can be safely converted to.
     */
    return fastf2i(aluClampf(val)*16777215.0f)<<7;
}
static inline ALuint aluF2UI(ALfloat val)
{ return aluF2I(val)+2147483648u; }

static inline ALshort aluF2S(ALfloat val)
{ return fastf2i(aluClampf(val)*32767.0f); }
static inline ALushort aluF2US(ALfloat val)
{ return aluF2S(val)+32768; }

static inline ALbyte aluF2B(ALfloat val)
{ return fastf2i(aluClampf(val)*127.0f); }
static inline ALubyte aluF2UB(ALfloat val)
{ return aluF2B(val)+128; }

#define DECL_TEMPLATE(T, func)                                                \
static void Write_##T(ALfloatBUFFERSIZE *InBuffer, ALvoid *OutBuffer,         \
                      ALuint SamplesToDo, ALuint numchans)                    \
{                                                                             \
    ALuint i, j;                                                              \
    for(j = 0;j < numchans;j++)                                               \
    {                                                                         \
        const ALfloat *in = InBuffer[j];                                      \
        T *restrict out = (T*)OutBuffer + j;                                  \
        for(i = 0;i < SamplesToDo;i++)                                        \
            out[i*numchans] = func(in[i]);                                    \
    }                                                                         \
}

DECL_TEMPLATE(ALfloat, aluF2F)
DECL_TEMPLATE(ALuint, aluF2UI)
DECL_TEMPLATE(ALint, aluF2I)
DECL_TEMPLATE(ALushort, aluF2US)
DECL_TEMPLATE(ALshort, aluF2S)
DECL_TEMPLATE(ALubyte, aluF2UB)
DECL_TEMPLATE(ALbyte, aluF2B)

#undef DECL_TEMPLATE


ALvoid aluMixData(ALCdevice *device, ALvoid *buffer, ALsizei size)
{
    ALuint SamplesToDo;
    ALvoice *voice, *voice_end;
    ALeffectslot *slot;
    ALsource *source;
    ALCcontext *ctx;
    FPUCtl oldMode;
    ALuint i, c;

    SetMixerFPUMode(&oldMode);

    while(size > 0)
    {
        SamplesToDo = minu(size, BUFFERSIZE);
        for(c = 0;c < device->Dry.NumChannels;c++)
            memset(device->Dry.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));
        if(device->Dry.Buffer != device->RealOut.Buffer)
            for(c = 0;c < device->RealOut.NumChannels;c++)
                memset(device->RealOut.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));
        if(device->Dry.Buffer != device->FOAOut.Buffer)
            for(c = 0;c < device->FOAOut.NumChannels;c++)
                memset(device->FOAOut.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));

        IncrementRef(&device->MixCount);
        V0(device->Backend,lock)();

        if((slot=device->DefaultSlot) != NULL)
        {
            CalcEffectSlotParams(device->DefaultSlot, device);
            for(i = 0;i < slot->NumChannels;i++)
                memset(slot->WetBuffer[i], 0, SamplesToDo*sizeof(ALfloat));
        }

        ctx = ATOMIC_LOAD(&device->ContextList);
        while(ctx)
        {
            ALeffectslot *slotroot;

            slotroot = ATOMIC_LOAD(&ctx->ActiveAuxSlotList);
            UpdateContextSources(ctx, slotroot);

            slot = slotroot;
            while(slot)
            {
                for(i = 0;i < slot->NumChannels;i++)
                    memset(slot->WetBuffer[i], 0, SamplesToDo*sizeof(ALfloat));
                slot = ATOMIC_LOAD(&slot->next, almemory_order_relaxed);
            }

            /* source processing */
            voice = ctx->Voices;
            voice_end = voice + ctx->VoiceCount;
            for(;voice != voice_end;++voice)
            {
                ALboolean IsVoiceInit = (voice->Step > 0);
                source = voice->Source;
                if(source && source->state == AL_PLAYING && IsVoiceInit)
                    MixSource(voice, source, device, SamplesToDo);
            }

            /* effect slot processing */
            slot = slotroot;
            while(slot)
            {
                ALeffectState *state = slot->Params.EffectState;
                V(state,process)(SamplesToDo, SAFE_CONST(ALfloatBUFFERSIZE*,slot->WetBuffer),
                                 state->OutBuffer, state->OutChannels);
                slot = ATOMIC_LOAD(&slot->next, almemory_order_relaxed);
            }

            ctx = ctx->next;
        }

        if(device->DefaultSlot != NULL)
        {
            const ALeffectslot *slot = device->DefaultSlot;
            ALeffectState *state = slot->Params.EffectState;
            V(state,process)(SamplesToDo, slot->WetBuffer, state->OutBuffer,
                             state->OutChannels);
        }

        /* Increment the clock time. Every second's worth of samples is
         * converted and added to clock base so that large sample counts don't
         * overflow during conversion. This also guarantees an exact, stable
         * conversion. */
        device->SamplesDone += SamplesToDo;
        device->ClockBase += (device->SamplesDone/device->Frequency) * DEVICE_CLOCK_RES;
        device->SamplesDone %= device->Frequency;
        V0(device->Backend,unlock)();
        IncrementRef(&device->MixCount);

        if(device->Hrtf.Handle)
        {
            int lidx = GetChannelIdxByName(device->RealOut, FrontLeft);
            int ridx = GetChannelIdxByName(device->RealOut, FrontRight);
            if(lidx != -1 && ridx != -1)
            {
                HrtfDirectMixerFunc HrtfMix = SelectHrtfMixer();
                ALuint irsize = device->Hrtf.IrSize;
                for(c = 0;c < device->Dry.NumChannels;c++)
                {
                    HrtfMix(device->RealOut.Buffer, lidx, ridx,
                        device->Dry.Buffer[c], device->Hrtf.Offset, irsize,
                        device->Hrtf.Coeffs[c], device->Hrtf.Values[c],
                        SamplesToDo
                    );
                }
                device->Hrtf.Offset += SamplesToDo;
            }
        }
        else if(device->AmbiDecoder)
        {
            if(device->Dry.Buffer != device->FOAOut.Buffer)
                bformatdec_upSample(device->AmbiDecoder,
                    device->Dry.Buffer, SAFE_CONST(ALfloatBUFFERSIZE*,device->FOAOut.Buffer),
                    device->FOAOut.NumChannels, SamplesToDo
                );
            bformatdec_process(device->AmbiDecoder,
                device->RealOut.Buffer, device->RealOut.NumChannels,
                SAFE_CONST(ALfloatBUFFERSIZE*,device->Dry.Buffer), SamplesToDo
            );
        }
        else if(device->AmbiUp)
        {
            ambiup_process(device->AmbiUp,
                device->RealOut.Buffer, device->RealOut.NumChannels,
                SAFE_CONST(ALfloatBUFFERSIZE*,device->FOAOut.Buffer), SamplesToDo
            );
        }
        else if(device->Uhj_Encoder)
        {
            int lidx = GetChannelIdxByName(device->RealOut, FrontLeft);
            int ridx = GetChannelIdxByName(device->RealOut, FrontRight);
            if(lidx != -1 && ridx != -1)
            {
                /* Encode to stereo-compatible 2-channel UHJ output. */
                EncodeUhj2(device->Uhj_Encoder,
                    device->RealOut.Buffer[lidx], device->RealOut.Buffer[ridx],
                    device->Dry.Buffer, SamplesToDo
                );
            }
        }
        else if(device->Bs2b)
        {
            int lidx = GetChannelIdxByName(device->RealOut, FrontLeft);
            int ridx = GetChannelIdxByName(device->RealOut, FrontRight);
            if(lidx != -1 && ridx != -1)
            {
                /* Apply binaural/crossfeed filter */
                bs2b_cross_feed(device->Bs2b, device->RealOut.Buffer[lidx],
                                device->RealOut.Buffer[ridx], SamplesToDo);
            }
        }

        if(buffer)
        {
            ALfloat (*OutBuffer)[BUFFERSIZE] = device->RealOut.Buffer;
            ALuint OutChannels = device->RealOut.NumChannels;

#define WRITE(T, a, b, c, d) do {               \
    Write_##T((a), (b), (c), (d));              \
    buffer = (T*)buffer + (c)*(d);              \
} while(0)
            switch(device->FmtType)
            {
                case DevFmtByte:
                    WRITE(ALbyte, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtUByte:
                    WRITE(ALubyte, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtShort:
                    WRITE(ALshort, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtUShort:
                    WRITE(ALushort, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtInt:
                    WRITE(ALint, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtUInt:
                    WRITE(ALuint, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
                case DevFmtFloat:
                    WRITE(ALfloat, OutBuffer, buffer, SamplesToDo, OutChannels);
                    break;
            }
#undef WRITE
        }

        size -= SamplesToDo;
    }

    RestoreFPUMode(&oldMode);
}


ALvoid aluHandleDisconnect(ALCdevice *device)
{
    ALCcontext *Context;

    device->Connected = ALC_FALSE;

    Context = ATOMIC_LOAD(&device->ContextList);
    while(Context)
    {
        ALvoice *voice, *voice_end;

        voice = Context->Voices;
        voice_end = voice + Context->VoiceCount;
        while(voice != voice_end)
        {
            ALsource *source = voice->Source;
            voice->Source = NULL;

            if(source && source->state == AL_PLAYING)
            {
                source->state = AL_STOPPED;
                ATOMIC_STORE(&source->current_buffer, NULL);
                ATOMIC_STORE(&source->position, 0);
                ATOMIC_STORE(&source->position_fraction, 0);
            }

            voice++;
        }
        Context->VoiceCount = 0;

        Context = Context->next;
    }
}
