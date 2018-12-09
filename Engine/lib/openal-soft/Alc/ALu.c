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
#include "mastering.h"
#include "uhjfilter.h"
#include "bformatdec.h"
#include "static_assert.h"
#include "ringbuffer.h"
#include "filters/splitter.h"

#include "mixer/defs.h"
#include "fpu_modes.h"
#include "cpu_caps.h"
#include "bsinc_inc.h"

#include "backends/base.h"


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

extern inline size_t minz(size_t a, size_t b);
extern inline size_t maxz(size_t a, size_t b);
extern inline size_t clampz(size_t val, size_t min, size_t max);

extern inline ALfloat lerp(ALfloat val1, ALfloat val2, ALfloat mu);
extern inline ALfloat cubic(ALfloat val1, ALfloat val2, ALfloat val3, ALfloat val4, ALfloat mu);

extern inline void aluVectorSet(aluVector *restrict vector, ALfloat x, ALfloat y, ALfloat z, ALfloat w);

extern inline void aluMatrixfSetRow(aluMatrixf *matrix, ALuint row,
                                    ALfloat m0, ALfloat m1, ALfloat m2, ALfloat m3);
extern inline void aluMatrixfSet(aluMatrixf *matrix,
                                 ALfloat m00, ALfloat m01, ALfloat m02, ALfloat m03,
                                 ALfloat m10, ALfloat m11, ALfloat m12, ALfloat m13,
                                 ALfloat m20, ALfloat m21, ALfloat m22, ALfloat m23,
                                 ALfloat m30, ALfloat m31, ALfloat m32, ALfloat m33);


/* Cone scalar */
ALfloat ConeScale = 1.0f;

/* Localized Z scalar for mono sources */
ALfloat ZScale = 1.0f;

/* Force default speed of sound for distance-related reverb decay. */
ALboolean OverrideReverbSpeedOfSound = AL_FALSE;

const aluMatrixf IdentityMatrixf = {{
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
}};


static void ClearArray(ALfloat f[MAX_OUTPUT_CHANNELS])
{
    size_t i;
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        f[i] = 0.0f;
}

struct ChanMap {
    enum Channel channel;
    ALfloat angle;
    ALfloat elevation;
};

static HrtfDirectMixerFunc MixDirectHrtf = MixDirectHrtf_C;


void DeinitVoice(ALvoice *voice)
{
    al_free(ATOMIC_EXCHANGE_PTR_SEQ(&voice->Update, NULL));
}


static inline HrtfDirectMixerFunc SelectHrtfMixer(void)
{
#ifdef HAVE_NEON
    if((CPUCapFlags&CPU_CAP_NEON))
        return MixDirectHrtf_Neon;
#endif
#ifdef HAVE_SSE
    if((CPUCapFlags&CPU_CAP_SSE))
        return MixDirectHrtf_SSE;
#endif

    return MixDirectHrtf_C;
}


/* Prior to VS2013, MSVC lacks the round() family of functions. */
#if defined(_MSC_VER) && _MSC_VER < 1800
static float roundf(float val)
{
    if(val < 0.0f)
        return ceilf(val-0.5f);
    return floorf(val+0.5f);
}
#endif

/* This RNG method was created based on the math found in opusdec. It's quick,
 * and starting with a seed value of 22222, is suitable for generating
 * whitenoise.
 */
