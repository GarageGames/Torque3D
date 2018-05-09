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
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"
#include "filters/defs.h"


static_assert(AL_CHORUS_WAVEFORM_SINUSOID == AL_FLANGER_WAVEFORM_SINUSOID, "Chorus/Flanger waveform value mismatch");
static_assert(AL_CHORUS_WAVEFORM_TRIANGLE == AL_FLANGER_WAVEFORM_TRIANGLE, "Chorus/Flanger waveform value mismatch");

enum WaveForm {
    WF_Sinusoid,
    WF_Triangle
};

typedef struct ALchorusState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALfloat *SampleBuffer;
    ALsizei BufferLength;
    ALsizei offset;

    ALsizei lfo_offset;
    ALsizei lfo_range;
    ALfloat lfo_scale;
    ALint lfo_disp;

    /* Gains for left and right sides */
    struct {
        ALfloat Current[MAX_OUTPUT_CHANNELS];
        ALfloat Target[MAX_OUTPUT_CHANNELS];
    } Gains[2];

    /* effect parameters */
    enum WaveForm waveform;
    ALint delay;
    ALfloat depth;
    ALfloat feedback;
} ALchorusState;

static ALvoid ALchorusState_Destruct(ALchorusState *state);
static ALboolean ALchorusState_deviceUpdate(ALchorusState *state, ALCdevice *Device);
static ALvoid ALchorusState_update(ALchorusState *state, const ALCcontext *Context, const ALeffectslot *Slot, const ALeffectProps *props);
static ALvoid ALchorusState_process(ALchorusState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALchorusState)

DEFINE_ALEFFECTSTATE_VTABLE(ALchorusState);


static void ALchorusState_Construct(ALchorusState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALchorusState, ALeffectState, state);

    state->BufferLength = 0;
    state->SampleBuffer = NULL;
    state->offset = 0;
    state->lfo_offset = 0;
    state->lfo_range = 1;
    state->waveform = WF_Triangle;
}

