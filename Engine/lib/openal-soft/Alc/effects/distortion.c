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


typedef struct ALdistortionState {
    DERIVE_FROM_TYPE(ALeffectState);

    /* Effect gains for each channel */
    ALfloat Gain[MAX_OUTPUT_CHANNELS];

    /* Effect parameters */
    BiquadFilter lowpass;
    BiquadFilter bandpass;
    ALfloat attenuation;
    ALfloat edge_coeff;

    ALfloat Buffer[2][BUFFERSIZE];
} ALdistortionState;

static ALvoid ALdistortionState_Destruct(ALdistortionState *state);
static ALboolean ALdistortionState_deviceUpdate(ALdistortionState *state, ALCdevice *device);
static ALvoid ALdistortionState_update(ALdistortionState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALdistortionState_process(ALdistortionState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALdistortionState)

DEFINE_ALEFFECTSTATE_VTABLE(ALdistortionState);


static void ALdistortionState_Construct(ALdistortionState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALdistortionState, ALeffectState, state);
}

static ALvoid ALdistortionState_Destruct(ALdistortionState *state)
{
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

static ALboolean ALdistortionState_deviceUpdate(ALdistortionState *state, ALCdevice *UNUSED(device))
{
    BiquadFilter_clear(&state->lowpass);
    BiquadFilter_clear(&state->bandpass);
    return AL_TRUE;
}

static ALvoid ALdistortionState_update(ALdistortionState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props)
{
    const ALCdevice *device = context->Device;
    ALfloat frequency = (ALfloat)device->Frequency;
    ALfloat coeffs[MAX_AMBI_COEFFS];
    ALfloat bandwidth;
    ALfloat cutoff;
    ALfloat edge;

    /* Store waveshaper edge settings. */
    edge = sinf(props->Distortion.Edge * (F_PI_2));
    edge = minf(edge, 0.99f);
    state->edge_coeff = 2.0f * edge / (1.0f-edge);

    cutoff = props->Distortion.LowpassCutoff;
    /* Bandwidth value is constant in octaves. */
    bandwidth = (cutoff / 2.0f) / (cutoff * 0.67f);
    /* Multiply sampling frequency by the amount of oversampling done during
     * processing.
     */
    BiquadFilter_setParams(&state->lowpass, BiquadType_LowPass, 1.0f,
        cutoff / (frequency*4.0f), calc_rcpQ_from_bandwidth(cutoff / (frequency*4.0f), bandwidth)
    );

    cutoff = props->Distortion.EQCenter;
    /* Convert bandwidth in Hz to octaves. */
    bandwidth = props->Distortion.EQBandwidth / (cutoff * 0.67f);
    BiquadFilter_setParams(&state->bandpass, BiquadType_BandPass, 1.0f,
        cutoff / (frequency*4.0f), calc_rcpQ_from_bandwidth(cutoff / (frequency*4.0f), bandwidth)
    );

    CalcAngleCoeffs(0.0f, 0.0f, 0.0f, coeffs);
    ComputeDryPanGains(&device->Dry, coeffs, slot->Params.Gain * props->Distortion.Gain,
                       state->Gain);
}

static ALvoid ALdistortionState_process(ALdistortionState *state, ALsizei SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALsizei NumChannels)
{
    ALfloat (*restrict buffer)[BUFFERSIZE] = state->Buffer;
    const ALfloat fc = state->edge_coeff;
    ALsizei base;
    ALsizei i, k;

    for(base = 0;base < SamplesToDo;)
    {
        /* Perform 4x oversampling to avoid aliasing. Oversampling greatly
         * improves distortion quality and allows to implement lowpass and
         * bandpass filters using high frequencies, at which classic IIR
         * filters became unstable.
         */
        ALsizei todo = mini(BUFFERSIZE, (SamplesToDo-base) * 4);

        /* Fill oversample buffer using zero stuffing. Multiply the sample by
         * the amount of oversampling to maintain the signal's power.
         */
        for(i = 0;i < todo;i++)
            buffer[0][i] = !(i&3) ? SamplesIn[0][(i>>2)+base] * 4.0f : 0.0f;

        /* First step, do lowpass filtering of original signal. Additionally
         * perform buffer interpolation and lowpass cutoff for oversampling
         * (which is fortunately first step of distortion). So combine three
         * operations into the one.
         */
        BiquadFilter_process(&state->lowpass, buffer[1], buffer[0], todo);

        /* Second step, do distortion using waveshaper function to emulate
         * signal processing during tube overdriving. Three steps of
         * waveshaping are intended to modify waveform without boost/clipping/
         * attenuation process.
         */
        for(i = 0;i < todo;i++)
        {
            ALfloat smp = buffer[1][i];

            smp = (1.0f + fc) * smp/(1.0f + fc*fabsf(smp));
            smp = (1.0f + fc) * smp/(1.0f + fc*fabsf(smp)) * -1.0f;
            smp = (1.0f + fc) * smp/(1.0f + fc*fabsf(smp));

            buffer[0][i] = smp;
        }

        /* Third step, do bandpass filtering of distorted signal. */
        BiquadFilter_process(&state->bandpass, buffer[1], buffer[0], todo);

        todo >>= 2;
        for(k = 0;k < NumChannels;k++)
        {
            /* Fourth step, final, do attenuation and perform decimation,
             * storing only one sample out of four.
             */
            ALfloat gain = state->Gain[k];
            if(!(fabsf(gain) > GAIN_SILENCE_THRESHOLD))
                continue;

            for(i = 0;i < todo;i++)
                SamplesOut[k][base+i] += gain * buffer[1][i*4];
        }

        base += todo;
    }
}


typedef struct DistortionStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} DistortionStateFactory;