static inline ALuint dither_rng(ALuint *seed)
{
    *seed = (*seed * 96314165) + 907633515;
    return *seed;
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
    if(length > FLT_EPSILON)
    {
        ALfloat inv_length = 1.0f/length;
        vec[0] *= inv_length;
        vec[1] *= inv_length;
        vec[2] *= inv_length;
        return length;
    }
    vec[0] = vec[1] = vec[2] = 0.0f;
    return 0.0f;
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


void aluInit(void)
{
    MixDirectHrtf = SelectHrtfMixer();
}


static void SendSourceStoppedEvent(ALCcontext *context, ALuint id)
{
    ALbitfieldSOFT enabledevt;
    AsyncEvent evt;
    size_t strpos;
    ALuint scale;

    enabledevt = ATOMIC_LOAD(&context->EnabledEvts, almemory_order_acquire);
    if(!(enabledevt&EventType_SourceStateChange)) return;

    evt.EnumType = EventType_SourceStateChange;
    evt.Type = AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT;
    evt.ObjectId = id;
    evt.Param = AL_STOPPED;

    /* Normally snprintf would be used, but this is called from the mixer and
     * that function's not real-time safe, so we have to construct it manually.
     */
    strcpy(evt.Message, "Source ID "); strpos = 10;
    scale = 1000000000;
    while(scale > 0 && scale > id)
        scale /= 10;
    while(scale > 0)
    {
        evt.Message[strpos++] = '0' + ((id/scale)%10);
        scale /= 10;
    }
    strcpy(evt.Message+strpos, " state changed to AL_STOPPED");

    if(ll_ringbuffer_write(context->AsyncEvents, (const char*)&evt, 1) == 1)
        alsem_post(&context->EventSem);
}


static void ProcessHrtf(ALCdevice *device, ALsizei SamplesToDo)
{
    DirectHrtfState *state;
    int lidx, ridx;
    ALsizei c;

    if(device->AmbiUp)
        ambiup_process(device->AmbiUp,
            device->Dry.Buffer, device->Dry.NumChannels, device->FOAOut.Buffer,
            SamplesToDo
        );

    lidx = GetChannelIdxByName(&device->RealOut, FrontLeft);
    ridx = GetChannelIdxByName(&device->RealOut, FrontRight);
    assert(lidx != -1 && ridx != -1);

    state = device->Hrtf;
    for(c = 0;c < device->Dry.NumChannels;c++)
    {
        MixDirectHrtf(device->RealOut.Buffer[lidx], device->RealOut.Buffer[ridx],
            device->Dry.Buffer[c], state->Offset, state->IrSize,
            state->Chan[c].Coeffs, state->Chan[c].Values, SamplesToDo
        );
    }
    state->Offset += SamplesToDo;
}

static void ProcessAmbiDec(ALCdevice *device, ALsizei SamplesToDo)
{
    if(device->Dry.Buffer != device->FOAOut.Buffer)
        bformatdec_upSample(device->AmbiDecoder,
            device->Dry.Buffer, device->FOAOut.Buffer, device->FOAOut.NumChannels,
            SamplesToDo
        );
    bformatdec_process(device->AmbiDecoder,
        device->RealOut.Buffer, device->RealOut.NumChannels, device->Dry.Buffer,
        SamplesToDo
    );
}

static void ProcessAmbiUp(ALCdevice *device, ALsizei SamplesToDo)
{
    ambiup_process(device->AmbiUp,
        device->RealOut.Buffer, device->RealOut.NumChannels, device->FOAOut.Buffer,
        SamplesToDo
    );
}

static void ProcessUhj(ALCdevice *device, ALsizei SamplesToDo)
{
    int lidx = GetChannelIdxByName(&device->RealOut, FrontLeft);
    int ridx = GetChannelIdxByName(&device->RealOut, FrontRight);
    assert(lidx != -1 && ridx != -1);

    /* Encode to stereo-compatible 2-channel UHJ output. */
    EncodeUhj2(device->Uhj_Encoder,
        device->RealOut.Buffer[lidx], device->RealOut.Buffer[ridx],
        device->Dry.Buffer, SamplesToDo
    );
}

static void ProcessBs2b(ALCdevice *device, ALsizei SamplesToDo)
{
    int lidx = GetChannelIdxByName(&device->RealOut, FrontLeft);
    int ridx = GetChannelIdxByName(&device->RealOut, FrontRight);
    assert(lidx != -1 && ridx != -1);

    /* Apply binaural/crossfeed filter */
    bs2b_cross_feed(device->Bs2b, device->RealOut.Buffer[lidx],
                    device->RealOut.Buffer[ridx], SamplesToDo);
}

void aluSelectPostProcess(ALCdevice *device)
{
    if(device->HrtfHandle)
        device->PostProcess = ProcessHrtf;
    else if(device->AmbiDecoder)
        device->PostProcess = ProcessAmbiDec;
    else if(device->AmbiUp)
        device->PostProcess = ProcessAmbiUp;
    else if(device->Uhj_Encoder)
        device->PostProcess = ProcessUhj;
    else if(device->Bs2b)
        device->PostProcess = ProcessBs2b;
    else
        device->PostProcess = NULL;
}


/* Prepares the interpolator for a given rate (determined by increment).  A
 * result of AL_FALSE indicates that the filter output will completely cut
 * the input signal.
 *
 * With a bit of work, and a trade of memory for CPU cost, this could be
 * modified for use with an interpolated increment for buttery-smooth pitch
 * changes.
 */
void BsincPrepare(const ALuint increment, BsincState *state, const BSincTable *table)
{
    ALfloat sf = 0.0f;
    ALsizei si = BSINC_SCALE_COUNT-1;

    if(increment > FRACTIONONE)
    {
        sf = (ALfloat)FRACTIONONE / increment;
        sf = maxf(0.0f, (BSINC_SCALE_COUNT-1) * (sf-table->scaleBase) * table->scaleRange);
        si = float2int(sf);
        /* The interpolation factor is fit to this diagonally-symmetric curve
         * to reduce the transition ripple caused by interpolating different
         * scales of the sinc function.
         */
        sf = 1.0f - cosf(asinf(sf - si));
    }

    state->sf = sf;
    state->m = table->m[si];
    state->l = -((state->m/2) - 1);
    state->filter = table->Tab + table->filterOffset[si];
}


static bool CalcContextParams(ALCcontext *Context)
{
    ALlistener *Listener = Context->Listener;
    struct ALcontextProps *props;

    props = ATOMIC_EXCHANGE_PTR(&Context->Update, NULL, almemory_order_acq_rel);
    if(!props) return false;

    Listener->Params.MetersPerUnit = props->MetersPerUnit;

    Listener->Params.DopplerFactor = props->DopplerFactor;
    Listener->Params.SpeedOfSound = props->SpeedOfSound * props->DopplerVelocity;
    if(!OverrideReverbSpeedOfSound)
        Listener->Params.ReverbSpeedOfSound = Listener->Params.SpeedOfSound *
                                              Listener->Params.MetersPerUnit;

    Listener->Params.SourceDistanceModel = props->SourceDistanceModel;
    Listener->Params.DistanceModel = props->DistanceModel;

    ATOMIC_REPLACE_HEAD(struct ALcontextProps*, &Context->FreeContextProps, props);
    return true;
}

static bool CalcListenerParams(ALCcontext *Context)
{
    ALlistener *Listener = Context->Listener;
    ALfloat N[3], V[3], U[3], P[3];
    struct ALlistenerProps *props;
    aluVector vel;

    props = ATOMIC_EXCHANGE_PTR(&Listener->Update, NULL, almemory_order_acq_rel);
    if(!props) return false;

    /* AT then UP */
    N[0] = props->Forward[0];
    N[1] = props->Forward[1];
    N[2] = props->Forward[2];
    aluNormalize(N);
    V[0] = props->Up[0];
    V[1] = props->Up[1];
    V[2] = props->Up[2];
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

    P[0] = props->Position[0];
    P[1] = props->Position[1];
    P[2] = props->Position[2];
    aluMatrixfFloat3(P, 1.0, &Listener->Params.Matrix);
    aluMatrixfSetRow(&Listener->Params.Matrix, 3, -P[0], -P[1], -P[2], 1.0f);

    aluVectorSet(&vel, props->Velocity[0], props->Velocity[1], props->Velocity[2], 0.0f);
    Listener->Params.Velocity = aluMatrixfVector(&Listener->Params.Matrix, &vel);

    Listener->Params.Gain = props->Gain * Context->GainBoost;

    ATOMIC_REPLACE_HEAD(struct ALlistenerProps*, &Context->FreeListenerProps, props);
    return true;
}

static bool CalcEffectSlotParams(ALeffectslot *slot, ALCcontext *context, bool force)
{
    struct ALeffectslotProps *props;
    ALeffectState *state;

    props = ATOMIC_EXCHANGE_PTR(&slot->Update, NULL, almemory_order_acq_rel);
    if(!props && !force) return false;

    if(props)
    {
        slot->Params.Gain = props->Gain;
        slot->Params.AuxSendAuto = props->AuxSendAuto;
        slot->Params.EffectType = props->Type;
        slot->Params.EffectProps = props->Props;
        if(IsReverbEffect(props->Type))
        {
            slot->Params.RoomRolloff = props->Props.Reverb.RoomRolloffFactor;
            slot->Params.DecayTime = props->Props.Reverb.DecayTime;
            slot->Params.DecayLFRatio = props->Props.Reverb.DecayLFRatio;
            slot->Params.DecayHFRatio = props->Props.Reverb.DecayHFRatio;
            slot->Params.DecayHFLimit = props->Props.Reverb.DecayHFLimit;
            slot->Params.AirAbsorptionGainHF = props->Props.Reverb.AirAbsorptionGainHF;
        }
        else
        {
            slot->Params.RoomRolloff = 0.0f;
            slot->Params.DecayTime = 0.0f;
            slot->Params.DecayLFRatio = 0.0f;
            slot->Params.DecayHFRatio = 0.0f;
            slot->Params.DecayHFLimit = AL_FALSE;
            slot->Params.AirAbsorptionGainHF = 1.0f;
        }

        /* Swap effect states. No need to play with the ref counts since they
         * keep the same number of refs.
         */
        state = props->State;
        props->State = slot->Params.EffectState;
        slot->Params.EffectState = state;

        ATOMIC_REPLACE_HEAD(struct ALeffectslotProps*, &context->FreeEffectslotProps, props);
    }
    else
        state = slot->Params.EffectState;

    V(state,update)(context, slot, &slot->Params.EffectProps);
    return true;
}


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

static void CalcPanningAndFilters(ALvoice *voice, const ALfloat Azi, const ALfloat Elev,
                                  const ALfloat Distance, const ALfloat Spread,
                                  const ALfloat DryGain, const ALfloat DryGainHF,
                                  const ALfloat DryGainLF, const ALfloat *WetGain,
                                  const ALfloat *WetGainLF, const ALfloat *WetGainHF,
                                  ALeffectslot **SendSlots, const ALbuffer *Buffer,
                                  const struct ALvoiceProps *props, const ALlistener *Listener,
                                  const ALCdevice *Device)
{
    struct ChanMap StereoMap[2] = {
        { FrontLeft,  DEG2RAD(-30.0f), DEG2RAD(0.0f) },
        { FrontRight, DEG2RAD( 30.0f), DEG2RAD(0.0f) }
    };
    bool DirectChannels = props->DirectChannels;
    const ALsizei NumSends = Device->NumAuxSends;
    const ALuint Frequency = Device->Frequency;
    const struct ChanMap *chans = NULL;
    ALsizei num_channels = 0;
    bool isbformat = false;
    ALfloat downmix_gain = 1.0f;
    ALsizei c, i;

    switch(Buffer->FmtChannels)
    {
    case FmtMono:
        chans = MonoMap;
        num_channels = 1;
        /* Mono buffers are never played direct. */
        DirectChannels = false;
        break;

    case FmtStereo:
        /* Convert counter-clockwise to clockwise. */
        StereoMap[0].angle = -props->StereoPan[0];
        StereoMap[1].angle = -props->StereoPan[1];

        chans = StereoMap;
        num_channels = 2;
        downmix_gain = 1.0f / 2.0f;
        break;

    case FmtRear:
        chans = RearMap;
        num_channels = 2;
        downmix_gain = 1.0f / 2.0f;
        break;

    case FmtQuad:
        chans = QuadMap;
        num_channels = 4;
        downmix_gain = 1.0f / 4.0f;
        break;

    case FmtX51:
        chans = X51Map;
        num_channels = 6;
        /* NOTE: Excludes LFE. */
        downmix_gain = 1.0f / 5.0f;
        break;

    case FmtX61:
        chans = X61Map;
        num_channels = 7;
        /* NOTE: Excludes LFE. */
        downmix_gain = 1.0f / 6.0f;
        break;

    case FmtX71:
        chans = X71Map;
        num_channels = 8;
        /* NOTE: Excludes LFE. */
        downmix_gain = 1.0f / 7.0f;
        break;

    case FmtBFormat2D:
        num_channels = 3;
        isbformat = true;
        DirectChannels = false;
        break;

    case FmtBFormat3D:
        num_channels = 4;
        isbformat = true;
        DirectChannels = false;
        break;
    }

    for(c = 0;c < num_channels;c++)
    {
        memset(&voice->Direct.Params[c].Hrtf.Target, 0,
               sizeof(voice->Direct.Params[c].Hrtf.Target));
        ClearArray(voice->Direct.Params[c].Gains.Target);
    }
    for(i = 0;i < NumSends;i++)
    {
        for(c = 0;c < num_channels;c++)
            ClearArray(voice->Send[i].Params[c].Gains.Target);
    }

    voice->Flags &= ~(VOICE_HAS_HRTF | VOICE_HAS_NFC);
    if(isbformat)
    {
        /* Special handling for B-Format sources. */

        if(Distance > FLT_EPSILON)
        {
            /* Panning a B-Format sound toward some direction is easy. Just pan
             * the first (W) channel as a normal mono sound and silence the
             * others.
             */
            ALfloat coeffs[MAX_AMBI_COEFFS];

            if(Device->AvgSpeakerDist > 0.0f)
            {
                ALfloat mdist = Distance * Listener->Params.MetersPerUnit;
                ALfloat w0 = SPEEDOFSOUNDMETRESPERSEC /
                             (mdist * (ALfloat)Device->Frequency);
                ALfloat w1 = SPEEDOFSOUNDMETRESPERSEC /
                             (Device->AvgSpeakerDist * (ALfloat)Device->Frequency);
                /* Clamp w0 for really close distances, to prevent excessive
                 * bass.
                 */
                w0 = minf(w0, w1*4.0f);

                /* Only need to adjust the first channel of a B-Format source. */
                NfcFilterAdjust(&voice->Direct.Params[0].NFCtrlFilter, w0);

                for(i = 0;i < MAX_AMBI_ORDER+1;i++)
                    voice->Direct.ChannelsPerOrder[i] = Device->Dry.NumChannelsPerOrder[i];
                voice->Flags |= VOICE_HAS_NFC;
            }

            if(Device->Render_Mode == StereoPair)
                CalcAnglePairwiseCoeffs(Azi, Elev, Spread, coeffs);
            else
                CalcAngleCoeffs(Azi, Elev, Spread, coeffs);

            /* NOTE: W needs to be scaled by sqrt(2) due to FuMa normalization. */
            ComputeDryPanGains(&Device->Dry, coeffs, DryGain*1.414213562f,
                               voice->Direct.Params[0].Gains.Target);
            for(i = 0;i < NumSends;i++)
            {
                const ALeffectslot *Slot = SendSlots[i];
                if(Slot)
                    ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels,
                        coeffs, WetGain[i]*1.414213562f, voice->Send[i].Params[0].Gains.Target
                    );
            }
        }
        else
        {
            /* Local B-Format sources have their XYZ channels rotated according
             * to the orientation.
             */
            const ALfloat sqrt_2 = sqrtf(2.0f);
            const ALfloat sqrt_3 = sqrtf(3.0f);
            ALfloat N[3], V[3], U[3];
            aluMatrixf matrix;

            if(Device->AvgSpeakerDist > 0.0f)
            {
                /* NOTE: The NFCtrlFilters were created with a w0 of 0, which
                 * is what we want for FOA input. The first channel may have
                 * been previously re-adjusted if panned, so reset it.
                 */
                NfcFilterAdjust(&voice->Direct.Params[0].NFCtrlFilter, 0.0f);

                voice->Direct.ChannelsPerOrder[0] = 1;
                voice->Direct.ChannelsPerOrder[1] = mini(voice->Direct.Channels-1, 3);
                for(i = 2;i < MAX_AMBI_ORDER+1;i++)
                    voice->Direct.ChannelsPerOrder[i] = 0;
                voice->Flags |= VOICE_HAS_NFC;
            }

            /* AT then UP */
            N[0] = props->Orientation[0][0];
            N[1] = props->Orientation[0][1];
            N[2] = props->Orientation[0][2];
            aluNormalize(N);
            V[0] = props->Orientation[1][0];
            V[1] = props->Orientation[1][1];
            V[2] = props->Orientation[1][2];
            aluNormalize(V);
            if(!props->HeadRelative)
            {
                const aluMatrixf *lmatrix = &Listener->Params.Matrix;
                aluMatrixfFloat3(N, 0.0f, lmatrix);
                aluMatrixfFloat3(V, 0.0f, lmatrix);
            }
            /* Build and normalize right-vector */
            aluCrossproduct(N, V, U);
            aluNormalize(U);

            /* Build a rotate + conversion matrix (FuMa -> ACN+N3D). NOTE: This
             * matrix is transposed, for the inputs to align on the rows and
             * outputs on the columns.
             */
            aluMatrixfSet(&matrix,
                // ACN0         ACN1          ACN2          ACN3
                sqrt_2,         0.0f,         0.0f,         0.0f, // Ambi W
                  0.0f, -N[0]*sqrt_3,  N[1]*sqrt_3, -N[2]*sqrt_3, // Ambi X
                  0.0f,  U[0]*sqrt_3, -U[1]*sqrt_3,  U[2]*sqrt_3, // Ambi Y
                  0.0f, -V[0]*sqrt_3,  V[1]*sqrt_3, -V[2]*sqrt_3  // Ambi Z
            );

            voice->Direct.Buffer = Device->FOAOut.Buffer;
            voice->Direct.Channels = Device->FOAOut.NumChannels;
            for(c = 0;c < num_channels;c++)
                ComputeFirstOrderGains(&Device->FOAOut, matrix.m[c], DryGain,
                                       voice->Direct.Params[c].Gains.Target);
            for(i = 0;i < NumSends;i++)
            {
                const ALeffectslot *Slot = SendSlots[i];
                if(Slot)
                {
                    for(c = 0;c < num_channels;c++)
                        ComputeFirstOrderGainsBF(Slot->ChanMap, Slot->NumChannels,
                            matrix.m[c], WetGain[i], voice->Send[i].Params[c].Gains.Target
                        );
                }
            }
        }
    }
    else if(DirectChannels)
    {
        /* Direct source channels always play local. Skip the virtual channels
         * and write inputs to the matching real outputs.
         */
        voice->Direct.Buffer = Device->RealOut.Buffer;
        voice->Direct.Channels = Device->RealOut.NumChannels;

        for(c = 0;c < num_channels;c++)
        {
            int idx = GetChannelIdxByName(&Device->RealOut, chans[c].channel);
            if(idx != -1) voice->Direct.Params[c].Gains.Target[idx] = DryGain;
        }

        /* Auxiliary sends still use normal channel panning since they mix to
         * B-Format, which can't channel-match.
         */
        for(c = 0;c < num_channels;c++)
        {
            ALfloat coeffs[MAX_AMBI_COEFFS];
            CalcAngleCoeffs(chans[c].angle, chans[c].elevation, 0.0f, coeffs);

            for(i = 0;i < NumSends;i++)
            {
                const ALeffectslot *Slot = SendSlots[i];
                if(Slot)
                    ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels,
                        coeffs, WetGain[i], voice->Send[i].Params[c].Gains.Target
                    );
            }
        }
    }
    else if(Device->Render_Mode == HrtfRender)
    {
        /* Full HRTF rendering. Skip the virtual channels and render to the
         * real outputs.
         */
        voice->Direct.Buffer = Device->RealOut.Buffer;
        voice->Direct.Channels = Device->RealOut.NumChannels;

        if(Distance > FLT_EPSILON)
        {
            ALfloat coeffs[MAX_AMBI_COEFFS];

            /* Get the HRIR coefficients and delays just once, for the given
             * source direction.
             */
            GetHrtfCoeffs(Device->HrtfHandle, Elev, Azi, Spread,
                          voice->Direct.Params[0].Hrtf.Target.Coeffs,
                          voice->Direct.Params[0].Hrtf.Target.Delay);
            voice->Direct.Params[0].Hrtf.Target.Gain = DryGain * downmix_gain;

            /* Remaining channels use the same results as the first. */
            for(c = 1;c < num_channels;c++)
            {
                /* Skip LFE */
                if(chans[c].channel != LFE)
                    voice->Direct.Params[c].Hrtf.Target = voice->Direct.Params[0].Hrtf.Target;
            }

            /* Calculate the directional coefficients once, which apply to all
             * input channels of the source sends.
             */
            CalcAngleCoeffs(Azi, Elev, Spread, coeffs);

            for(i = 0;i < NumSends;i++)
            {
                const ALeffectslot *Slot = SendSlots[i];
                if(Slot)
                    for(c = 0;c < num_channels;c++)
                    {
                        /* Skip LFE */
                        if(chans[c].channel != LFE)
                            ComputePanningGainsBF(Slot->ChanMap,
                                Slot->NumChannels, coeffs, WetGain[i] * downmix_gain,
                                voice->Send[i].Params[c].Gains.Target
                            );
                    }
            }
        }
        else
        {
            /* Local sources on HRTF play with each channel panned to its
             * relative location around the listener, providing "virtual
             * speaker" responses.
             */
            for(c = 0;c < num_channels;c++)
            {
                ALfloat coeffs[MAX_AMBI_COEFFS];

                if(chans[c].channel == LFE)
                {
                    /* Skip LFE */
                    continue;
                }

                /* Get the HRIR coefficients and delays for this channel
                 * position.
                 */
                GetHrtfCoeffs(Device->HrtfHandle,
                    chans[c].elevation, chans[c].angle, Spread,
                    voice->Direct.Params[c].Hrtf.Target.Coeffs,
                    voice->Direct.Params[c].Hrtf.Target.Delay
                );
                voice->Direct.Params[c].Hrtf.Target.Gain = DryGain;

                /* Normal panning for auxiliary sends. */
                CalcAngleCoeffs(chans[c].angle, chans[c].elevation, Spread, coeffs);

                for(i = 0;i < NumSends;i++)
                {
                    const ALeffectslot *Slot = SendSlots[i];
                    if(Slot)
                        ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels,
                            coeffs, WetGain[i], voice->Send[i].Params[c].Gains.Target
                        );
                }
            }
        }

        voice->Flags |= VOICE_HAS_HRTF;
    }
    else
    {
        /* Non-HRTF rendering. Use normal panning to the output. */

        if(Distance > FLT_EPSILON)
        {
            ALfloat coeffs[MAX_AMBI_COEFFS];
            ALfloat w0 = 0.0f;

            /* Calculate NFC filter coefficient if needed. */
            if(Device->AvgSpeakerDist > 0.0f)
            {
                ALfloat mdist = Distance * Listener->Params.MetersPerUnit;
                ALfloat w1 = SPEEDOFSOUNDMETRESPERSEC /
                             (Device->AvgSpeakerDist * (ALfloat)Device->Frequency);
                w0 = SPEEDOFSOUNDMETRESPERSEC /
                     (mdist * (ALfloat)Device->Frequency);
                /* Clamp w0 for really close distances, to prevent excessive
                 * bass.
                 */
                w0 = minf(w0, w1*4.0f);

                /* Adjust NFC filters. */
                for(c = 0;c < num_channels;c++)
                    NfcFilterAdjust(&voice->Direct.Params[c].NFCtrlFilter, w0);

                for(i = 0;i < MAX_AMBI_ORDER+1;i++)
                    voice->Direct.ChannelsPerOrder[i] = Device->Dry.NumChannelsPerOrder[i];
                voice->Flags |= VOICE_HAS_NFC;
            }

            /* Calculate the directional coefficients once, which apply to all
             * input channels.
             */
            if(Device->Render_Mode == StereoPair)
                CalcAnglePairwiseCoeffs(Azi, Elev, Spread, coeffs);
            else
                CalcAngleCoeffs(Azi, Elev, Spread, coeffs);

            for(c = 0;c < num_channels;c++)
            {
                /* Special-case LFE */
                if(chans[c].channel == LFE)
                {
                    if(Device->Dry.Buffer == Device->RealOut.Buffer)
                    {
                        int idx = GetChannelIdxByName(&Device->RealOut, chans[c].channel);
                        if(idx != -1) voice->Direct.Params[c].Gains.Target[idx] = DryGain;
                    }
                    continue;
                }

                ComputeDryPanGains(&Device->Dry,
                    coeffs, DryGain * downmix_gain, voice->Direct.Params[c].Gains.Target
                );
            }

            for(i = 0;i < NumSends;i++)
            {
                const ALeffectslot *Slot = SendSlots[i];
                if(Slot)
                    for(c = 0;c < num_channels;c++)
                    {
                        /* Skip LFE */
                        if(chans[c].channel != LFE)
                            ComputePanningGainsBF(Slot->ChanMap,
                                Slot->NumChannels, coeffs, WetGain[i] * downmix_gain,
                                voice->Send[i].Params[c].Gains.Target
                            );
                    }
            }
        }
        else
        {
            ALfloat w0 = 0.0f;

            if(Device->AvgSpeakerDist > 0.0f)
            {
                /* If the source distance is 0, set w0 to w1 to act as a pass-
                 * through. We still want to pass the signal through the
                 * filters so they keep an appropriate history, in case the
                 * source moves away from the listener.
                 */
                w0 = SPEEDOFSOUNDMETRESPERSEC /
                     (Device->AvgSpeakerDist * (ALfloat)Device->Frequency);

                for(c = 0;c < num_channels;c++)
                    NfcFilterAdjust(&voice->Direct.Params[c].NFCtrlFilter, w0);

                for(i = 0;i < MAX_AMBI_ORDER+1;i++)
                    voice->Direct.ChannelsPerOrder[i] = Device->Dry.NumChannelsPerOrder[i];
                voice->Flags |= VOICE_HAS_NFC;
            }

            for(c = 0;c < num_channels;c++)
            {
                ALfloat coeffs[MAX_AMBI_COEFFS];

                /* Special-case LFE */
                if(chans[c].channel == LFE)
                {
                    if(Device->Dry.Buffer == Device->RealOut.Buffer)
                    {
                        int idx = GetChannelIdxByName(&Device->RealOut, chans[c].channel);
                        if(idx != -1) voice->Direct.Params[c].Gains.Target[idx] = DryGain;
                    }
                    continue;
                }

                if(Device->Render_Mode == StereoPair)
                    CalcAnglePairwiseCoeffs(chans[c].angle, chans[c].elevation, Spread, coeffs);
                else
                    CalcAngleCoeffs(chans[c].angle, chans[c].elevation, Spread, coeffs);
                ComputeDryPanGains(&Device->Dry,
                    coeffs, DryGain, voice->Direct.Params[c].Gains.Target
                );

                for(i = 0;i < NumSends;i++)
                {
                    const ALeffectslot *Slot = SendSlots[i];
                    if(Slot)
                        ComputePanningGainsBF(Slot->ChanMap, Slot->NumChannels,
                            coeffs, WetGain[i], voice->Send[i].Params[c].Gains.Target
                        );
                }
            }
        }
    }

    {
        ALfloat hfScale = props->Direct.HFReference / Frequency;
        ALfloat lfScale = props->Direct.LFReference / Frequency;
        ALfloat gainHF = maxf(DryGainHF, 0.001f); /* Limit -60dB */
        ALfloat gainLF = maxf(DryGainLF, 0.001f);

        voice->Direct.FilterType = AF_None;
        if(gainHF != 1.0f) voice->Direct.FilterType |= AF_LowPass;
        if(gainLF != 1.0f) voice->Direct.FilterType |= AF_HighPass;
        BiquadFilter_setParams(
            &voice->Direct.Params[0].LowPass, BiquadType_HighShelf,
            gainHF, hfScale, calc_rcpQ_from_slope(gainHF, 1.0f)
        );
        BiquadFilter_setParams(
            &voice->Direct.Params[0].HighPass, BiquadType_LowShelf,
            gainLF, lfScale, calc_rcpQ_from_slope(gainLF, 1.0f)
        );
        for(c = 1;c < num_channels;c++)
        {
            BiquadFilter_copyParams(&voice->Direct.Params[c].LowPass,
                                    &voice->Direct.Params[0].LowPass);
            BiquadFilter_copyParams(&voice->Direct.Params[c].HighPass,
                                    &voice->Direct.Params[0].HighPass);
        }
    }
    for(i = 0;i < NumSends;i++)
    {
        ALfloat hfScale = props->Send[i].HFReference / Frequency;
        ALfloat lfScale = props->Send[i].LFReference / Frequency;
        ALfloat gainHF = maxf(WetGainHF[i], 0.001f);
        ALfloat gainLF = maxf(WetGainLF[i], 0.001f);

        voice->Send[i].FilterType = AF_None;
        if(gainHF != 1.0f) voice->Send[i].FilterType |= AF_LowPass;
        if(gainLF != 1.0f) voice->Send[i].FilterType |= AF_HighPass;
        BiquadFilter_setParams(
            &voice->Send[i].Params[0].LowPass, BiquadType_HighShelf,
            gainHF, hfScale, calc_rcpQ_from_slope(gainHF, 1.0f)
        );
        BiquadFilter_setParams(
            &voice->Send[i].Params[0].HighPass, BiquadType_LowShelf,
            gainLF, lfScale, calc_rcpQ_from_slope(gainLF, 1.0f)
        );
        for(c = 1;c < num_channels;c++)
        {
            BiquadFilter_copyParams(&voice->Send[i].Params[c].LowPass,
                                    &voice->Send[i].Params[0].LowPass);
            BiquadFilter_copyParams(&voice->Send[i].Params[c].HighPass,
                                    &voice->Send[i].Params[0].HighPass);
        }
    }
}