static ALvoid ALchorusState_Destruct(ALchorusState *state)
{
    al_free(state->SampleBuffer);
    state->SampleBuffer = NULL;

    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALchorusState_deviceUpdate(ALchorusState *state, ALCdevice *Device)
{
    const ALfloat max_delay = maxf(AL_CHORUS_MAX_DELAY, AL_FLANGER_MAX_DELAY);
    ALsizei maxlen;

    maxlen = NextPowerOf2(float2int(max_delay*2.0f*Device->Frequency) + 1u);
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

static ALvoid ALchorusState_update(ALchorusState *state, const ALCcontext *Context, const ALeffectslot *Slot, const ALeffectProps *props)
{
    const ALsizei mindelay = MAX_RESAMPLE_PADDING << FRACTIONBITS;
    const ALCdevice *device = Context->Device;
    ALfloat frequency = (ALfloat)device->Frequency;
    ALfloat coeffs[MAX_AMBI_COEFFS];
    ALfloat rate;
    ALint phase;

    switch(props->Chorus.Waveform)
    {
        case AL_CHORUS_WAVEFORM_TRIANGLE:
            state->waveform = WF_Triangle;
            break;
        case AL_CHORUS_WAVEFORM_SINUSOID:
            state->waveform = WF_Sinusoid;
            break;
    }

    /* The LFO depth is scaled to be relative to the sample delay. Clamp the
     * delay and depth to allow enough padding for resampling.
     */
    state->delay = maxi(float2int(props->Chorus.Delay*frequency*FRACTIONONE + 0.5f),
                        mindelay);
    state->depth = minf(props->Chorus.Depth * state->delay,
                        (ALfloat)(state->delay - mindelay));

    state->feedback = props->Chorus.Feedback;

    /* Gains for left and right sides */
    CalcAngleCoeffs(-F_PI_2, 0.0f, 0.0f, coeffs);
    ComputeDryPanGains(&device->Dry, coeffs, Slot->Params.Gain, state->Gains[0].Target);
    CalcAngleCoeffs( F_PI_2, 0.0f, 0.0f, coeffs);
    ComputeDryPanGains(&device->Dry, coeffs, Slot->Params.Gain, state->Gains[1].Target);

    phase = props->Chorus.Phase;
    rate = props->Chorus.Rate;
    if(!(rate > 0.0f))
    {
        state->lfo_offset = 0;
        state->lfo_range = 1;
        state->lfo_scale = 0.0f;
        state->lfo_disp = 0;
    }
    else
    {
        /* Calculate LFO coefficient (number of samples per cycle). Limit the
         * max range to avoid overflow when calculating the displacement.
         */
        ALsizei lfo_range = float2int(minf(frequency/rate + 0.5f, (ALfloat)(INT_MAX/360 - 180)));

        state->lfo_offset = float2int((ALfloat)state->lfo_offset/state->lfo_range*
                                      lfo_range + 0.5f) % lfo_range;
        state->lfo_range = lfo_range;
        switch(state->waveform)
        {
            case WF_Triangle:
                state->lfo_scale = 4.0f / state->lfo_range;
                break;
            case WF_Sinusoid:
                state->lfo_scale = F_TAU / state->lfo_range;
                break;
        }

        /* Calculate lfo phase displacement */
        if(phase < 0) phase = 360 + phase;
        state->lfo_disp = (state->lfo_range*phase + 180) / 360;
    }
}

static void GetTriangleDelays(ALint *restrict delays, ALsizei offset, const ALsizei lfo_range,
                              const ALfloat lfo_scale, const ALfloat depth, const ALsizei delay,
                              const ALsizei todo)
{
    ALsizei i;
    for(i = 0;i < todo;i++)
    {
        delays[i] = fastf2i((1.0f - fabsf(2.0f - lfo_scale*offset)) * depth) + delay;
        offset = (offset+1)%lfo_range;
    }
}

static void GetSinusoidDelays(ALint *restrict delays, ALsizei offset, const ALsizei lfo_range,
                              const ALfloat lfo_scale, const ALfloat depth, const ALsizei delay,
                              const ALsizei todo)
{
    ALsizei i;
    for(i = 0;i < todo;i++)
    {
        delays[i] = fastf2i(sinf(lfo_scale*offset) * depth) + delay;
        offset = (offset+1)%lfo_range;
    }
}


static ALvoid ALchorusState_process(ALchorusState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels)
{
    const ALsizei bufmask = state->BufferLength-1;
    const ALfloat feedback = state->feedback;
    const ALsizei avgdelay = (state->delay + (FRACTIONONE>>1)) >> FRACTIONBITS;
    ALfloat *restrict delaybuf = state->SampleBuffer;
    ALsizei offset = state->offset;
    ALsizei i, c;
    ALsizei base;

    for(base = 0;base < SamplesToDo;)
    {
        const ALsizei todo = mini(256, SamplesToDo-base);
        ALint moddelays[2][256];
        alignas(16) ALfloat temps[2][256];

        if(state->waveform == WF_Sinusoid)
        {
            GetSinusoidDelays(moddelays[0], state->lfo_offset, state->lfo_range, state->lfo_scale,
                              state->depth, state->delay, todo);
            GetSinusoidDelays(moddelays[1], (state->lfo_offset+state->lfo_disp)%state->lfo_range,
                              state->lfo_range, state->lfo_scale, state->depth, state->delay,
                              todo);
        }
        else /*if(state->waveform == WF_Triangle)*/
        {
            GetTriangleDelays(moddelays[0], state->lfo_offset, state->lfo_range, state->lfo_scale,
                              state->depth, state->delay, todo);
            GetTriangleDelays(moddelays[1], (state->lfo_offset+state->lfo_disp)%state->lfo_range,
                              state->lfo_range, state->lfo_scale, state->depth, state->delay,
                              todo);
        }
        state->lfo_offset = (state->lfo_offset+todo) % state->lfo_range;

        for(i = 0;i < todo;i++)
        {
            ALint delay;
            ALfloat mu;

            // Feed the buffer's input first (necessary for delays < 1).
            delaybuf[offset&bufmask] = SamplesIn[0][base+i];

            // Tap for the left output.
            delay = offset - (moddelays[0][i]>>FRACTIONBITS);
            mu = (moddelays[0][i]&FRACTIONMASK) * (1.0f/FRACTIONONE);
            temps[0][i] = cubic(delaybuf[(delay+1) & bufmask], delaybuf[(delay  ) & bufmask],
                                delaybuf[(delay-1) & bufmask], delaybuf[(delay-2) & bufmask],
                                mu);

            // Tap for the right output.
            delay = offset - (moddelays[1][i]>>FRACTIONBITS);
            mu = (moddelays[1][i]&FRACTIONMASK) * (1.0f/FRACTIONONE);
            temps[1][i] = cubic(delaybuf[(delay+1) & bufmask], delaybuf[(delay  ) & bufmask],
                                delaybuf[(delay-1) & bufmask], delaybuf[(delay-2) & bufmask],
                                mu);

            // Accumulate feedback from the average delay of the taps.
            delaybuf[offset&bufmask] += delaybuf[(offset-avgdelay) & bufmask] * feedback;
            offset++;
        }

        for(c = 0;c < 2;c++)
            MixSamples(temps[c], NumChannels, SamplesOut, state->Gains[c].Current,
                       state->Gains[c].Target, SamplesToDo-base, base, todo);

        base += todo;
    }

    state->offset = offset;
}


typedef struct ChorusStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} ChorusStateFactory;

static ALeffectState *ChorusStateFactory_create(ChorusStateFactory *UNUSED(factory))
{
    ALchorusState *state;

    NEW_OBJ0(state, ALchorusState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(ChorusStateFactory);


EffectStateFactory *ChorusStateFactory_getFactory(void)
{
    static ChorusStateFactory ChorusFactory = { { GET_VTABLE2(ChorusStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &ChorusFactory);
}


void ALchorus_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_WAVEFORM:
            if(!(val >= AL_CHORUS_MIN_WAVEFORM && val <= AL_CHORUS_MAX_WAVEFORM))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Invalid chorus waveform");
            props->Chorus.Waveform = val;
            break;

        case AL_CHORUS_PHASE:
            if(!(val >= AL_CHORUS_MIN_PHASE && val <= AL_CHORUS_MAX_PHASE))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Chorus phase out of range");
            props->Chorus.Phase = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid chorus integer property 0x%04x", param);
    }
}
void ALchorus_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{ ALchorus_setParami(effect, context, param, vals[0]); }
void ALchorus_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_CHORUS_RATE:
            if(!(val >= AL_CHORUS_MIN_RATE && val <= AL_CHORUS_MAX_RATE))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Chorus rate out of range");
            props->Chorus.Rate = val;
            break;

        case AL_CHORUS_DEPTH:
            if(!(val >= AL_CHORUS_MIN_DEPTH && val <= AL_CHORUS_MAX_DEPTH))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Chorus depth out of range");
            props->Chorus.Depth = val;
            break;

        case AL_CHORUS_FEEDBACK:
            if(!(val >= AL_CHORUS_MIN_FEEDBACK && val <= AL_CHORUS_MAX_FEEDBACK))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Chorus feedback out of range");
            props->Chorus.Feedback = val;
            break;

        case AL_CHORUS_DELAY:
            if(!(val >= AL_CHORUS_MIN_DELAY && val <= AL_CHORUS_MAX_DELAY))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Chorus delay out of range");
            props->Chorus.Delay = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid chorus float property 0x%04x", param);
    }
}
void ALchorus_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALchorus_setParamf(effect, context, param, vals[0]); }

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
            alSetError(context, AL_INVALID_ENUM, "Invalid chorus integer property 0x%04x", param);
    }
}
void ALchorus_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{ ALchorus_getParami(effect, context, param, vals); }
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
            alSetError(context, AL_INVALID_ENUM, "Invalid chorus float property 0x%04x", param);
    }
}
void ALchorus_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALchorus_getParamf(effect, context, param, vals); }