static ALeffectState *DistortionStateFactory_create(DistortionStateFactory *UNUSED(factory))
{
    ALdistortionState *state;

    NEW_OBJ0(state, ALdistortionState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_EFFECTSTATEFACTORY_VTABLE(DistortionStateFactory);


EffectStateFactory *DistortionStateFactory_getFactory(void)
{
    static DistortionStateFactory DistortionFactory = { { GET_VTABLE2(DistortionStateFactory, EffectStateFactory) } };

    return STATIC_CAST(EffectStateFactory, &DistortionFactory);
}


void ALdistortion_setParami(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid distortion integer property 0x%04x", param); }
void ALdistortion_setParamiv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid distortion integer-vector property 0x%04x", param); }
void ALdistortion_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_DISTORTION_EDGE:
            if(!(val >= AL_DISTORTION_MIN_EDGE && val <= AL_DISTORTION_MAX_EDGE))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Distortion edge out of range");
            props->Distortion.Edge = val;
            break;

        case AL_DISTORTION_GAIN:
            if(!(val >= AL_DISTORTION_MIN_GAIN && val <= AL_DISTORTION_MAX_GAIN))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Distortion gain out of range");
            props->Distortion.Gain = val;
            break;

        case AL_DISTORTION_LOWPASS_CUTOFF:
            if(!(val >= AL_DISTORTION_MIN_LOWPASS_CUTOFF && val <= AL_DISTORTION_MAX_LOWPASS_CUTOFF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Distortion low-pass cutoff out of range");
            props->Distortion.LowpassCutoff = val;
            break;

        case AL_DISTORTION_EQCENTER:
            if(!(val >= AL_DISTORTION_MIN_EQCENTER && val <= AL_DISTORTION_MAX_EQCENTER))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Distortion EQ center out of range");
            props->Distortion.EQCenter = val;
            break;

        case AL_DISTORTION_EQBANDWIDTH:
            if(!(val >= AL_DISTORTION_MIN_EQBANDWIDTH && val <= AL_DISTORTION_MAX_EQBANDWIDTH))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Distortion EQ bandwidth out of range");
            props->Distortion.EQBandwidth = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid distortion float property 0x%04x",
                       param);
    }
}
void ALdistortion_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALdistortion_setParamf(effect, context, param, vals[0]); }

void ALdistortion_getParami(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid distortion integer property 0x%04x", param); }
void ALdistortion_getParamiv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid distortion integer-vector property 0x%04x", param); }
void ALdistortion_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_DISTORTION_EDGE:
            *val = props->Distortion.Edge;
            break;

        case AL_DISTORTION_GAIN:
            *val = props->Distortion.Gain;
            break;

        case AL_DISTORTION_LOWPASS_CUTOFF:
            *val = props->Distortion.LowpassCutoff;
            break;

        case AL_DISTORTION_EQCENTER:
            *val = props->Distortion.EQCenter;
            break;

        case AL_DISTORTION_EQBANDWIDTH:
            *val = props->Distortion.EQBandwidth;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid distortion float property 0x%04x",
                       param);
    }
}
void ALdistortion_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALdistortion_getParamf(effect, context, param, vals); }

DEFINE_ALEFFECT_VTABLE(ALdistortion);