static void CalcNonAttnSourceParams(ALvoice *voice, const struct ALvoiceProps *props, const ALbuffer *ALBuffer, const ALCcontext *ALContext)
{
    const ALCdevice *Device = ALContext->Device;
    const ALlistener *Listener = ALContext->Listener;
    ALfloat DryGain, DryGainHF, DryGainLF;
    ALfloat WetGain[MAX_SENDS];
    ALfloat WetGainHF[MAX_SENDS];
    ALfloat WetGainLF[MAX_SENDS];
    ALeffectslot *SendSlots[MAX_SENDS];
    ALfloat Pitch;
    ALsizei i;

    voice->Direct.Buffer = Device->Dry.Buffer;
    voice->Direct.Channels = Device->Dry.NumChannels;
    for(i = 0;i < Device->NumAuxSends;i++)
    {
        SendSlots[i] = props->Send[i].Slot;
        if(!SendSlots[i] && i == 0)
            SendSlots[i] = ALContext->DefaultSlot;
        if(!SendSlots[i] || SendSlots[i]->Params.EffectType == AL_EFFECT_NULL)
        {
            SendSlots[i] = NULL;
            voice->Send[i].Buffer = NULL;
            voice->Send[i].Channels = 0;
        }
        else
        {
            voice->Send[i].Buffer = SendSlots[i]->WetBuffer;
            voice->Send[i].Channels = SendSlots[i]->NumChannels;
        }
    }

    /* Calculate the stepping value */
    Pitch = (ALfloat)ALBuffer->Frequency/(ALfloat)Device->Frequency * props->Pitch;
    if(Pitch > (ALfloat)MAX_PITCH)
        voice->Step = MAX_PITCH<<FRACTIONBITS;
    else
        voice->Step = maxi(fastf2i(Pitch * FRACTIONONE), 1);
    if(props->Resampler == BSinc24Resampler)
        BsincPrepare(voice->Step, &voice->ResampleState.bsinc, &bsinc24);
    else if(props->Resampler == BSinc12Resampler)
        BsincPrepare(voice->Step, &voice->ResampleState.bsinc, &bsinc12);
    voice->Resampler = SelectResampler(props->Resampler);

    /* Calculate gains */
    DryGain  = clampf(props->Gain, props->MinGain, props->MaxGain);
    DryGain *= props->Direct.Gain * Listener->Params.Gain;
    DryGain  = minf(DryGain, GAIN_MIX_MAX);
    DryGainHF = props->Direct.GainHF;
    DryGainLF = props->Direct.GainLF;
    for(i = 0;i < Device->NumAuxSends;i++)
    {
        WetGain[i]  = clampf(props->Gain, props->MinGain, props->MaxGain);
        WetGain[i] *= props->Send[i].Gain * Listener->Params.Gain;
        WetGain[i]  = minf(WetGain[i], GAIN_MIX_MAX);
        WetGainHF[i] = props->Send[i].GainHF;
        WetGainLF[i] = props->Send[i].GainLF;
    }

    CalcPanningAndFilters(voice, 0.0f, 0.0f, 0.0f, 0.0f, DryGain, DryGainHF, DryGainLF, WetGain,
                          WetGainLF, WetGainHF, SendSlots, ALBuffer, props, Listener, Device);
}

