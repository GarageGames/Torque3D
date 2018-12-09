/**
 * OpenAL cross platform audio library
 * Copyright (C) 2009 by Chris Robinson.
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

#include "alMain.h"
#include "alFilter.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"
#include "filters/defs.h"


typedef struct ALechoState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat *SampleBuffer;
    ALsizei BufferLength;

    // The echo is two tap. The delay is the number of samples from before the
    // current offset
    struct {
        ALsizei delay;
    } Tap[2];
    ALsizei Offset;

    /* The panning gains for the two taps */
    struct {
        ALfloat Current[MAX_OUTPUT_CHANNELS];
        ALfloat Target[MAX_OUTPUT_CHANNELS];
    } Gains[2];

    ALfloat FeedGain;

    BiquadFilter Filter;
} ALechoState;

static ALvoid ALechoState_Destruct(ALechoState *state);
static ALboolean ALechoState_deviceUpdate(ALechoState *state, ALCdevice *Device);
static ALvoid ALechoState_update(ALechoState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALechoState_process(ALechoState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALechoState)

DEFINE_ALEFFECTSTATE_VTABLE(ALechoState);


static void ALechoState_Construct(ALechoState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALechoState, ALeffectState, state);

    state->BufferLength = 0;
    state->SampleBuffer = NULL;

    state->Tap[0].delay = 0;
    state->Tap[1].delay = 0;
    state->Offset = 0;

    BiquadFilter_clear(&state->Filter);
}

static ALvoid ALechoState_Destruct(ALechoState *state)
{
    al_free(state->SampleBuffer);
    state->SampleBuffer = NULL;
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALechoState_deviceUpdate(ALechoState *state, ALCdevice *Device)
{
    ALsizei maxlen;

    // Use the next power of 2 for the buffer length, so the tap offsets can be
    // wrapped using a mask instead of a modulo
    maxlen = float2int(AL_ECHO_MAX_DELAY*Device->Frequency + 0.5f) +
             float2int(AL_ECHO_MAX_LRDELAY*Device->Frequency + 0.5f);
    maxlen = NextPowerOf2(maxlen);
    if(maxlen <= 0) return AL_FALSE;

    if(maxlen != state->BufferLength)
    {
        void *temp = al_calloc(16, maxlen * sizeof(ALfloat));
        if(!temp) return AL_FALSE;

        al_free(state->SampleBuffer);
        state->SampleBuffer = temp;
        state->BufferLength = maxlen;
    }

    memset(state->SampleBuffer, 0, state->BufferLength*sizeof(ALfloat));
    memset(state->Gains, 0, sizeof(state->Gains));

    return AL_TRUE;
}

static ALvoid ALechoState_update(ALechoState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props)
{
    const ALCdevice *device = context->Device;
    ALuint frequency = device->Frequency;
    ALfloat coeffs[MAX_AMBI_COEFFS];
    ALfloat gainhf, lrpan, spread;

    state->Tap[0].delay = maxi(float2int(props->Echo.Delay*frequency + 0.5f), 1);
    state->Tap[1].delay = float2int(props->Echo.LRDelay*frequency + 0.5f);
    state->Tap[1].delay += state->Tap[0].delay;

    spread = props->Echo.Spread;
    if(spread < 0.0f) lrpan = -1.0f;
    else lrpan = 1.0f;
    /* Convert echo spread (where 0 = omni, +/-1 = directional) to coverage
     * spread (where 0 = point, tau = omni).
     */
    spread = asinf(1.0f - fabsf(spread))*4.0f;

    state->FeedGain = props->Echo.Feedback;

    gainhf = maxf(1.0f - props->Echo.Damping, 0.0625f); /* Limit -24dB */
    BiquadFilter_setParams(&state->Filter, BiquadType_HighShelf,
        gainhf, LOWPASSFREQREF/frequency, calc_rcpQ_from_slope(gainhf, 1.0f)
    );

    /* First tap panning */
    CalcAngleCoeffs(-F_PI_2*lrpan, 0.0f, spread, coeffs);
    ComputeDryPanGains(&device->Dry, coeffs, slot->Params.Gain, state->Gains[0].Target);

    /* Second tap panning */
    CalcAngleCoeffs( F_PI_2*lrpan, 0.0f, spread, coeffs);
    ComputeDryPanGains(&device->Dry, coeffs, slot->Params.Gain, state->Gains[1].Target);
}

static ALvoid ALechoState_process(ALechoState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels)
{
    const ALsizei mask = state->BufferLength-1;
    const ALsizei tap1 = state->Tap[0].delay;
    const ALsizei tap2 = state->Tap[1].delay;
    ALfloat *restrict delaybuf = state->SampleBuffer;
    ALsizei offset = state->Offset;
    ALfloat z1, z2, in, out;
    ALsizei base;
    ALsizei c, i;

    z1 = state->Filter.z1;
    z2 = state->Filter.z2;
    for(base = 0;base < SamplesToDo;)
    {
        alignas(16) ALfloat temps[2][128];
        ALsizei td = mini(128, SamplesToDo-base);

        for(i = 0;i < td;i++)
        {
            /* Feed the delay buffer's input first. */
            delaybuf[offset&mask] = SamplesIn[0][i+base];

            /* First tap */
            temps[0][i] = delaybuf[(offset-tap1) & mask];
            /* Second tap */
            temps[1][i] = delaybuf[(offset-tap2) & mask];

            /* Apply damping to the second tap, then add it to the buffer with
             * feedback attenuation.
             */
            in = temps[1][i];
            out = in*state->Filter.b0 + z1;
            z1 = in*state->Filter.b1 - out*state->Filter.a1 + z2;
            z2 = in*state->Filter.b2 - out*state->Filter.a2;

            delaybuf[offset&mask] += out * state->FeedGain;
            offset++;
        }

        for(c = 0;c < 2;c++)
            MixSamples(temps[c], NumChannels, SamplesOut, state->Gains[c].Current,
                       state->Gains[c].Target, SamplesToDo-base, base, td);

        base += td;
    }
    state->Filter.z1 = z1;
    state->Filter.z2 = z2;

    state->Offset = offset;
}


typedef struct EchoStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} EchoStateFactory;

