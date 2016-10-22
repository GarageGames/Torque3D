/**
 * OpenAL cross platform audio library
 * Copyright (C) 2013 by Mike Gorchak
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


enum ChorusWaveForm {
    CWF_Triangle = AL_CHORUS_WAVEFORM_TRIANGLE,
    CWF_Sinusoid = AL_CHORUS_WAVEFORM_SINUSOID
};

typedef struct ALchorusState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat *SampleBuffer[2];
    ALuint BufferLength;
    ALuint offset;
    ALuint lfo_range;
    ALfloat lfo_scale;
    ALint lfo_disp;

    /* Gains for left and right sides */
    ALfloat Gain[2][MAX_OUTPUT_CHANNELS];

    /* effect parameters */
    enum ChorusWaveForm waveform;
    ALint delay;
    ALfloat depth;
    ALfloat feedback;
} ALchorusState;

static ALvoid ALchorusState_Destruct(ALchorusState *state);
static ALboolean ALchorusState_deviceUpdate(ALchorusState *state, ALCdevice *Device);
static ALvoid ALchorusState_update(ALchorusState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props);
static ALvoid ALchorusState_process(ALchorusState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALchorusState)

DEFINE_ALEFFECTSTATE_VTABLE(ALchorusState);


static void ALchorusState_Construct(ALchorusState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALchorusState, ALeffectState, state);

    state->BufferLength = 0;
    state->SampleBuffer[0] = NULL;
    state->SampleBuffer[1] = NULL;
    state->offset = 0;
    state->lfo_range = 1;
    state->waveform = CWF_Triangle;
}

static ALvoid ALchorusState_Destruct(ALchorusState *state)
{
    al_free(state->SampleBuffer[0]);
    state->SampleBuffer[0] = NULL;
    state->SampleBuffer[1] = NULL;

    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALchorusState_deviceUpdate(ALchorusState *state, ALCdevice *Device)
{
    ALuint maxlen;
    ALuint it;

    maxlen = fastf2u(AL_CHORUS_MAX_DELAY * 3.0f * Device->Frequency) + 1;
    maxlen = NextPowerOf2(maxlen);

    if(maxlen != state->BufferLength)
    {
        void *temp = al_calloc(16, maxlen * sizeof(ALfloat) * 2);
        if(!temp) return AL_FALSE;

        al_free(state->SampleBuffer[0]);
        state->SampleBuffer[0] = temp;
        state->SampleBuffer[1] = state->SampleBuffer[0] + maxlen;

        state->BufferLength = maxlen;
    }

    for(it = 0;it < state->BufferLength;it++)
    {
        state->SampleBuffer[0][it] = 0.0f;
        state->SampleBuffer[1][it] = 0.0f;
    }

    return AL_TRUE;
}

static ALvoid ALchorusState_update(ALchorusState *state, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props)
{
    ALfloat frequency = (ALfloat)Device->Frequency;
    ALfloat coeffs[MAX_AMBI_COEFFS];
    ALfloat rate;
    ALint phase;

    switch(props->Chorus.Waveform)
    {
        case AL_CHORUS_WAVEFORM_TRIANGLE:
            state->waveform = CWF_Triangle;
            break;
        case AL_CHORUS_WAVEFORM_SINUSOID:
            state->waveform = CWF_Sinusoid;
            break;
    }
    state->depth = props->Chorus.Depth;
    state->feedback = props->Chorus.Feedback;
    state->delay = fastf2i(props->Chorus.Delay * frequency);

    /* Gains for left and right sides */
    CalcXYZCoeffs(-1.0f, 0.0f, 0.0f, 0.0f, coeffs);
    ComputePanningGains(Device->Dry, coeffs, Slot->Params.Gain, state->Gain[0]);
    CalcXYZCoeffs( 1.0f, 0.0f, 0.0f, 0.0f, coeffs);
    ComputePanningGains(Device->Dry, coeffs, Slot->Params.Gain, state->Gain[1]);

    phase = props->Chorus.Phase;
    rate = props->Chorus.Rate;
    if(!(rate > 0.0f))
    {
        state->lfo_scale = 0.0f;
        state->lfo_range = 1;
        state->lfo_disp = 0;
    }
    else
    {
        /* Calculate LFO coefficient */
        state->lfo_range = fastf2u(frequency/rate + 0.5f);
        switch(state->waveform)
        {
            case CWF_Triangle:
                state->lfo_scale = 4.0f / state->lfo_range;
                break;
            case CWF_Sinusoid:
                state->lfo_scale = F_TAU / state->lfo_range;
                break;
        }

        /* Calculate lfo phase displacement */
        state->lfo_disp = fastf2i(state->lfo_range * (phase/360.0f));
    }
}

static inline void Triangle(ALint *delay_left, ALint *delay_right, ALuint offset, const ALchorusState *state)
{
    ALfloat lfo_value;

    lfo_value = 2.0f - fabsf(2.0f - state->lfo_scale*(offset%state->lfo_range));
    lfo_value *= state->depth * state->delay;
    *delay_left = fastf2i(lfo_value) + state->delay;

    offset += state->lfo_disp;
    lfo_value = 2.0f - fabsf(2.0f - state->lfo_scale*(offset%state->lfo_range));
    lfo_value *= state->depth * state->delay;
    *delay_right = fastf2i(lfo_value) + state->delay;
}

static inline void Sinusoid(ALint *delay_left, ALint *delay_right, ALuint offset, const ALchorusState *state)
{
    ALfloat lfo_value;

    lfo_value = 1.0f + sinf(state->lfo_scale*(offset%state->lfo_range));
    lfo_value *= state->depth * state->delay;
    *delay_left = fastf2i(lfo_value) + state->delay;

    offset += state->lfo_disp;
    lfo_value = 1.0f + sinf(state->lfo_scale*(offset%state->lfo_range));
    lfo_value *= state->depth * state->delay;
    *delay_right = fastf2i(lfo_value) + state->delay;
}

#define DECL_TEMPLATE(Func)                                                   \
static void Process##Func(ALchorusState *state, const ALuint SamplesToDo,     \
  const ALfloat *restrict SamplesIn, ALfloat (*restrict out)[2])              \
{                                                                             \
    const ALuint bufmask = state->BufferLength-1;                             \
    ALfloat *restrict leftbuf = state->SampleBuffer[0];                       \
    ALfloat *restrict rightbuf = state->SampleBuffer[1];                      \
    ALuint offset = state->offset;                                            \
    const ALfloat feedback = state->feedback;                                 \
    ALuint it;                                                                \
                                                                              \
    for(it = 0;it < SamplesToDo;it++)                                         \
    {                                                                         \
        ALint delay_left, delay_right;                                        \
        Func(&delay_left, &delay_right, offset, state);                       \
                                                                              \
        out[it][0] = leftbuf[(offset-delay_left)&bufmask];                    \
        leftbuf[offset&bufmask] = (out[it][0]+SamplesIn[it]) * feedback;      \
                                                                              \
        out[it][1] = rightbuf[(offset-delay_right)&bufmask];                  \
        rightbuf[offset&bufmask] = (out[it][1]+SamplesIn[it]) * feedback;     \
                                                                              \
        offset++;                                                             \
    }                                                                         \
    state->offset = offset;                                                   \
}