static void CalcAttnSourceParams(ALvoice *voice, const struct ALvoiceProps *props, const ALbuffer *ALBuffer, const ALCcontext *ALContext)
{
    const ALCdevice *Device = ALContext->Device;
    const ALlistener *Listener = ALContext->Listener;
    const ALsizei NumSends = Device->NumAuxSends;
    aluVector Position, Velocity, Direction, SourceToListener;
    ALfloat Distance, ClampedDist, DopplerFactor;
    ALeffectslot *SendSlots[MAX_SENDS];
    ALfloat RoomRolloff[MAX_SENDS];
    ALfloat DecayDistance[MAX_SENDS];
    ALfloat DecayLFDistance[MAX_SENDS];
    ALfloat DecayHFDistance[MAX_SENDS];
    ALfloat DryGain, DryGainHF, DryGainLF;
    ALfloat WetGain[MAX_SENDS];
    ALfloat WetGainHF[MAX_SENDS];
    ALfloat WetGainLF[MAX_SENDS];
    bool directional;
    ALfloat ev, az;
    ALfloat spread;
    ALfloat Pitch;
    ALint i;

    /* Set mixing buffers and get send parameters. */
    voice->Direct.Buffer = Device->Dry.Buffer;
    voice->Direct.Channels = Device->Dry.NumChannels;
    for(i = 0;i < NumSends;i++)
    {
        SendSlots[i] = props->Send[i].Slot;
        if(!SendSlots[i] && i == 0)
            SendSlots[i] = ALContext->DefaultSlot;
        if(!SendSlots[i] || SendSlots[i]->Params.EffectType == AL_EFFECT_NULL)
        {
            SendSlots[i] = NULL;
            RoomRolloff[i] = 0.0f;
            DecayDistance[i] = 0.0f;
            DecayLFDistance[i] = 0.0f;
            DecayHFDistance[i] = 0.0f;
        }
        else if(SendSlots[i]->Params.AuxSendAuto)
        {
            RoomRolloff[i] = SendSlots[i]->Params.RoomRolloff + props->RoomRolloffFactor;
            /* Calculate the distances to where this effect's decay reaches
             * -60dB.
             */
            DecayDistance[i] = SendSlots[i]->Params.DecayTime *
                               Listener->Params.ReverbSpeedOfSound;
            DecayLFDistance[i] = DecayDistance[i] * SendSlots[i]->Params.DecayLFRatio;
            DecayHFDistance[i] = DecayDistance[i] * SendSlots[i]->Params.DecayHFRatio;
            if(SendSlots[i]->Params.DecayHFLimit)
            {
                ALfloat airAbsorption = SendSlots[i]->Params.AirAbsorptionGainHF;
                if(airAbsorption < 1.0f)
                {
                    /* Calculate the distance to where this effect's air
                     * absorption reaches -60dB, and limit the effect's HF
                     * decay distance (so it doesn't take any longer to decay
                     * than the air would allow).
                     */
                    ALfloat absorb_dist = log10f(REVERB_DECAY_GAIN) / log10f(airAbsorption);
                    DecayHFDistance[i] = minf(absorb_dist, DecayHFDistance[i]);
                }
            }
        }
        else
        {
            /* If the slot's auxiliary send auto is off, the data sent to the
             * effect slot is the same as the dry path, sans filter effects */
            RoomRolloff[i] = props->RolloffFactor;
            DecayDistance[i] = 0.0f;
            DecayLFDistance[i] = 0.0f;
            DecayHFDistance[i] = 0.0f;
        }

        if(!SendSlots[i])
        {
            voice->Send[i].Buffer = NULL;
            voice->Send[i].Channels = 0;
        }
        else
        {
            voice->Send[i].Buffer = SendSlots[i]->WetBuffer;
            voice->Send[i].Channels = SendSlots[i]->NumChannels;
        }
    }

    /* Transform source to listener space (convert to head relative) */
    aluVectorSet(&Position, props->Position[0], props->Position[1], props->Position[2], 1.0f);
    aluVectorSet(&Direction, props->Direction[0], props->Direction[1], props->Direction[2], 0.0f);
    aluVectorSet(&Velocity, props->Velocity[0], props->Velocity[1], props->Velocity[2], 0.0f);
    if(props->HeadRelative == AL_FALSE)
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

    directional = aluNormalize(Direction.v) > 0.0f;
    SourceToListener.v[0] = -Position.v[0];
    SourceToListener.v[1] = -Position.v[1];
    SourceToListener.v[2] = -Position.v[2];
    SourceToListener.v[3] = 0.0f;
    Distance = aluNormalize(SourceToListener.v);

    /* Initial source gain */
    DryGain = props->Gain;
    DryGainHF = 1.0f;
    DryGainLF = 1.0f;
    for(i = 0;i < NumSends;i++)
    {
        WetGain[i] = props->Gain;
        WetGainHF[i] = 1.0f;
        WetGainLF[i] = 1.0f;
    }

    /* Calculate distance attenuation */
    ClampedDist = Distance;

    switch(Listener->Params.SourceDistanceModel ?
           props->DistanceModel : Listener->Params.DistanceModel)
    {
        case InverseDistanceClamped:
            ClampedDist = clampf(ClampedDist, props->RefDistance, props->MaxDistance);
            if(props->MaxDistance < props->RefDistance)
                break;
            /*fall-through*/
        case InverseDistance:
            if(!(props->RefDistance > 0.0f))
                ClampedDist = props->RefDistance;
            else
            {
                ALfloat dist = lerp(props->RefDistance, ClampedDist, props->RolloffFactor);
                if(dist > 0.0f) DryGain *= props->RefDistance / dist;
                for(i = 0;i < NumSends;i++)
                {
                    dist = lerp(props->RefDistance, ClampedDist, RoomRolloff[i]);
                    if(dist > 0.0f) WetGain[i] *= props->RefDistance / dist;
                }
            }
            break;

        case LinearDistanceClamped:
            ClampedDist = clampf(ClampedDist, props->RefDistance, props->MaxDistance);
            if(props->MaxDistance < props->RefDistance)
                break;
            /*fall-through*/
        case LinearDistance:
            if(!(props->MaxDistance != props->RefDistance))
                ClampedDist = props->RefDistance;
            else
            {
                ALfloat attn = props->RolloffFactor * (ClampedDist-props->RefDistance) /
                               (props->MaxDistance-props->RefDistance);
                DryGain *= maxf(1.0f - attn, 0.0f);
                for(i = 0;i < NumSends;i++)
                {
                    attn = RoomRolloff[i] * (ClampedDist-props->RefDistance) /
                           (props->MaxDistance-props->RefDistance);
                    WetGain[i] *= maxf(1.0f - attn, 0.0f);
                }
            }
            break;

        case ExponentDistanceClamped:
            ClampedDist = clampf(ClampedDist, props->RefDistance, props->MaxDistance);
            if(props->MaxDistance < props->RefDistance)
                break;
            /*fall-through*/
        case ExponentDistance:
            if(!(ClampedDist > 0.0f && props->RefDistance > 0.0f))
                ClampedDist = props->RefDistance;
            else
            {
                DryGain *= powf(ClampedDist/props->RefDistance, -props->RolloffFactor);
                for(i = 0;i < NumSends;i++)
                    WetGain[i] *= powf(ClampedDist/props->RefDistance, -RoomRolloff[i]);
            }
            break;

        case DisableDistance:
            ClampedDist = props->RefDistance;
            break;
    }

    /* Calculate directional soundcones */
    if(directional && props->InnerAngle < 360.0f)
    {
        ALfloat ConeVolume;
        ALfloat ConeHF;
        ALfloat Angle;

        Angle = acosf(aluDotproduct(&Direction, &SourceToListener));
        Angle = RAD2DEG(Angle * ConeScale * 2.0f);
        if(!(Angle > props->InnerAngle))
        {
            ConeVolume = 1.0f;
            ConeHF = 1.0f;
        }
        else if(Angle < props->OuterAngle)
        {
            ALfloat scale = (            Angle-props->InnerAngle) /
                            (props->OuterAngle-props->InnerAngle);
            ConeVolume = lerp(1.0f, props->OuterGain, scale);
            ConeHF = lerp(1.0f, props->OuterGainHF, scale);
        }
        else
        {
            ConeVolume = props->OuterGain;
            ConeHF = props->OuterGainHF;
        }

        DryGain *= ConeVolume;
        if(props->DryGainHFAuto)
            DryGainHF *= ConeHF;
        if(props->WetGainAuto)
        {
            for(i = 0;i < NumSends;i++)
                WetGain[i] *= ConeVolume;
        }
        if(props->WetGainHFAuto)
        {
            for(i = 0;i < NumSends;i++)
                WetGainHF[i] *= ConeHF;
        }
    }

    /* Apply gain and frequency filters */
    DryGain  = clampf(DryGain, props->MinGain, props->MaxGain);
    DryGain  = minf(DryGain*props->Direct.Gain*Listener->Params.Gain, GAIN_MIX_MAX);
    DryGainHF *= props->Direct.GainHF;
    DryGainLF *= props->Direct.GainLF;
    for(i = 0;i < NumSends;i++)
    {
        WetGain[i]  = clampf(WetGain[i], props->MinGain, props->MaxGain);
        WetGain[i]  = minf(WetGain[i]*props->Send[i].Gain*Listener->Params.Gain, GAIN_MIX_MAX);
        WetGainHF[i] *= props->Send[i].GainHF;
        WetGainLF[i] *= props->Send[i].GainLF;
    }

    /* Distance-based air absorption and initial send decay. */
    if(ClampedDist > props->RefDistance && props->RolloffFactor > 0.0f)
    {
        ALfloat meters_base = (ClampedDist-props->RefDistance) * props->RolloffFactor *
                              Listener->Params.MetersPerUnit;
        if(props->AirAbsorptionFactor > 0.0f)
        {
            ALfloat hfattn = powf(AIRABSORBGAINHF, meters_base * props->AirAbsorptionFactor);
            DryGainHF *= hfattn;
            for(i = 0;i < NumSends;i++)
                WetGainHF[i] *= hfattn;
        }

        if(props->WetGainAuto)
        {
            /* Apply a decay-time transformation to the wet path, based on the
             * source distance in meters. The initial decay of the reverb
             * effect is calculated and applied to the wet path.
             */
            for(i = 0;i < NumSends;i++)
            {
                ALfloat gain, gainhf, gainlf;

                if(!(DecayDistance[i] > 0.0f))
                    continue;

                gain = powf(REVERB_DECAY_GAIN, meters_base/DecayDistance[i]);
                WetGain[i] *= gain;
                /* Yes, the wet path's air absorption is applied with
                 * WetGainAuto on, rather than WetGainHFAuto.
                 */
                if(gain > 0.0f)
                {
                    gainhf = powf(REVERB_DECAY_GAIN, meters_base/DecayHFDistance[i]);
                    WetGainHF[i] *= minf(gainhf / gain, 1.0f);
                    gainlf = powf(REVERB_DECAY_GAIN, meters_base/DecayLFDistance[i]);
                    WetGainLF[i] *= minf(gainlf / gain, 1.0f);
                }
            }
        }
    }


    /* Initial source pitch */
    Pitch = props->Pitch;

    /* Calculate velocity-based doppler effect */
    DopplerFactor = props->DopplerFactor * Listener->Params.DopplerFactor;
    if(DopplerFactor > 0.0f)
    {
        const aluVector *lvelocity = &Listener->Params.Velocity;
        const ALfloat SpeedOfSound = Listener->Params.SpeedOfSound;
        ALfloat vss, vls;

        vss = aluDotproduct(&Velocity, &SourceToListener) * DopplerFactor;
        vls = aluDotproduct(lvelocity, &SourceToListener) * DopplerFactor;

        if(!(vls < SpeedOfSound))
        {
            /* Listener moving away from the source at the speed of sound.
             * Sound waves can't catch it.
             */
            Pitch = 0.0f;
        }
        else if(!(vss < SpeedOfSound))
        {
            /* Source moving toward the listener at the speed of sound. Sound
             * waves bunch up to extreme frequencies.
             */
            Pitch = HUGE_VALF;
        }
        else
        {
            /* Source and listener movement is nominal. Calculate the proper
             * doppler shift.
             */
            Pitch *= (SpeedOfSound-vls) / (SpeedOfSound-vss);
        }
    }

    /* Adjust pitch based on the buffer and output frequencies, and calculate
     * fixed-point stepping value.
     */
    Pitch *= (ALfloat)ALBuffer->Frequency/(ALfloat)Device->Frequency;
    if(Pitch > (ALfloat)MAX_PITCH)
        voice->Step = MAX_PITCH<<FRACTIONBITS;
    else
        voice->Step = maxi(fastf2i(Pitch * FRACTIONONE), 1);
    if(props->Resampler == BSinc24Resampler)
        BsincPrepare(voice->Step, &voice->ResampleState.bsinc, &bsinc24);
    else if(props->Resampler == BSinc12Resampler)
        BsincPrepare(voice->Step, &voice->ResampleState.bsinc, &bsinc12);
    voice->Resampler = SelectResampler(props->Resampler);

    if(Distance > 0.0f)
    {
        /* Clamp Y, in case rounding errors caused it to end up outside of
         * -1...+1.
         */
        ev = asinf(clampf(-SourceToListener.v[1], -1.0f, 1.0f));
        /* Double negation on Z cancels out; negate once for changing source-
         * to-listener to listener-to-source, and again for right-handed coords
         * with -Z in front.
         */
        az = atan2f(-SourceToListener.v[0], SourceToListener.v[2]*ZScale);
    }
    else
        ev = az = 0.0f;

    if(props->Radius > Distance)
        spread = F_TAU - Distance/props->Radius*F_PI;
    else if(Distance > 0.0f)
        spread = asinf(props->Radius / Distance) * 2.0f;
    else
        spread = 0.0f;

    CalcPanningAndFilters(voice, az, ev, Distance, spread, DryGain, DryGainHF, DryGainLF, WetGain,
                          WetGainLF, WetGainHF, SendSlots, ALBuffer, props, Listener, Device);
}