DEFINE_ALEFFECT_VTABLE(ALchorus);


/* Flanger is basically a chorus with a really short delay. They can both use
 * the same processing functions, so piggyback flanger on the chorus functions.
 */
typedef struct FlangerStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} FlangerStateFactory;

ALeffectState *FlangerStateFactory_create(FlangerStateFactory *UNUSED(factory))
{
    ALchorusState *state;

    NEW_OBJ0(state, ALchorusState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(FlangerStateFactory);

EffectStateFactory *FlangerStateFactory_getFactory(void)
{
    static FlangerStateFactory FlangerFactory = { { GET_VTABLE2(FlangerStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &FlangerFactory);
}


void ALflanger_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_FLANGER_WAVEFORM:
            if(!(val >= AL_FLANGER_MIN_WAVEFORM && val <= AL_FLANGER_MAX_WAVEFORM))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Invalid flanger waveform");
            props->Chorus.Waveform = val;
            break;

        case AL_FLANGER_PHASE:
            if(!(val >= AL_FLANGER_MIN_PHASE && val <= AL_FLANGER_MAX_PHASE))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Flanger phase out of range");
            props->Chorus.Phase = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid flanger integer property 0x%04x", param);
    }
}
void ALflanger_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{ ALflanger_setParami(effect, context, param, vals[0]); }
void ALflanger_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_FLANGER_RATE:
            if(!(val >= AL_FLANGER_MIN_RATE && val <= AL_FLANGER_MAX_RATE))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Flanger rate out of range");
            props->Chorus.Rate = val;
            break;

        case AL_FLANGER_DEPTH:
            if(!(val >= AL_FLANGER_MIN_DEPTH && val <= AL_FLANGER_MAX_DEPTH))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Flanger depth out of range");
            props->Chorus.Depth = val;
            break;

        case AL_FLANGER_FEEDBACK:
            if(!(val >= AL_FLANGER_MIN_FEEDBACK && val <= AL_FLANGER_MAX_FEEDBACK))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Flanger feedback out of range");
            props->Chorus.Feedback = val;
            break;

        case AL_FLANGER_DELAY:
            if(!(val >= AL_FLANGER_MIN_DELAY && val <= AL_FLANGER_MAX_DELAY))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Flanger delay out of range");
            props->Chorus.Delay = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid flanger float property 0x%04x", param);
    }
}
void ALflanger_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALflanger_setParamf(effect, context, param, vals[0]); }

void ALflanger_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_FLANGER_WAVEFORM:
            *val = props->Chorus.Waveform;
            break;

        case AL_FLANGER_PHASE:
            *val = props->Chorus.Phase;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid flanger integer property 0x%04x", param);
    }
}
void ALflanger_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{ ALflanger_getParami(effect, context, param, vals); }
void ALflanger_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_FLANGER_RATE:
            *val = props->Chorus.Rate;
            break;

        case AL_FLANGER_DEPTH:
            *val = props->Chorus.Depth;
            break;

        case AL_FLANGER_FEEDBACK:
            *val = props->Chorus.Feedback;
            break;

        case AL_FLANGER_DELAY:
            *val = props->Chorus.Delay;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid flanger float property 0x%04x", param);
    }
}
void ALflanger_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALflanger_getParamf(effect, context, param, vals); }

DEFINE_ALEFFECT_VTABLE(ALflanger);
