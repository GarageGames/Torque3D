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


typedef struct ALmodulatorState {
    DERIVE_FROM_TYPE(ALeffectState);

    void (*Process)(ALfloat*, const ALfloat*, ALuint, const ALuint, ALuint);

    ALuint index;
    ALuint step;

    ALfloat Gain[MAX_EFFECT_CHANNELS][MAX_OUTPUT_CHANNELS];

    ALfilterState Filter[MAX_EFFECT_CHANNELS];
} ALmodulatorState;

static ALvoid ALmodulatorState_Destruct(ALmodulatorState *state);
static ALboolean ALmodulatorState_deviceUpdate(ALmodulatorState *state, ALCdevice *device);
static ALvoid ALmodulatorState_update(ALmodulatorState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props);
static ALvoid ALmodulatorState_process(ALmodulatorState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALmodulatorState)

DEFINE_ALEFFECTSTATE_VTABLE(ALmodulatorState);


#define WAVEFORM_FRACBITS  24
#define WAVEFORM_FRACONE   (1<<WAVEFORM_FRACBITS)
#define WAVEFORM_FRACMASK  (WAVEFORM_FRACONE-1)

static inline ALfloat Sin(ALuint index)
{
    return sinf(index*(F_TAU/WAVEFORM_FRACONE) - F_PI)*0.5f + 0.5f;
}

static inline ALfloat Saw(ALuint index)
{
    return (ALfloat)index / WAVEFORM_FRACONE;
}

static inline ALfloat Square(ALuint index)
{
    return (ALfloat)((index >> (WAVEFORM_FRACBITS - 1)) & 1);
}

#define DECL_TEMPLATE(func)                                                   \
static void Modulate##func(ALfloat *restrict dst, const ALfloat *restrict src,\
                           ALuint index, const ALuint step, ALuint todo)      \
{                                                                             \
    ALuint i;                                                                 \
    for(i = 0;i < todo;i++)                                                   \
    {                                                                         \
        index += step;                                                        \
        index &= WAVEFORM_FRACMASK;                                           \
        dst[i] = src[i] * func(index);                                        \
    }                                                                         \
}

DECL_TEMPLATE(Sin)
DECL_TEMPLATE(Saw)
DECL_TEMPLATE(Square)

#undef DECL_TEMPLATE


static void ALmodulatorState_Construct(ALmodulatorState *state)
{
    ALuint i;

    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALmodulatorState, ALeffectState, state);

    state->index = 0;
    state->step = 1;

    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
        ALfilterState_clear(&state->Filter[i]);
}