static void CalcSourceParams(ALvoice *voice, ALCcontext *context, bool force)
{
    ALbufferlistitem *BufferListItem;
    struct ALvoiceProps *props;

    props = ATOMIC_EXCHANGE_PTR(&voice->Update, NULL, almemory_order_acq_rel);
    if(!props && !force) return;

    if(props)
    {
        memcpy(voice->Props, props,
            FAM_SIZE(struct ALvoiceProps, Send, context->Device->NumAuxSends)
        );

        ATOMIC_REPLACE_HEAD(struct ALvoiceProps*, &context->FreeVoiceProps, props);
    }
    props = voice->Props;

    BufferListItem = ATOMIC_LOAD(&voice->current_buffer, almemory_order_relaxed);
    while(BufferListItem != NULL)
    {
        const ALbuffer *buffer = NULL;
        ALsizei i = 0;
        while(!buffer && i < BufferListItem->num_buffers)
            buffer = BufferListItem->buffers[i];
        if(LIKELY(buffer))
        {
            if(props->SpatializeMode == SpatializeOn ||
               (props->SpatializeMode == SpatializeAuto && buffer->FmtChannels == FmtMono))
                CalcAttnSourceParams(voice, props, buffer, context);
            else
                CalcNonAttnSourceParams(voice, props, buffer, context);
            break;
        }
        BufferListItem = ATOMIC_LOAD(&BufferListItem->next, almemory_order_acquire);
    }
}