ALeffectState *EchoStateFactory_create(EchoStateFactory *UNUSED(factory))
{
    ALechoState *state;

    NEW_OBJ0(state, ALechoState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(EchoStateFactory);

EffectStateFactory *EchoStateFactory_getFactory(void)
{
    static EchoStateFactory EchoFactory = { { GET_VTABLE2(EchoStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &EchoFactory);
}


void ALecho_setParami(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid echo integer property 0x%04x", param); }
void ALecho_setParamiv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid echo integer-vector property 0x%04x", param); }
void ALecho_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_ECHO_DELAY:
            if(!(val >= AL_ECHO_MIN_DELAY && val <= AL_ECHO_MAX_DELAY))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Echo delay out of range");
            props->Echo.Delay = val;
            break;

        case AL_ECHO_LRDELAY:
            if(!(val >= AL_ECHO_MIN_LRDELAY && val <= AL_ECHO_MAX_LRDELAY))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Echo LR delay out of range");
            props->Echo.LRDelay = val;
            break;

        case AL_ECHO_DAMPING:
            if(!(val >= AL_ECHO_MIN_DAMPING && val <= AL_ECHO_MAX_DAMPING))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Echo damping out of range");
            props->Echo.Damping = val;
            break;

        case AL_ECHO_FEEDBACK:
            if(!(val >= AL_ECHO_MIN_FEEDBACK && val <= AL_ECHO_MAX_FEEDBACK))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Echo feedback out of range");
            props->Echo.Feedback = val;
            break;

        case AL_ECHO_SPREAD:
            if(!(val >= AL_ECHO_MIN_SPREAD && val <= AL_ECHO_MAX_SPREAD))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Echo spread out of range");
            props->Echo.Spread = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid echo float property 0x%04x", param);
    }
}
void ALecho_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALecho_setParamf(effect, context, param, vals[0]); }

void ALecho_getParami(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid echo integer property 0x%04x", param); }
void ALecho_getParamiv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid echo integer-vector property 0x%04x", param); }
void ALecho_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_ECHO_DELAY:
            *val = props->Echo.Delay;
            break;

        case AL_ECHO_LRDELAY:
            *val = props->Echo.LRDelay;
            break;

        case AL_ECHO_DAMPING:
            *val = props->Echo.Damping;
            break;

        case AL_ECHO_FEEDBACK:
            *val = props->Echo.Feedback;
            break;

        case AL_ECHO_SPREAD:
            *val = props->Echo.Spread;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid echo float property 0x%04x", param);
    }
}
void ALecho_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALecho_getParamf(effect, context, param, vals); }

DEFINE_ALEFFECT_VTABLE(ALecho);