static ALvoid ALmodulatorState_Destruct(ALmodulatorState *state)
{
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALmodulatorState_deviceUpdate(ALmodulatorState *UNUSED(state), ALCdevice *UNUSED(device))
{
    return AL_TRUE;
}

static ALvoid ALmodulatorState_update(ALmodulatorState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props)
{
    ALfloat cw, a;
    ALuint i;

    if(props->Modulator.Waveform == AL_RING_MODULATOR_SINUSOID)
        state->Process = ModulateSin;
    else if(props->Modulator.Waveform == AL_RING_MODULATOR_SAWTOOTH)
        state->Process = ModulateSaw;
    else /*if(Slot->Params.EffectProps.Modulator.Waveform == AL_RING_MODULATOR_SQUARE)*/
        state->Process = ModulateSquare;

    state->step = fastf2u(props->Modulator.Frequency*WAVEFORM_FRACONE /
                          Device->Frequency);
    if(state->step == 0) state->step = 1;

    /* Custom filter coeffs, which match the old version instead of a low-shelf. */
    cw = cosf(F_TAU * props->Modulator.HighPassCutoff / Device->Frequency);
    a = (2.0f-cw) - sqrtf(powf(2.0f-cw, 2.0f) - 1.0f);

    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
    {
        state->Filter[i].a1 = -a;
        state->Filter[i].a2 = 0.0f;
        state->Filter[i].b0 = a;
        state->Filter[i].b1 = -a;
        state->Filter[i].b2 = 0.0f;
    }

    STATIC_CAST(ALeffectState,state)->OutBuffer = Device->FOAOut.Buffer;
    STATIC_CAST(ALeffectState,state)->OutChannels = Device->FOAOut.NumChannels;
    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
        ComputeFirstOrderGains(Device->FOAOut, IdentityMatrixf.m[i],
                               Slot->Params.Gain, state->Gain[i]);
}

static ALvoid ALmodulatorState_process(ALmodulatorState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    const ALuint step = state->step;
    ALuint index = state->index;
    ALuint base;

    for(base = 0;base < SamplesToDo;)
    {
        ALfloat temps[2][128];
        ALuint td = minu(128, SamplesToDo-base);
        ALuint i, j, k;

        for(j = 0;j < MAX_EFFECT_CHANNELS;j++)
        {
            ALfilterState_process(&state->Filter[j], temps[0], &SamplesIn[j][base], td);
            state->Process(temps[1], temps[0], index, step, td);

            for(k = 0;k < NumChannels;k++)
            {
                ALfloat gain = state->Gain[j][k];
                if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
                    continue;

                for(i = 0;i < td;i++)
                    SamplesOut[k][base+i] += gain * temps[1][i];
            }
        }

        for(i = 0;i < td;i++)
        {
            index += step;
            index &= WAVEFORM_FRACMASK;
        }
        base += td;
    }
    state->index = index;
}


typedef struct ALmodulatorStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALmodulatorStateFactory;

static ALeffectState *ALmodulatorStateFactory_create(ALmodulatorStateFactory *UNUSED(factory))
{
    ALmodulatorState *state;

    NEW_OBJ0(state, ALmodulatorState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALmodulatorStateFactory);

ALeffectStateFactory *ALmodulatorStateFactory_getFactory(void)
{
    static ALmodulatorStateFactory ModulatorFactory = { { GET_VTABLE2(ALmodulatorStateFactory, ALeffectStateFactory) } };

    return STATIC_CAST(ALeffectStateFactory, &ModulatorFactory);
}


void ALmodulator_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_RING_MODULATOR_FREQUENCY:
            if(!(val >= AL_RING_MODULATOR_MIN_FREQUENCY && val <= AL_RING_MODULATOR_MAX_FREQUENCY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Modulator.Frequency = val;
            break;

        case AL_RING_MODULATOR_HIGHPASS_CUTOFF:
            if(!(val >= AL_RING_MODULATOR_MIN_HIGHPASS_CUTOFF && val <= AL_RING_MODULATOR_MAX_HIGHPASS_CUTOFF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Modulator.HighPassCutoff = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALmodulator_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALmodulator_setParamf(effect, context, param, vals[0]);
}
void ALmodulator_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_RING_MODULATOR_FREQUENCY:
        case AL_RING_MODULATOR_HIGHPASS_CUTOFF:
            ALmodulator_setParamf(effect, context, param, (ALfloat)val);
            break;

        case AL_RING_MODULATOR_WAVEFORM:
            if(!(val >= AL_RING_MODULATOR_MIN_WAVEFORM && val <= AL_RING_MODULATOR_MAX_WAVEFORM))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Modulator.Waveform = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALmodulator_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALmodulator_setParami(effect, context, param, vals[0]);
}

void ALmodulator_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_RING_MODULATOR_FREQUENCY:
            *val = (ALint)props->Modulator.Frequency;
            break;
        case AL_RING_MODULATOR_HIGHPASS_CUTOFF:
            *val = (ALint)props->Modulator.HighPassCutoff;
            break;
        case AL_RING_MODULATOR_WAVEFORM:
            *val = props->Modulator.Waveform;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALmodulator_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALmodulator_getParami(effect, context, param, vals);
}
void ALmodulator_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_RING_MODULATOR_FREQUENCY:
            *val = props->Modulator.Frequency;
            break;
        case AL_RING_MODULATOR_HIGHPASS_CUTOFF:
            *val = props->Modulator.HighPassCutoff;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALmodulator_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    ALmodulator_getParamf(effect, context, param, vals);
}

DEFINE_ALEFFECT_VTABLE(ALmodulator);