static void ProcessParamUpdates(ALCcontext *ctx, const struct ALeffectslotArray *slots)
{
    ALvoice **voice, **voice_end;
    ALsource *source;
    ALsizei i;

    IncrementRef(&ctx->UpdateCount);
    if(!ATOMIC_LOAD(&ctx->HoldUpdates, almemory_order_acquire))
    {
        bool cforce = CalcContextParams(ctx);
        bool force = CalcListenerParams(ctx) | cforce;
        for(i = 0;i < slots->count;i++)
            force |= CalcEffectSlotParams(slots->slot[i], ctx, cforce);

        voice = ctx->Voices;
        voice_end = voice + ctx->VoiceCount;
        for(;voice != voice_end;++voice)
        {
            source = ATOMIC_LOAD(&(*voice)->Source, almemory_order_acquire);
            if(source) CalcSourceParams(*voice, ctx, force);
        }
    }
    IncrementRef(&ctx->UpdateCount);
}


static void ApplyStablizer(FrontStablizer *Stablizer, ALfloat (*restrict Buffer)[BUFFERSIZE],
                           int lidx, int ridx, int cidx, ALsizei SamplesToDo,
                           ALsizei NumChannels)
{
    ALfloat (*restrict lsplit)[BUFFERSIZE] = ASSUME_ALIGNED(Stablizer->LSplit, 16);
    ALfloat (*restrict rsplit)[BUFFERSIZE] = ASSUME_ALIGNED(Stablizer->RSplit, 16);
    ALsizei i;

    /* Apply an all-pass to all channels, except the front-left and front-
     * right, so they maintain the same relative phase.
     */
    for(i = 0;i < NumChannels;i++)
    {
        if(i == lidx || i == ridx)
            continue;
        splitterap_process(&Stablizer->APFilter[i], Buffer[i], SamplesToDo);
    }

    bandsplit_process(&Stablizer->LFilter, lsplit[1], lsplit[0], Buffer[lidx], SamplesToDo);
    bandsplit_process(&Stablizer->RFilter, rsplit[1], rsplit[0], Buffer[ridx], SamplesToDo);

    for(i = 0;i < SamplesToDo;i++)
    {
        ALfloat lfsum, hfsum;
        ALfloat m, s, c;

        lfsum = lsplit[0][i] + rsplit[0][i];
        hfsum = lsplit[1][i] + rsplit[1][i];
        s = lsplit[0][i] + lsplit[1][i] - rsplit[0][i] - rsplit[1][i];

        /* This pans the separate low- and high-frequency sums between being on
         * the center channel and the left/right channels. The low-frequency
         * sum is 1/3rd toward center (2/3rds on left/right) and the high-
         * frequency sum is 1/4th toward center (3/4ths on left/right). These
         * values can be tweaked.
         */
        m = lfsum*cosf(1.0f/3.0f * F_PI_2) + hfsum*cosf(1.0f/4.0f * F_PI_2);
        c = lfsum*sinf(1.0f/3.0f * F_PI_2) + hfsum*sinf(1.0f/4.0f * F_PI_2);

        /* The generated center channel signal adds to the existing signal,
         * while the modified left and right channels replace.
         */
        Buffer[lidx][i] = (m + s) * 0.5f;
        Buffer[ridx][i] = (m - s) * 0.5f;
        Buffer[cidx][i] += c * 0.5f;
    }
}

