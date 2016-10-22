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


typedef struct ALechoState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat *SampleBuffer;
    ALuint BufferLength;

    // The echo is two tap. The delay is the number of samples from before the
    // current offset
    struct {
        ALuint delay;
    } Tap[2];
    ALuint Offset;
    /* The panning gains for the two taps */
    ALfloat Gain[2][MAX_OUTPUT_CHANNELS];

    ALfloat FeedGain;

    ALfilterState Filter;
} ALechoState;

static ALvoid ALechoState_Destruct(ALechoState *state);
static ALboolean ALechoState_deviceUpdate(ALechoState *state, ALCdevice *Device);
static ALvoid ALechoState_update(ALechoState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props);
static ALvoid ALechoState_process(ALechoState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels);
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

    ALfilterState_clear(&state->Filter);
}

static ALvoid ALechoState_Destruct(ALechoState *state)
{
    al_free(state->SampleBuffer);
    state->SampleBuffer = NULL;
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALechoState_deviceUpdate(ALechoState *state, ALCdevice *Device)
{
    ALuint maxlen, i;

    // Use the next power of 2 for the buffer length, so the tap offsets can be
    // wrapped using a mask instead of a modulo
    maxlen  = fastf2u(AL_ECHO_MAX_DELAY * Device->Frequency) + 1;
    maxlen += fastf2u(AL_ECHO_MAX_LRDELAY * Device->Frequency) + 1;
    maxlen  = NextPowerOf2(maxlen);

    if(maxlen != state->BufferLength)
    {
        void *temp = al_calloc(16, maxlen * sizeof(ALfloat));
        if(!temp) return AL_FALSE;

        al_free(state->SampleBuffer);
        state->SampleBuffer = temp;
        state->BufferLength = maxlen;
    }
    for(i = 0;i < state->BufferLength;i++)
        state->SampleBuffer[i] = 0.0f;

    return AL_TRUE;
}

static ALvoid ALechoState_update(ALechoState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props)
{
    ALuint frequency = Device->Frequency;
    ALfloat coeffs[MAX_AMBI_COEFFS];
    ALfloat gain, lrpan, spread;

    state->Tap[0].delay = fastf2u(props->Echo.Delay * frequency) + 1;
    state->Tap[1].delay = fastf2u(props->Echo.LRDelay * frequency);
    state->Tap[1].delay += state->Tap[0].delay;

    spread = props->Echo.Spread;
    if(spread < 0.0f) lrpan = -1.0f;
    else lrpan = 1.0f;
    /* Convert echo spread (where 0 = omni, +/-1 = directional) to coverage
     * spread (where 0 = point, tau = omni).
     */
    spread = asinf(1.0f - fabsf(spread))*4.0f;

    state->FeedGain = props->Echo.Feedback;

    gain = minf(1.0f - props->Echo.Damping, 0.01f);
    ALfilterState_setParams(&state->Filter, ALfilterType_HighShelf,
                            gain, LOWPASSFREQREF/frequency,
                            calc_rcpQ_from_slope(gain, 0.75f));

    gain = Slot->Params.Gain;

    /* First tap panning */
    CalcXYZCoeffs(-lrpan, 0.0f, 0.0f, spread, coeffs);
    ComputePanningGains(Device->Dry, coeffs, gain, state->Gain[0]);

    /* Second tap panning */
    CalcXYZCoeffs( lrpan, 0.0f, 0.0f, spread, coeffs);
    ComputePanningGains(Device->Dry, coeffs, gain, state->Gain[1]);
}

static ALvoid ALechoState_process(ALechoState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    const ALuint mask = state->BufferLength-1;
    const ALuint tap1 = state->Tap[0].delay;
    const ALuint tap2 = state->Tap[1].delay;
    ALuint offset = state->Offset;
    ALfloat x[2], y[2], in, out;
    ALuint base;
    ALuint i, k;

    x[0] = state->Filter.x[0];
    x[1] = state->Filter.x[1];
    y[0] = state->Filter.y[0];
    y[1] = state->Filter.y[1];
    for(base = 0;base < SamplesToDo;)
    {
        ALfloat temps[128][2];
        ALuint td = minu(128, SamplesToDo-base);

        for(i = 0;i < td;i++)
        {
            /* First tap */
            temps[i][0] = state->SampleBuffer[(offset-tap1) & mask];
            /* Second tap */
            temps[i][1] = state->SampleBuffer[(offset-tap2) & mask];

            // Apply damping and feedback gain to the second tap, and mix in the
            // new sample
            in = temps[i][1] + SamplesIn[0][i+base];
            out = in*state->Filter.b0 +
                  x[0]*state->Filter.b1 + x[1]*state->Filter.b2 -
                  y[0]*state->Filter.a1 - y[1]*state->Filter.a2;
            x[1] = x[0]; x[0] = in;
            y[1] = y[0]; y[0] = out;

            state->SampleBuffer[offset&mask] = out * state->FeedGain;
            offset++;
        }

        for(k = 0;k < NumChannels;k++)
        {
            ALfloat gain = state->Gain[0][k];
            if(fabsf(gain) > GAIN_SILENCE_THRESHOLD)
            {
                for(i = 0;i < td;i++)
                    SamplesOut[k][i+base] += temps[i][0] * gain;
            }

            gain = state->Gain[1][k];
            if(fabsf(gain) > GAIN_SILENCE_THRESHOLD)
            {
                for(i = 0;i < td;i++)
                    SamplesOut[k][i+base] += temps[i][1] * gain;
            }
        }

        base += td;
    }
    state->Filter.x[0] = x[0];
    state->Filter.x[1] = x[1];
    state->Filter.y[0] = y[0];
    state->Filter.y[1] = y[1];

    state->Offset = offset;
}


typedef struct ALechoStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALechoStateFactory;

ALeffectState *ALechoStateFactory_create(ALechoStateFactory *UNUSED(factory))
{
    ALechoState *state;

    NEW_OBJ0(state, ALechoState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALechoStateFactory);

ALeffectStateFactory *ALechoStateFactory_getFactory(void)
{
    static ALechoStateFactory EchoFactory = { { GET_VTABLE2(ALechoStateFactory, ALeffectStateFactory) } };

    return STATIC_CAST(ALeffectStateFactory, &EchoFactory);
}


void ALecho_setParami(ALeffect *UNUSED(effect), ALCcontext *context, ALenum UNUSED(param), ALint UNUSED(val))
{ SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM); }
void ALecho_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALecho_setParami(effect, context, param, vals[0]);
}
void ALecho_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_ECHO_DELAY:
            if(!(val >= AL_ECHO_MIN_DELAY && val <= AL_ECHO_MAX_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Echo.Delay = val;
            break;

        case AL_ECHO_LRDELAY:
            if(!(val >= AL_ECHO_MIN_LRDELAY && val <= AL_ECHO_MAX_LRDELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Echo.LRDelay = val;
            break;

        case AL_ECHO_DAMPING:
            if(!(val >= AL_ECHO_MIN_DAMPING && val <= AL_ECHO_MAX_DAMPING))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Echo.Damping = val;
            break;

        case AL_ECHO_FEEDBACK:
            if(!(val >= AL_ECHO_MIN_FEEDBACK && val <= AL_ECHO_MAX_FEEDBACK))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Echo.Feedback = val;
            break;

        case AL_ECHO_SPREAD:
            if(!(val >= AL_ECHO_MIN_SPREAD && val <= AL_ECHO_MAX_SPREAD))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Echo.Spread = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALecho_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALecho_setParamf(effect, context, param, vals[0]);
}

void ALecho_getParami(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum UNUSED(param), ALint *UNUSED(val))
{ SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM); }
void ALecho_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALecho_getParami(effect, context, param, vals);
}
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
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALecho_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    ALecho_getParamf(effect, context, param, vals);
}

DEFINE_ALEFFECT_VTABLE(ALecho);