DECL_TEMPLATE(Triangle)
DECL_TEMPLATE(Sinusoid)

#undef DECL_TEMPLATE

static ALvoid ALchorusState_process(ALchorusState *state, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    ALuint it, kt;
    ALuint base;

    for(base = 0;base < SamplesToDo;)
    {
        ALfloat temps[128][2];
        ALuint td = minu(128, SamplesToDo-base);

        switch(state->waveform)
        {
            case CWF_Triangle:
                ProcessTriangle(state, td, SamplesIn[0]+base, temps);
                break;
            case CWF_Sinusoid:
                ProcessSinusoid(state, td, SamplesIn[0]+base, temps);
                break;
        }

        for(kt = 0;kt < NumChannels;kt++)
        {
            ALfloat gain = state->Gain[0][kt];
            if(fabsf(gain) > GAIN_SILENCE_THRESHOLD)
            {
                for(it = 0;it < td;it++)
                    SamplesOut[kt][it+base] += temps[it][0] * gain;
            }

            gain = state->Gain[1][kt];
            if(fabsf(gain) > GAIN_SILENCE_THRESHOLD)
            {
                for(it = 0;it < td;it++)
                    SamplesOut[kt][it+base] += temps[it][1] * gain;
            }
        }

        base += td;
    }
}


typedef struct ALchorusStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALchorusStateFactory;

static ALeffectState *ALchorusStateFactory_create(ALchorusStateFactory *UNUSED(factory))
{
    ALchorusState *state;

    NEW_OBJ0(state, ALchorusState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALchorusStateFactory);


ALeffectStateFactory *ALchorusStateFactory_getFactory(void)
{
    static ALchorusStateFactory ChorusFactory = { { GET_VTABLE2(ALchorusStateFactory, ALeffectStateFactory) } };

    return STATIC_CAST(ALeffectStateFactory, &ChorusFactory);
}


void ALchorus_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_WAVEFORM:
            if(!(val >= AL_CHORUS_MIN_WAVEFORM && val <= AL_CHORUS_MAX_WAVEFORM))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Waveform = val;
            break;

        case AL_CHORUS_PHASE:
            if(!(val >= AL_CHORUS_MIN_PHASE && val <= AL_CHORUS_MAX_PHASE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Phase = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALchorus_setParami(effect, context, param, vals[0]);
}
void ALchorus_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_RATE:
            if(!(val >= AL_CHORUS_MIN_RATE && val <= AL_CHORUS_MAX_RATE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Rate = val;
            break;

        case AL_CHORUS_DEPTH:
            if(!(val >= AL_CHORUS_MIN_DEPTH && val <= AL_CHORUS_MAX_DEPTH))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Depth = val;
            break;

        case AL_CHORUS_FEEDBACK:
            if(!(val >= AL_CHORUS_MIN_FEEDBACK && val <= AL_CHORUS_MAX_FEEDBACK))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Feedback = val;
            break;

        case AL_CHORUS_DELAY:
            if(!(val >= AL_CHORUS_MIN_DELAY && val <= AL_CHORUS_MAX_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Chorus.Delay = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALchorus_setParamf(effect, context, param, vals[0]);
}

void ALchorus_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_WAVEFORM:
            *val = props->Chorus.Waveform;
            break;

        case AL_CHORUS_PHASE:
            *val = props->Chorus.Phase;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALchorus_getParami(effect, context, param, vals);
}
void ALchorus_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_RATE:
            *val = props->Chorus.Rate;
            break;

        case AL_CHORUS_DEPTH:
            *val = props->Chorus.Depth;
            break;

        case AL_CHORUS_FEEDBACK:
            *val = props->Chorus.Feedback;
            break;

        case AL_CHORUS_DELAY:
            *val = props->Chorus.Delay;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALchorus_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    ALchorus_getParamf(effect, context, param, vals);
}

DEFINE_ALEFFECT_VTABLE(ALchorus);