static void ApplyDistanceComp(ALfloat (*restrict Samples)[BUFFERSIZE], DistanceComp *distcomp,
                              ALfloat *restrict Values, ALsizei SamplesToDo, ALsizei numchans)
{
    ALsizei i, c;

    Values = ASSUME_ALIGNED(Values, 16);
    for(c = 0;c < numchans;c++)
    {
        ALfloat *restrict inout = ASSUME_ALIGNED(Samples[c], 16);
        const ALfloat gain = distcomp[c].Gain;
        const ALsizei base = distcomp[c].Length;
        ALfloat *restrict distbuf = ASSUME_ALIGNED(distcomp[c].Buffer, 16);

        if(base == 0)
        {
            if(gain < 1.0f)
            {
                for(i = 0;i < SamplesToDo;i++)
                    inout[i] *= gain;
            }
            continue;
        }

        if(SamplesToDo >= base)
        {
            for(i = 0;i < base;i++)
                Values[i] = distbuf[i];
            for(;i < SamplesToDo;i++)
                Values[i] = inout[i-base];
            memcpy(distbuf, &inout[SamplesToDo-base], base*sizeof(ALfloat));
        }
        else
        {
            for(i = 0;i < SamplesToDo;i++)
                Values[i] = distbuf[i];
            memmove(distbuf, distbuf+SamplesToDo, (base-SamplesToDo)*sizeof(ALfloat));
            memcpy(distbuf+base-SamplesToDo, inout, SamplesToDo*sizeof(ALfloat));
        }
        for(i = 0;i < SamplesToDo;i++)
            inout[i] = Values[i]*gain;
    }
}

static void ApplyDither(ALfloat (*restrict Samples)[BUFFERSIZE], ALuint *dither_seed,
                        const ALfloat quant_scale, const ALsizei SamplesToDo,
                        const ALsizei numchans)
{
    const ALfloat invscale = 1.0f / quant_scale;
    ALuint seed = *dither_seed;
    ALsizei c, i;

    /* Dithering. Step 1, generate whitenoise (uniform distribution of random
     * values between -1 and +1). Step 2 is to add the noise to the samples,
     * before rounding and after scaling up to the desired quantization depth.
     */
    for(c = 0;c < numchans;c++)
    {
        ALfloat *restrict samples = Samples[c];
        for(i = 0;i < SamplesToDo;i++)
        {
            ALfloat val = samples[i] * quant_scale;
            ALuint rng0 = dither_rng(&seed);
            ALuint rng1 = dither_rng(&seed);
            val += (ALfloat)(rng0*(1.0/UINT_MAX) - rng1*(1.0/UINT_MAX));
            samples[i] = fastf2i(val) * invscale;
        }
    }
    *dither_seed = seed;
}


static inline ALfloat Conv_ALfloat(ALfloat val)
{ return val; }
static inline ALint Conv_ALint(ALfloat val)
{
    /* Floats have a 23-bit mantissa. A bit of the exponent helps out along
     * with the sign bit, giving 25 bits. So [-16777216, +16777216] is the max
     * integer range normalized floats can be converted to before losing
     * precision.
     */
    return fastf2i(clampf(val*16777216.0f, -16777216.0f, 16777215.0f))<<7;
}
static inline ALshort Conv_ALshort(ALfloat val)
{ return fastf2i(clampf(val*32768.0f, -32768.0f, 32767.0f)); }
static inline ALbyte Conv_ALbyte(ALfloat val)
{ return fastf2i(clampf(val*128.0f, -128.0f, 127.0f)); }

/* Define unsigned output variations. */
#define DECL_TEMPLATE(T, func, O)                             \
static inline T Conv_##T(ALfloat val) { return func(val)+O; }

DECL_TEMPLATE(ALubyte, Conv_ALbyte, 128)
DECL_TEMPLATE(ALushort, Conv_ALshort, 32768)
DECL_TEMPLATE(ALuint, Conv_ALint, 2147483648u)

