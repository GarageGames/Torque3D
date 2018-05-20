/**
 * OpenAL cross platform audio library
 * Copyright (C) 2013 by Anis A. Hireche
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

#include <stdlib.h>

#include "config.h"
#include "alError.h"
#include "alMain.h"
#include "alAuxEffectSlot.h"
#include "alu.h"


typedef struct ALcompressorState {
    DERIVE_FROM_TYPE(ALeffectState);

    /* Effect gains for each channel */
    ALfloat Gain[MAX_EFFECT_CHANNELS][MAX_OUTPUT_CHANNELS];

    /* Effect parameters */
    ALboolean Enabled;
    ALfloat AttackRate;
    ALfloat ReleaseRate;
    ALfloat GainCtrl;
} ALcompressorState;

static ALvoid ALcompressorState_Destruct(ALcompressorState *state);
static ALboolean ALcompressorState_deviceUpdate(ALcompressorState *state, ALCdevice *device);
static ALvoid ALcompressorState_update(ALcompressorState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALcompressorState_process(ALcompressorState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALcompressorState)

DEFINE_ALEFFECTSTATE_VTABLE(ALcompressorState);


static void ALcompressorState_Construct(ALcompressorState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALcompressorState, ALeffectState, state);

    state->Enabled = AL_TRUE;
    state->AttackRate = 0.0f;
    state->ReleaseRate = 0.0f;
    state->GainCtrl = 1.0f;
}

static ALvoid ALcompressorState_Destruct(ALcompressorState *state)
{
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALcompressorState_deviceUpdate(ALcompressorState *state, ALCdevice *device)
{
    const ALfloat attackTime = device->Frequency * 0.2f; /* 200ms Attack */
    const ALfloat releaseTime = device->Frequency * 0.4f; /* 400ms Release */

    state->AttackRate = 1.0f / attackTime;
    state->ReleaseRate = 1.0f / releaseTime;

    return AL_TRUE;
}

static ALvoid ALcompressorState_update(ALcompressorState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props)
{
    const ALCdevice *device = context->Device;
    ALuint i;

    state->Enabled = props->Compressor.OnOff;

    STATIC_CAST(ALeffectState,state)->OutBuffer = device->FOAOut.Buffer;
    STATIC_CAST(ALeffectState,state)->OutChannels = device->FOAOut.NumChannels;
    for(i = 0;i < 4;i++)
        ComputeFirstOrderGains(&device->FOAOut, IdentityMatrixf.m[i],
                               slot->Params.Gain, state->Gain[i]);
}

static ALvoid ALcompressorState_process(ALcompressorState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels)
{
    ALsizei i, j, k;
    ALsizei base;

    for(base = 0;base < SamplesToDo;)
    {
        ALfloat temps[64][4];
        ALsizei td = mini(64, SamplesToDo-base);

        /* Load samples into the temp buffer first. */
        for(j = 0;j < 4;j++)
        {
            for(i = 0;i < td;i++)
                temps[i][j] = SamplesIn[j][i+base];
        }

        if(state->Enabled)
        {
            ALfloat gain = state->GainCtrl;
            ALfloat output, amplitude;

            for(i = 0;i < td;i++)
            {
                /* Roughly calculate the maximum amplitude from the 4-channel
                 * signal, and attack or release the gain control to reach it.
                 */
                amplitude = fabsf(temps[i][0]);
                amplitude = maxf(amplitude + fabsf(temps[i][1]),
                                 maxf(amplitude + fabsf(temps[i][2]),
                                      amplitude + fabsf(temps[i][3])));
                if(amplitude > gain)
                    gain = minf(gain+state->AttackRate, amplitude);
                else if(amplitude < gain)
                    gain = maxf(gain-state->ReleaseRate, amplitude);

                /* Apply the inverse of the gain control to normalize/compress
                 * the volume. */
                output = 1.0f / clampf(gain, 0.5f, 2.0f);
                for(j = 0;j < 4;j++)
                    temps[i][j] *= output;
            }

            state->GainCtrl = gain;
        }
        else
        {
            ALfloat gain = state->GainCtrl;
            ALfloat output, amplitude;

            for(i = 0;i < td;i++)
            {
                /* Same as above, except the amplitude is forced to 1. This
                 * helps ensure smooth gain changes when the compressor is
                 * turned on and off.
                 */
                amplitude = 1.0f;
                if(amplitude > gain)
                    gain = minf(gain+state->AttackRate, amplitude);
                else if(amplitude < gain)
                    gain = maxf(gain-state->ReleaseRate, amplitude);

                output = 1.0f / clampf(gain, 0.5f, 2.0f);
                for(j = 0;j < 4;j++)
                    temps[i][j] *= output;
            }

            state->GainCtrl = gain;
        }

        /* Now mix to the output. */
        for(j = 0;j < 4;j++)
        {
            for(k = 0;k < NumChannels;k++)
            {
                ALfloat gain = state->Gain[j][k];
                if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
                    continue;

                for(i = 0;i < td;i++)
                    SamplesOut[k][base+i] += gain * temps[i][j];
            }
        }

        base += td;
    }
}


typedef struct CompressorStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} CompressorStateFactory;

static ALeffectState *CompressorStateFactory_create(CompressorStateFactory *UNUSED(factory))
{
    ALcompressorState *state;

    NEW_OBJ0(state, ALcompressorState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(CompressorStateFactory);

EffectStateFactory *CompressorStateFactory_getFactory(void)
{
    static CompressorStateFactory CompressorFactory = { { GET_VTABLE2(CompressorStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &CompressorFactory);
}


void ALcompressor_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_COMPRESSOR_ONOFF:
            if(!(val >= AL_COMPRESSOR_MIN_ONOFF && val <= AL_COMPRESSOR_MAX_ONOFF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Compressor state out of range");
            props->Compressor.OnOff = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid compressor integer property 0x%04x",
                       param);
    }
}
void ALcompressor_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{ ALcompressor_setParami(effect, context, param, vals[0]); }
void ALcompressor_setParamf(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid compressor float property 0x%04x", param); }
void ALcompressor_setParamfv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALfloat *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid compressor float-vector property 0x%04x", param); }

void ALcompressor_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{ 
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_COMPRESSOR_ONOFF:
            *val = props->Compressor.OnOff;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid compressor integer property 0x%04x",
                       param);
    }
}
void ALcompressor_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{ ALcompressor_getParami(effect, context, param, vals); }
void ALcompressor_getParamf(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid compressor float property 0x%04x", param); }
void ALcompressor_getParamfv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid compressor float-vector property 0x%04x", param); }

DEFINE_ALEFFECT_VTABLE(ALcompressor);
