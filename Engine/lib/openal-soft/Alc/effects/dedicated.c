/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by Chris Robinson.
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

#include "alMain.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"
#include "filters/defs.h"


typedef struct ALdedicatedState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat CurrentGains[MAX_OUTPUT_CHANNELS];
    ALfloat TargetGains[MAX_OUTPUT_CHANNELS];
} ALdedicatedState;

static ALvoid ALdedicatedState_Destruct(ALdedicatedState *state);
static ALboolean ALdedicatedState_deviceUpdate(ALdedicatedState *state, ALCdevice *device);
static ALvoid ALdedicatedState_update(ALdedicatedState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALdedicatedState_process(ALdedicatedState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALdedicatedState)

DEFINE_ALEFFECTSTATE_VTABLE(ALdedicatedState);


static void ALdedicatedState_Construct(ALdedicatedState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALdedicatedState, ALeffectState, state);
}

static ALvoid ALdedicatedState_Destruct(ALdedicatedState *state)
{
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALdedicatedState_deviceUpdate(ALdedicatedState *state, ALCdevice *UNUSED(device))
{
    ALsizei i;
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        state->CurrentGains[i] = 0.0f;
    return AL_TRUE;
}

static ALvoid ALdedicatedState_update(ALdedicatedState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props)
{
    const ALCdevice *device = context->Device;
    ALfloat Gain;
    ALsizei i;

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        state->TargetGains[i] = 0.0f;

    Gain = slot->Params.Gain * props->Dedicated.Gain;
    if(slot->Params.EffectType == AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT)
    {
        int idx;
        if((idx=GetChannelIdxByName(&device->RealOut, LFE)) != -1)
        {
            STATIC_CAST(ALeffectState,state)->OutBuffer = device->RealOut.Buffer;
            STATIC_CAST(ALeffectState,state)->OutChannels = device->RealOut.NumChannels;
            state->TargetGains[idx] = Gain;
        }
    }
    else if(slot->Params.EffectType == AL_EFFECT_DEDICATED_DIALOGUE)
    {
        int idx;
        /* Dialog goes to the front-center speaker if it exists, otherwise it
         * plays from the front-center location. */
        if((idx=GetChannelIdxByName(&device->RealOut, FrontCenter)) != -1)
        {
            STATIC_CAST(ALeffectState,state)->OutBuffer = device->RealOut.Buffer;
            STATIC_CAST(ALeffectState,state)->OutChannels = device->RealOut.NumChannels;
            state->TargetGains[idx] = Gain;
        }
        else
        {
            ALfloat coeffs[MAX_AMBI_COEFFS];
            CalcAngleCoeffs(0.0f, 0.0f, 0.0f, coeffs);

            STATIC_CAST(ALeffectState,state)->OutBuffer = device->Dry.Buffer;
            STATIC_CAST(ALeffectState,state)->OutChannels = device->Dry.NumChannels;
            ComputeDryPanGains(&device->Dry, coeffs, Gain, state->TargetGains);
        }
    }
}

static ALvoid ALdedicatedState_process(ALdedicatedState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels)
{
    MixSamples(SamplesIn[0], NumChannels, SamplesOut, state->CurrentGains,
               state->TargetGains, SamplesToDo, 0, SamplesToDo);
}


typedef struct DedicatedStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} DedicatedStateFactory;

ALeffectState *DedicatedStateFactory_create(DedicatedStateFactory *UNUSED(factory))
{
    ALdedicatedState *state;

    NEW_OBJ0(state, ALdedicatedState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(DedicatedStateFactory);


EffectStateFactory *DedicatedStateFactory_getFactory(void)
{
    static DedicatedStateFactory DedicatedFactory = { { GET_VTABLE2(DedicatedStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &DedicatedFactory);
}


void ALdedicated_setParami(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid dedicated integer property 0x%04x", param); }
void ALdedicated_setParamiv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid dedicated integer-vector property 0x%04x", param); }
void ALdedicated_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_DEDICATED_GAIN:
            if(!(val >= 0.0f && isfinite(val)))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Dedicated gain out of range");
            props->Dedicated.Gain = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid dedicated float property 0x%04x", param);
    }
}
void ALdedicated_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALdedicated_setParamf(effect, context, param, vals[0]); }

void ALdedicated_getParami(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid dedicated integer property 0x%04x", param); }
void ALdedicated_getParamiv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid dedicated integer-vector property 0x%04x", param); }
void ALdedicated_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_DEDICATED_GAIN:
            *val = props->Dedicated.Gain;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid dedicated float property 0x%04x", param);
    }
}
void ALdedicated_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALdedicated_getParamf(effect, context, param, vals); }

DEFINE_ALEFFECT_VTABLE(ALdedicated);