#undef DECL_TEMPLATE

#define DECL_TEMPLATE(T, A)                                                   \
static void Write##A(const ALfloat (*restrict InBuffer)[BUFFERSIZE],          \
                     ALvoid *OutBuffer, ALsizei Offset, ALsizei SamplesToDo,  \
                     ALsizei numchans)                                        \
{                                                                             \
    ALsizei i, j;                                                             \
    for(j = 0;j < numchans;j++)                                               \
    {                                                                         \
        const ALfloat *restrict in = ASSUME_ALIGNED(InBuffer[j], 16);         \
        T *restrict out = (T*)OutBuffer + Offset*numchans + j;                \
                                                                              \
        for(i = 0;i < SamplesToDo;i++)                                        \
            out[i*numchans] = Conv_##T(in[i]);                                \
    }                                                                         \
}

DECL_TEMPLATE(ALfloat, F32)
DECL_TEMPLATE(ALuint, UI32)
DECL_TEMPLATE(ALint, I32)
DECL_TEMPLATE(ALushort, UI16)
DECL_TEMPLATE(ALshort, I16)
DECL_TEMPLATE(ALubyte, UI8)
DECL_TEMPLATE(ALbyte, I8)

#undef DECL_TEMPLATE


void aluMixData(ALCdevice *device, ALvoid *OutBuffer, ALsizei NumSamples)
{
    ALsizei SamplesToDo;
    ALsizei SamplesDone;
    ALCcontext *ctx;
    ALsizei i, c;

    START_MIXER_MODE();
    for(SamplesDone = 0;SamplesDone < NumSamples;)
    {
        SamplesToDo = mini(NumSamples-SamplesDone, BUFFERSIZE);
        for(c = 0;c < device->Dry.NumChannels;c++)
            memset(device->Dry.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));
        if(device->Dry.Buffer != device->FOAOut.Buffer)
            for(c = 0;c < device->FOAOut.NumChannels;c++)
                memset(device->FOAOut.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));
        if(device->Dry.Buffer != device->RealOut.Buffer)
            for(c = 0;c < device->RealOut.NumChannels;c++)
                memset(device->RealOut.Buffer[c], 0, SamplesToDo*sizeof(ALfloat));

        IncrementRef(&device->MixCount);

        ctx = ATOMIC_LOAD(&device->ContextList, almemory_order_acquire);
        while(ctx)
        {
            const struct ALeffectslotArray *auxslots;

            auxslots = ATOMIC_LOAD(&ctx->ActiveAuxSlots, almemory_order_acquire);
            ProcessParamUpdates(ctx, auxslots);

            for(i = 0;i < auxslots->count;i++)
            {
                ALeffectslot *slot = auxslots->slot[i];
                for(c = 0;c < slot->NumChannels;c++)
                    memset(slot->WetBuffer[c], 0, SamplesToDo*sizeof(ALfloat));
            }

            /* source processing */
            for(i = 0;i < ctx->VoiceCount;i++)
            {
                ALvoice *voice = ctx->Voices[i];
                ALsource *source = ATOMIC_LOAD(&voice->Source, almemory_order_acquire);
                if(source && ATOMIC_LOAD(&voice->Playing, almemory_order_relaxed) &&
                   voice->Step > 0)
                {
                    if(!MixSource(voice, source->id, ctx, SamplesToDo))
                    {
                        ATOMIC_STORE(&voice->Source, NULL, almemory_order_relaxed);
                        ATOMIC_STORE(&voice->Playing, false, almemory_order_release);
                        SendSourceStoppedEvent(ctx, source->id);
                    }
                }
            }

            /* effect slot processing */
            for(i = 0;i < auxslots->count;i++)
            {
                const ALeffectslot *slot = auxslots->slot[i];
                ALeffectState *state = slot->Params.EffectState;
                V(state,process)(SamplesToDo, slot->WetBuffer, state->OutBuffer,
                                 state->OutChannels);
            }

            ctx = ATOMIC_LOAD(&ctx->next, almemory_order_relaxed);
        }

        /* Increment the clock time. Every second's worth of samples is
         * converted and added to clock base so that large sample counts don't
         * overflow during conversion. This also guarantees an exact, stable
         * conversion. */
        device->SamplesDone += SamplesToDo;
        device->ClockBase += (device->SamplesDone/device->Frequency) * DEVICE_CLOCK_RES;
        device->SamplesDone %= device->Frequency;
        IncrementRef(&device->MixCount);

        /* Apply post-process for finalizing the Dry mix to the RealOut
         * (Ambisonic decode, UHJ encode, etc).
         */
        if(LIKELY(device->PostProcess))
            device->PostProcess(device, SamplesToDo);

        if(device->Stablizer)
        {
            int lidx = GetChannelIdxByName(&device->RealOut, FrontLeft);
            int ridx = GetChannelIdxByName(&device->RealOut, FrontRight);
            int cidx = GetChannelIdxByName(&device->RealOut, FrontCenter);
            assert(lidx >= 0 && ridx >= 0 && cidx >= 0);

            ApplyStablizer(device->Stablizer, device->RealOut.Buffer, lidx, ridx, cidx,
                           SamplesToDo, device->RealOut.NumChannels);
        }

        ApplyDistanceComp(device->RealOut.Buffer, device->ChannelDelay, device->TempBuffer[0],
                          SamplesToDo, device->RealOut.NumChannels);

        if(device->Limiter)
            ApplyCompression(device->Limiter, device->RealOut.NumChannels, SamplesToDo,
                             device->RealOut.Buffer);

        if(device->DitherDepth > 0.0f)
            ApplyDither(device->RealOut.Buffer, &device->DitherSeed, device->DitherDepth,
                        SamplesToDo, device->RealOut.NumChannels);

        if(LIKELY(OutBuffer))
        {
            ALfloat (*Buffer)[BUFFERSIZE] = device->RealOut.Buffer;
            ALsizei Channels = device->RealOut.NumChannels;

            switch(device->FmtType)
            {
                case DevFmtByte:
                    WriteI8(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtUByte:
                    WriteUI8(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtShort:
                    WriteI16(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtUShort:
                    WriteUI16(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtInt:
                    WriteI32(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtUInt:
                    WriteUI32(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
                case DevFmtFloat:
                    WriteF32(Buffer, OutBuffer, SamplesDone, SamplesToDo, Channels);
                    break;
            }
        }

        SamplesDone += SamplesToDo;
    }
    END_MIXER_MODE();
}


void aluHandleDisconnect(ALCdevice *device, const char *msg, ...)
{
    ALCcontext *ctx;
    AsyncEvent evt;
    va_list args;
    int msglen;

    if(!ATOMIC_EXCHANGE(&device->Connected, AL_FALSE, almemory_order_acq_rel))
        return;

    evt.EnumType = EventType_Disconnected;
    evt.Type = AL_EVENT_TYPE_DISCONNECTED_SOFT;
    evt.ObjectId = 0;
    evt.Param = 0;

    va_start(args, msg);
    msglen = vsnprintf(evt.Message, sizeof(evt.Message), msg, args);
    va_end(args);

    if(msglen < 0 || (size_t)msglen >= sizeof(evt.Message))
    {
        evt.Message[sizeof(evt.Message)-1] = 0;
        msglen = (int)strlen(evt.Message);
    }
    if(msglen > 0)
        msg = evt.Message;
    else
    {
        msg = "<internal error constructing message>";
        msglen = (int)strlen(msg);
    }

    ctx = ATOMIC_LOAD_SEQ(&device->ContextList);
    while(ctx)
    {
        ALbitfieldSOFT enabledevt = ATOMIC_LOAD(&ctx->EnabledEvts, almemory_order_acquire);
        ALsizei i;

        if((enabledevt&EventType_Disconnected) &&
           ll_ringbuffer_write(ctx->AsyncEvents, (const char*)&evt, 1) == 1)
            alsem_post(&ctx->EventSem);

        for(i = 0;i < ctx->VoiceCount;i++)
        {
            ALvoice *voice = ctx->Voices[i];
            ALsource *source;

            source = ATOMIC_EXCHANGE_PTR(&voice->Source, NULL, almemory_order_relaxed);
            if(source && ATOMIC_LOAD(&voice->Playing, almemory_order_relaxed))
            {
                /* If the source's voice was playing, it's now effectively
                 * stopped (the source state will be updated the next time it's
                 * checked).
                 */
                SendSourceStoppedEvent(ctx, source->id);
            }
            ATOMIC_STORE(&voice->Playing, false, almemory_order_release);
        }

        ctx = ATOMIC_LOAD(&ctx->next, almemory_order_relaxed);
    }
}
