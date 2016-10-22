/**
 * Reverb for the OpenAL cross platform audio library
 * Copyright (C) 2008-2009 by Christopher Fitzgerald.
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "alMain.h"
#include "alu.h"
#include "alAuxEffectSlot.h"
#include "alEffect.h"
#include "alFilter.h"
#include "alError.h"
#include "mixer_defs.h"


/* This is the maximum number of samples processed for each inner loop
 * iteration. */
#define MAX_UPDATE_SAMPLES  256


static MixerFunc MixSamples = Mix_C;
static RowMixerFunc MixRowSamples = MixRow_C;

static alonce_flag mixfunc_inited = AL_ONCE_FLAG_INIT;
static void init_mixfunc(void)
{
    MixSamples = SelectMixer();
    MixRowSamples = SelectRowMixer();
}


typedef struct DelayLine
{
    // The delay lines use sample lengths that are powers of 2 to allow the
    // use of bit-masking instead of a modulus for wrapping.
    ALuint   Mask;
    ALfloat *Line;
} DelayLine;

typedef struct ALreverbState {
    DERIVE_FROM_TYPE(ALeffectState);

    ALboolean IsEax;

    // All delay lines are allocated as a single buffer to reduce memory
    // fragmentation and management code.
    ALfloat  *SampleBuffer;
    ALuint    TotalSamples;

    // Master effect filters
    struct {
        ALfilterState Lp;
        ALfilterState Hp; // EAX only
    } Filter[4];

    struct {
        // Modulator delay line.
        DelayLine Delay[4];

        // The vibrato time is tracked with an index over a modulus-wrapped
        // range (in samples).
        ALuint    Index;
        ALuint    Range;

        // The depth of frequency change (also in samples) and its filter.
        ALfloat   Depth;
        ALfloat   Coeff;
        ALfloat   Filter;
    } Mod; // EAX only

    /* Core delay line (early reflections and late reverb tap from this). */
    DelayLine Delay;
    /* The tap points for the initial delay. First set go to early
     * reflections, second to late reverb.
     */
    ALuint    EarlyDelayTap[4];
    ALuint    LateDelayTap[4];

    struct {
        // Early reflections are done with 4 delay lines.
        ALfloat   Coeff[4];
        DelayLine Delay[4];
        ALuint    Offset[4];

        // The gain for each output channel based on 3D panning.
        ALfloat CurrentGain[4][MAX_OUTPUT_CHANNELS];
        ALfloat PanGain[4][MAX_OUTPUT_CHANNELS];
    } Early;

    struct {
        // Output gain for late reverb.
        ALfloat   Gain;

        // Attenuation to compensate for the modal density and decay rate of
        // the late lines.
        ALfloat   DensityGain;

        // The feed-back and feed-forward all-pass coefficient.
        ALfloat   ApFeedCoeff;

        // Mixing matrix coefficient.
        ALfloat   MixCoeff;

        // Late reverb has 4 parallel all-pass filters.
        struct {
            ALfloat   Coeff;
            DelayLine Delay;
            ALuint    Offset;
        } Ap[4];

        // In addition to 4 cyclical delay lines.
        ALfloat   Coeff[4];
        DelayLine Delay[4];
        ALuint    Offset[4];

        // The cyclical delay lines are 1-pole low-pass filtered.
        struct {
            ALfloat Sample;
            ALfloat Coeff;
        } Lp[4];

        // The gain for each output channel based on 3D panning.
        ALfloat CurrentGain[4][MAX_OUTPUT_CHANNELS];
        ALfloat PanGain[4][MAX_OUTPUT_CHANNELS];
    } Late;

    struct {
        // Attenuation to compensate for the modal density and decay rate of
        // the echo line.
        ALfloat   DensityGain;

        // Echo delay and all-pass lines.
        struct {
            DelayLine Feedback;
            DelayLine Ap;
        } Delay[4];

        ALfloat   Coeff;
        ALfloat   ApFeedCoeff;
        ALfloat   ApCoeff;

        ALuint    Offset;
        ALuint    ApOffset;

        // The echo line is 1-pole low-pass filtered.
        ALfloat   LpCoeff;
        ALfloat   LpSample[4];

        // Echo mixing coefficient.
        ALfloat   MixCoeff;
    } Echo; // EAX only

    // The current read offset for all delay lines.
    ALuint Offset;

    /* Temporary storage used when processing. */
    alignas(16) ALfloat AFormatSamples[4][MAX_UPDATE_SAMPLES];
    alignas(16) ALfloat ReverbSamples[4][MAX_UPDATE_SAMPLES];
    alignas(16) ALfloat EarlySamples[4][MAX_UPDATE_SAMPLES];
} ALreverbState;

static ALvoid ALreverbState_Destruct(ALreverbState *State);
static ALboolean ALreverbState_deviceUpdate(ALreverbState *State, ALCdevice *Device);
static ALvoid ALreverbState_update(ALreverbState *State, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props);
static ALvoid ALreverbState_process(ALreverbState *State, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels);
DECLARE_DEFAULT_ALLOCATORS(ALreverbState)

DEFINE_ALEFFECTSTATE_VTABLE(ALreverbState);


static void ALreverbState_Construct(ALreverbState *state)
{
    ALuint index, l;

    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALreverbState, ALeffectState, state);

    state->IsEax = AL_FALSE;

    state->TotalSamples = 0;
    state->SampleBuffer = NULL;

    for(index = 0;index < 4;index++)
    {
        ALfilterState_clear(&state->Filter[index].Lp);
        ALfilterState_clear(&state->Filter[index].Hp);

        state->Mod.Delay[index].Mask = 0;
        state->Mod.Delay[index].Line = NULL;
    }

    state->Mod.Index = 0;
    state->Mod.Range = 1;
    state->Mod.Depth = 0.0f;
    state->Mod.Coeff = 0.0f;
    state->Mod.Filter = 0.0f;

    state->Delay.Mask = 0;
    state->Delay.Line = NULL;
    for(index = 0;index < 4;index++)
        state->EarlyDelayTap[index] = 0;
    for(index = 0;index < 4;index++)
        state->LateDelayTap[index] = 0;

    for(index = 0;index < 4;index++)
    {
        state->Early.Coeff[index] = 0.0f;
        state->Early.Delay[index].Mask = 0;
        state->Early.Delay[index].Line = NULL;
        state->Early.Offset[index] = 0;
    }

    state->Late.Gain = 0.0f;
    state->Late.DensityGain = 0.0f;
    state->Late.ApFeedCoeff = 0.0f;
    state->Late.MixCoeff = 0.0f;
    for(index = 0;index < 4;index++)
    {
        state->Late.Ap[index].Coeff = 0.0f;
        state->Late.Ap[index].Delay.Mask = 0;
        state->Late.Ap[index].Delay.Line = NULL;
        state->Late.Ap[index].Offset = 0;

        state->Late.Coeff[index] = 0.0f;
        state->Late.Delay[index].Mask = 0;
        state->Late.Delay[index].Line = NULL;
        state->Late.Offset[index] = 0;

        state->Late.Lp[index].Sample = 0.0f;
        state->Late.Lp[index].Coeff = 0.0f;
    }

    for(l = 0;l < 4;l++)
    {
        for(index = 0;index < MAX_OUTPUT_CHANNELS;index++)
        {
            state->Early.CurrentGain[l][index] = 0.0f;
            state->Early.PanGain[l][index] = 0.0f;
            state->Late.CurrentGain[l][index] = 0.0f;
            state->Late.PanGain[l][index] = 0.0f;
        }
    }

    state->Echo.DensityGain = 0.0f;
    for(l = 0;l < 4;l++)
    {
        state->Echo.Delay[l].Feedback.Mask = 0;
        state->Echo.Delay[l].Feedback.Line = NULL;
        state->Echo.Delay[l].Ap.Mask = 0;
        state->Echo.Delay[l].Ap.Line = NULL;
    }
    state->Echo.Coeff = 0.0f;
    state->Echo.ApFeedCoeff = 0.0f;
    state->Echo.ApCoeff = 0.0f;
    state->Echo.Offset = 0;
    state->Echo.ApOffset = 0;
    state->Echo.LpCoeff = 0.0f;
    for(l = 0;l < 4;l++)
        state->Echo.LpSample[l] = 0.0f;
    state->Echo.MixCoeff = 0.0f;

    state->Offset = 0;
}

static ALvoid ALreverbState_Destruct(ALreverbState *State)
{
    al_free(State->SampleBuffer);
    State->SampleBuffer = NULL;

    ALeffectState_Destruct(STATIC_CAST(ALeffectState,State));
}

/* This is a user config option for modifying the overall output of the reverb
 * effect.
 */
ALfloat ReverbBoost = 1.0f;

/* Specifies whether to use a standard reverb effect in place of EAX reverb (no
 * high-pass, modulation, or echo).
 */
ALboolean EmulateEAXReverb = AL_FALSE;

/* This coefficient is used to define the maximum frequency range controlled
 * by the modulation depth.  The current value of 0.1 will allow it to swing
 * from 0.9x to 1.1x.  This value must be below 1.  At 1 it will cause the
 * sampler to stall on the downswing, and above 1 it will cause it to sample
 * backwards.
 */
static const ALfloat MODULATION_DEPTH_COEFF = 0.1f;

/* A filter is used to avoid the terrible distortion caused by changing
 * modulation time and/or depth.  To be consistent across different sample
 * rates, the coefficient must be raised to a constant divided by the sample
 * rate:  coeff^(constant / rate).
 */
static const ALfloat MODULATION_FILTER_COEFF = 0.048f;
static const ALfloat MODULATION_FILTER_CONST = 100000.0f;

// When diffusion is above 0, an all-pass filter is used to take the edge off
// the echo effect.  It uses the following line length (in seconds).
static const ALfloat ECHO_ALLPASS_LENGTH = 0.0133f;

/* Input into the early reflections and late reverb are decorrelated between
 * four channels. Their timings are dependent on a fraction and multiplier. See
 * the UpdateDelayLine() routine for the calculations involved.
 */
static const ALfloat DECO_FRACTION = 0.15f;
static const ALfloat DECO_MULTIPLIER = 2.0f;

// All delay line lengths are specified in seconds.

// The lengths of the early delay lines.
static const ALfloat EARLY_LINE_LENGTH[4] =
{
    0.0015f, 0.0045f, 0.0135f, 0.0405f
};

// The lengths of the late cyclical delay lines.
static const ALfloat LATE_LINE_LENGTH[4] =
{
    0.0211f, 0.0311f, 0.0461f, 0.0680f
};

// The lengths of the late all-pass delay lines.
static const ALfloat ALLPASS_LINE_LENGTH[4] =
{
    0.0151f, 0.0167f, 0.0183f, 0.0200f,
};

// The late cyclical delay lines have a variable length dependent on the
// effect's density parameter (inverted for some reason) and this multiplier.
static const ALfloat LATE_LINE_MULTIPLIER = 4.0f;


#if defined(_WIN32) && !defined (_M_X64) && !defined(_M_ARM)
/* HACK: Workaround for a modff bug in 32-bit Windows, which attempts to write
 * a 64-bit double to the 32-bit float parameter.
 */
static inline float hack_modff(float x, float *y)
{
    double di;
    double df = modf((double)x, &di);
    *y = (float)di;
    return (float)df;
}
#define modff hack_modff
#endif


/**************************************
 *  Device Update                     *
 **************************************/

// Given the allocated sample buffer, this function updates each delay line
// offset.
static inline ALvoid RealizeLineOffset(ALfloat *sampleBuffer, DelayLine *Delay)
{
    Delay->Line = &sampleBuffer[(ptrdiff_t)Delay->Line];
}

// Calculate the length of a delay line and store its mask and offset.
static ALuint CalcLineLength(ALfloat length, ptrdiff_t offset, ALuint frequency, ALuint extra, DelayLine *Delay)
{
    ALuint samples;

    // All line lengths are powers of 2, calculated from their lengths, with
    // an additional sample in case of rounding errors.
    samples = fastf2u(length*frequency) + extra;
    samples = NextPowerOf2(samples + 1);
    // All lines share a single sample buffer.
    Delay->Mask = samples - 1;
    Delay->Line = (ALfloat*)offset;
    // Return the sample count for accumulation.
    return samples;
}

/* Calculates the delay line metrics and allocates the shared sample buffer
 * for all lines given the sample rate (frequency).  If an allocation failure
 * occurs, it returns AL_FALSE.
 */
static ALboolean AllocLines(ALuint frequency, ALreverbState *State)
{
    ALuint totalSamples, index;
    ALfloat length;

    // All delay line lengths are calculated to accomodate the full range of
    // lengths given their respective paramters.
    totalSamples = 0;

    /* The modulator's line length is calculated from the maximum modulation
     * time and depth coefficient, and halfed for the low-to-high frequency
     * swing.  An additional sample is added to keep it stable when there is no
     * modulation.
     */
    length = (AL_EAXREVERB_MAX_MODULATION_TIME*MODULATION_DEPTH_COEFF/2.0f);
    for(index = 0;index < 4;index++)
        totalSamples += CalcLineLength(length, totalSamples, frequency, 1,
                                       &State->Mod.Delay[index]);

    /* The initial delay is the sum of the reflections and late reverb delays.
     * The decorrelator length is calculated from the lowest reverb density (a
     * parameter value of 1). This must include space for storing a loop
     * update.
     */
    length = AL_EAXREVERB_MAX_REFLECTIONS_DELAY +
             AL_EAXREVERB_MAX_LATE_REVERB_DELAY;
    length += (DECO_FRACTION * DECO_MULTIPLIER * DECO_MULTIPLIER) *
              LATE_LINE_LENGTH[0] * (1.0f + LATE_LINE_MULTIPLIER);
    /* Multiply length by 4, since we're storing 4 interleaved channels in the
     * main delay line.
     */
    totalSamples += CalcLineLength(length*4, totalSamples, frequency,
                                   MAX_UPDATE_SAMPLES*4, &State->Delay);

    // The early reflection lines.
    for(index = 0;index < 4;index++)
        totalSamples += CalcLineLength(EARLY_LINE_LENGTH[index], totalSamples,
                                       frequency, 0, &State->Early.Delay[index]);

    // The late delay lines are calculated from the lowest reverb density.
    for(index = 0;index < 4;index++)
    {
        length = LATE_LINE_LENGTH[index] * (1.0f + LATE_LINE_MULTIPLIER);
        totalSamples += CalcLineLength(length, totalSamples, frequency, 0,
                                       &State->Late.Delay[index]);
    }

    // The late all-pass lines.
    for(index = 0;index < 4;index++)
        totalSamples += CalcLineLength(ALLPASS_LINE_LENGTH[index], totalSamples,
                                       frequency, 0, &State->Late.Ap[index].Delay);

    // The echo all-pass and delay lines.
    for(index = 0;index < 4;index++)
    {
        totalSamples += CalcLineLength(ECHO_ALLPASS_LENGTH, totalSamples,
                                       frequency, 0, &State->Echo.Delay[index].Ap);
        totalSamples += CalcLineLength(AL_EAXREVERB_MAX_ECHO_TIME, totalSamples,
                                       frequency, 0, &State->Echo.Delay[index].Feedback);
    }

    if(totalSamples != State->TotalSamples)
    {
        ALfloat *newBuffer;

        TRACE("New reverb buffer length: %u samples\n", totalSamples);
        newBuffer = al_calloc(16, sizeof(ALfloat) * totalSamples);
        if(!newBuffer) return AL_FALSE;

        al_free(State->SampleBuffer);
        State->SampleBuffer = newBuffer;
        State->TotalSamples = totalSamples;
    }

    // Update all delays to reflect the new sample buffer.
    RealizeLineOffset(State->SampleBuffer, &State->Delay);
    for(index = 0;index < 4;index++)
    {
        RealizeLineOffset(State->SampleBuffer, &State->Mod.Delay[index]);

        RealizeLineOffset(State->SampleBuffer, &State->Early.Delay[index]);

        RealizeLineOffset(State->SampleBuffer, &State->Late.Ap[index].Delay);
        RealizeLineOffset(State->SampleBuffer, &State->Late.Delay[index]);

        RealizeLineOffset(State->SampleBuffer, &State->Echo.Delay[index].Ap);
        RealizeLineOffset(State->SampleBuffer, &State->Echo.Delay[index].Feedback);
    }

    // Clear the sample buffer.
    for(index = 0;index < State->TotalSamples;index++)
        State->SampleBuffer[index] = 0.0f;

    return AL_TRUE;
}

static ALboolean ALreverbState_deviceUpdate(ALreverbState *State, ALCdevice *Device)
{
    ALuint frequency = Device->Frequency, index;

    // Allocate the delay lines.
    if(!AllocLines(frequency, State))
        return AL_FALSE;

    // Calculate the modulation filter coefficient.  Notice that the exponent
    // is calculated given the current sample rate.  This ensures that the
    // resulting filter response over time is consistent across all sample
    // rates.
    State->Mod.Coeff = powf(MODULATION_FILTER_COEFF,
                            MODULATION_FILTER_CONST / frequency);

    // The early reflection and late all-pass filter line lengths are static,
    // so their offsets only need to be calculated once.
    for(index = 0;index < 4;index++)
    {
        State->Early.Offset[index] = fastf2u(EARLY_LINE_LENGTH[index] * frequency);
        State->Late.Ap[index].Offset = fastf2u(ALLPASS_LINE_LENGTH[index] * frequency);
    }

    // The echo all-pass filter line length is static, so its offset only
    // needs to be calculated once.
    State->Echo.ApOffset = fastf2u(ECHO_ALLPASS_LENGTH * frequency);

    return AL_TRUE;
}

/**************************************
 *  Effect Update                     *
 **************************************/

// Calculate a decay coefficient given the length of each cycle and the time
// until the decay reaches -60 dB.
static inline ALfloat CalcDecayCoeff(ALfloat length, ALfloat decayTime)
{
    return powf(0.001f/*-60 dB*/, length/decayTime);
}

// Calculate a decay length from a coefficient and the time until the decay
// reaches -60 dB.
static inline ALfloat CalcDecayLength(ALfloat coeff, ALfloat decayTime)
{
    return log10f(coeff) * decayTime / log10f(0.001f)/*-60 dB*/;
}

// Calculate an attenuation to be applied to the input of any echo models to
// compensate for modal density and decay time.
static inline ALfloat CalcDensityGain(ALfloat a)
{
    /* The energy of a signal can be obtained by finding the area under the
     * squared signal.  This takes the form of Sum(x_n^2), where x is the
     * amplitude for the sample n.
     *
     * Decaying feedback matches exponential decay of the form Sum(a^n),
     * where a is the attenuation coefficient, and n is the sample.  The area
     * under this decay curve can be calculated as:  1 / (1 - a).
     *
     * Modifying the above equation to find the squared area under the curve
     * (for energy) yields:  1 / (1 - a^2).  Input attenuation can then be
     * calculated by inverting the square root of this approximation,
     * yielding:  1 / sqrt(1 / (1 - a^2)), simplified to: sqrt(1 - a^2).
     */
    return sqrtf(1.0f - (a * a));
}

// Calculate the mixing matrix coefficients given a diffusion factor.
static inline ALvoid CalcMatrixCoeffs(ALfloat diffusion, ALfloat *x, ALfloat *y)
{
    ALfloat n, t;

    // The matrix is of order 4, so n is sqrt (4 - 1).
    n = sqrtf(3.0f);
    t = diffusion * atanf(n);

    // Calculate the first mixing matrix coefficient.
    *x = cosf(t);
    // Calculate the second mixing matrix coefficient.
    *y = sinf(t) / n;
}

// Calculate the limited HF ratio for use with the late reverb low-pass
// filters.
static ALfloat CalcLimitedHfRatio(ALfloat hfRatio, ALfloat airAbsorptionGainHF, ALfloat decayTime)
{
    ALfloat limitRatio;

    /* Find the attenuation due to air absorption in dB (converting delay
     * time to meters using the speed of sound).  Then reversing the decay
     * equation, solve for HF ratio.  The delay length is cancelled out of
     * the equation, so it can be calculated once for all lines.
     */
    limitRatio = 1.0f / (CalcDecayLength(airAbsorptionGainHF, decayTime) *
                         SPEEDOFSOUNDMETRESPERSEC);
    /* Using the limit calculated above, apply the upper bound to the HF
     * ratio. Also need to limit the result to a minimum of 0.1, just like the
     * HF ratio parameter. */
    return clampf(limitRatio, 0.1f, hfRatio);
}

// Calculate the coefficient for a HF (and eventually LF) decay damping
// filter.
static inline ALfloat CalcDampingCoeff(ALfloat hfRatio, ALfloat length, ALfloat decayTime, ALfloat decayCoeff, ALfloat cw)
{
    ALfloat coeff, g;

    // Eventually this should boost the high frequencies when the ratio
    // exceeds 1.
    coeff = 0.0f;
    if (hfRatio < 1.0f)
    {
        // Calculate the low-pass coefficient by dividing the HF decay
        // coefficient by the full decay coefficient.
        g = CalcDecayCoeff(length, decayTime * hfRatio) / decayCoeff;

        // Damping is done with a 1-pole filter, so g needs to be squared.
        g *= g;
        if(g < 0.9999f) /* 1-epsilon */
        {
            /* Be careful with gains < 0.001, as that causes the coefficient
             * head towards 1, which will flatten the signal. */
            g = maxf(g, 0.001f);
            coeff = (1 - g*cw - sqrtf(2*g*(1-cw) - g*g*(1 - cw*cw))) /
                    (1 - g);
        }

        // Very low decay times will produce minimal output, so apply an
        // upper bound to the coefficient.
        coeff = minf(coeff, 0.98f);
    }
    return coeff;
}

// Update the EAX modulation index, range, and depth.  Keep in mind that this
// kind of vibrato is additive and not multiplicative as one may expect.  The
// downswing will sound stronger than the upswing.
static ALvoid UpdateModulator(ALfloat modTime, ALfloat modDepth, ALuint frequency, ALreverbState *State)
{
    ALuint range;

    /* Modulation is calculated in two parts.
     *
     * The modulation time effects the sinus applied to the change in
     * frequency.  An index out of the current time range (both in samples)
     * is incremented each sample.  The range is bound to a reasonable
     * minimum (1 sample) and when the timing changes, the index is rescaled
     * to the new range (to keep the sinus consistent).
     */
    range = maxu(fastf2u(modTime*frequency), 1);
    State->Mod.Index = (ALuint)(State->Mod.Index * (ALuint64)range /
                                State->Mod.Range);
    State->Mod.Range = range;

    /* The modulation depth effects the amount of frequency change over the
     * range of the sinus.  It needs to be scaled by the modulation time so
     * that a given depth produces a consistent change in frequency over all
     * ranges of time.  Since the depth is applied to a sinus value, it needs
     * to be halfed once for the sinus range and again for the sinus swing
     * in time (half of it is spent decreasing the frequency, half is spent
     * increasing it).
     */
    State->Mod.Depth = modDepth * MODULATION_DEPTH_COEFF * modTime / 2.0f /
                       2.0f * frequency;
}

// Update the offsets for the main effect delay line.
static ALvoid UpdateDelayLine(ALfloat earlyDelay, ALfloat lateDelay, ALfloat density, ALuint frequency, ALreverbState *State)
{
    ALfloat length;
    ALuint i;

    /* The early reflections and late reverb inputs are decorrelated to provide
     * time-varying reflections, smooth out the reverb tail, and reduce harsh
     * echoes. The first tap occurs immediately, while the remaining taps are
     * delayed by multiples of a fraction of the smallest cyclical delay time.
     *
     * offset[index] = (FRACTION (MULTIPLIER^(index-1))) smallest_delay
     *
     * for index = 1...max_lines
     */
    State->EarlyDelayTap[0] = fastf2u(earlyDelay * frequency);
    for(i = 1;i < 4;i++)
    {
        length = (DECO_FRACTION * powf(DECO_MULTIPLIER, (ALfloat)i-1.0f)) *
                 EARLY_LINE_LENGTH[0];
        State->EarlyDelayTap[i] = fastf2u(length * frequency) + State->EarlyDelayTap[0];
    }

    State->LateDelayTap[0] = fastf2u((earlyDelay + lateDelay) * frequency);
    for(i = 1;i < 4;i++)
    {
        length = (DECO_FRACTION * powf(DECO_MULTIPLIER, (ALfloat)i-1.0f)) *
                 LATE_LINE_LENGTH[0] * (1.0f + (density * LATE_LINE_MULTIPLIER));
        State->LateDelayTap[i] = fastf2u(length * frequency) + State->LateDelayTap[0];
    }
}

// Update the early reflections mix and line coefficients.
static ALvoid UpdateEarlyLines(ALfloat lateDelay, ALreverbState *State)
{
    ALuint index;

    // Calculate the gain (coefficient) for each early delay line using the
    // late delay time.  This expands the early reflections to the start of
    // the late reverb.
    for(index = 0;index < 4;index++)
        State->Early.Coeff[index] = CalcDecayCoeff(EARLY_LINE_LENGTH[index],
                                                   lateDelay);
}

// Update the late reverb mix, line lengths, and line coefficients.
static ALvoid UpdateLateLines(ALfloat xMix, ALfloat density, ALfloat decayTime, ALfloat diffusion, ALfloat echoDepth, ALfloat hfRatio, ALfloat cw, ALuint frequency, ALreverbState *State)
{
    ALfloat length;
    ALuint index;

    /* Calculate the late reverb gain. Since the output is tapped prior to the
     * application of the next delay line coefficients, this gain needs to be
     * attenuated by the 'x' mixing matrix coefficient as well.  Also attenuate
     * the late reverb when echo depth is high and diffusion is low, so the
     * echo is slightly stronger than the decorrelated echos in the reverb
     * tail.
     */
    State->Late.Gain = xMix * (1.0f - (echoDepth*0.5f*(1.0f - diffusion)));

    /* To compensate for changes in modal density and decay time of the late
     * reverb signal, the input is attenuated based on the maximal energy of
     * the outgoing signal.  This approximation is used to keep the apparent
     * energy of the signal equal for all ranges of density and decay time.
     *
     * The average length of the cyclcical delay lines is used to calculate
     * the attenuation coefficient.
     */
    length = (LATE_LINE_LENGTH[0] + LATE_LINE_LENGTH[1] +
              LATE_LINE_LENGTH[2] + LATE_LINE_LENGTH[3]) / 4.0f;
    length *= 1.0f + (density * LATE_LINE_MULTIPLIER);
    /* To account for each channel being a discrete input, also multiply by
     * sqrt(num_channels).
     */
    State->Late.DensityGain = 2.0f * CalcDensityGain(
        CalcDecayCoeff(length, decayTime)
    );

    // Calculate the all-pass feed-back and feed-forward coefficient.
    State->Late.ApFeedCoeff = 0.5f * powf(diffusion, 2.0f);

    for(index = 0;index < 4;index++)
    {
        // Calculate the gain (coefficient) for each all-pass line.
        State->Late.Ap[index].Coeff = CalcDecayCoeff(
            ALLPASS_LINE_LENGTH[index], decayTime
        );

        // Calculate the length (in seconds) of each cyclical delay line.
        length = LATE_LINE_LENGTH[index] *
                 (1.0f + (density * LATE_LINE_MULTIPLIER));

        // Calculate the delay offset for each cyclical delay line.
        State->Late.Offset[index] = fastf2u(length * frequency);

        // Calculate the gain (coefficient) for each cyclical line.
        State->Late.Coeff[index] = CalcDecayCoeff(length, decayTime);

        // Calculate the damping coefficient for each low-pass filter.
        State->Late.Lp[index].Coeff = CalcDampingCoeff(
            hfRatio, length, decayTime, State->Late.Coeff[index], cw
        );

        // Attenuate the cyclical line coefficients by the mixing coefficient
        // (x).
        State->Late.Coeff[index] *= xMix;
    }
}

// Update the echo gain, line offset, line coefficients, and mixing
// coefficients.
static ALvoid UpdateEchoLine(ALfloat echoTime, ALfloat decayTime, ALfloat diffusion, ALfloat echoDepth, ALfloat hfRatio, ALfloat cw, ALuint frequency, ALreverbState *State)
{
    // Update the offset and coefficient for the echo delay line.
    State->Echo.Offset = fastf2u(echoTime * frequency);

    // Calculate the decay coefficient for the echo line.
    State->Echo.Coeff = CalcDecayCoeff(echoTime, decayTime);

    // Calculate the energy-based attenuation coefficient for the echo delay
    // line.
    State->Echo.DensityGain = CalcDensityGain(State->Echo.Coeff);

    // Calculate the echo all-pass feed coefficient.
    State->Echo.ApFeedCoeff = 0.5f * powf(diffusion, 2.0f);

    // Calculate the echo all-pass attenuation coefficient.
    State->Echo.ApCoeff = CalcDecayCoeff(ECHO_ALLPASS_LENGTH, decayTime);

    // Calculate the damping coefficient for each low-pass filter.
    State->Echo.LpCoeff = CalcDampingCoeff(hfRatio, echoTime, decayTime,
                                           State->Echo.Coeff, cw);

    /* Calculate the echo mixing coefficient. This is applied to the output mix
     * only, not the feedback.
     */
    State->Echo.MixCoeff = echoDepth;
}

/* Creates a transform matrix given a reverb vector. This works by creating a
 * Z-focus transform, then a rotate transform around X, then Y, to place the
 * focal point in the direction of the vector, using the vector length as a
 * focus strength.
 *
 * This isn't technically correct since the vector is supposed to define the
 * aperture and not rotate the perceived soundfield, but in practice it's
 * probably good enough.
 */
static aluMatrixf GetTransformFromVector(const ALfloat *vec)
{
    aluMatrixf zfocus, xrot, yrot;
    aluMatrixf tmp1, tmp2;
    ALfloat length;
    ALfloat sa, a;

    length = sqrtf(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);

    /* Define a Z-focus (X in Ambisonics) transform, given the panning vector
     * length.
     */
    sa = sinf(minf(length, 1.0f) * (F_PI/4.0f));
    aluMatrixfSet(&zfocus,
                     1.0f/(1.0f+sa),                       0.0f,                       0.0f, (sa/(1.0f+sa))/1.732050808f,
                               0.0f, sqrtf((1.0f-sa)/(1.0f+sa)),                       0.0f,                        0.0f,
                               0.0f,                       0.0f, sqrtf((1.0f-sa)/(1.0f+sa)),                        0.0f,
        (sa/(1.0f+sa))*1.732050808f,                       0.0f,                       0.0f,              1.0f/(1.0f+sa)
    );

    /* Define rotation around X (Y in Ambisonics) */
    a = atan2f(vec[1], sqrtf(vec[0]*vec[0] + vec[2]*vec[2]));
    aluMatrixfSet(&xrot,
        1.0f, 0.0f,     0.0f,    0.0f,
        0.0f, 1.0f,     0.0f,    0.0f,
        0.0f, 0.0f,  cosf(a), sinf(a),
        0.0f, 0.0f, -sinf(a), cosf(a)
    );

    /* Define rotation around Y (Z in Ambisonics). NOTE: EFX's reverb vectors
     * use a right-handled coordinate system, compared to the rest of OpenAL
     * which uses left-handed. This is fixed by negating Z, however it would
     * need to also be negated to get a proper Ambisonics angle, thus
     * cancelling it out.
     */
    a = atan2f(-vec[0], vec[2]);
    aluMatrixfSet(&yrot,
        1.0f,     0.0f, 0.0f,    0.0f,
        0.0f,  cosf(a), 0.0f, sinf(a),
        0.0f,     0.0f, 1.0f,    0.0f,
        0.0f, -sinf(a), 0.0f, cosf(a)
    );

#define MATRIX_MULT(_res, _m1, _m2) do {                                      \
    int row, col;                                                             \
    for(col = 0;col < 4;col++)                                                \
    {                                                                         \
        for(row = 0;row < 4;row++)                                            \
            _res.m[row][col] = _m1.m[row][0]*_m2.m[0][col] + _m1.m[row][1]*_m2.m[1][col] + \
                               _m1.m[row][2]*_m2.m[2][col] + _m1.m[row][3]*_m2.m[3][col];  \
    }                                                                         \
} while(0)
    /* Define a matrix that first focuses on Z, then rotates around X then Y to
     * focus the output in the direction of the vector.
     */
    MATRIX_MULT(tmp1, xrot, zfocus);
    MATRIX_MULT(tmp2, yrot, tmp1);
#undef MATRIX_MULT

    return tmp2;
}

// Update the early and late 3D panning gains.
static ALvoid Update3DPanning(const ALCdevice *Device, const ALfloat *ReflectionsPan, const ALfloat *LateReverbPan, ALfloat Gain, ALfloat EarlyGain, ALfloat LateGain, ALreverbState *State)
{
    /* Converts early reflections A-Format to B-Format (transposed). */
    static const aluMatrixf EarlyA2B = {{
        { 0.8660254038f,  0.8660254038f,  0.8660254038f,  0.8660254038f },
        { 0.8660254038f,  0.8660254038f, -0.8660254038f, -0.8660254038f },
        { 0.8660254038f, -0.8660254038f,  0.8660254038f, -0.8660254038f },
        { 0.8660254038f, -0.8660254038f, -0.8660254038f,  0.8660254038f }
    }};
    /* Converts late reverb A-Format to B-Format (transposed). */
    static const aluMatrixf LateA2B = {{
        { 0.8660254038f, -0.8660254038f,  0.8660254038f,  0.8660254038f },
        { 0.8660254038f, -0.8660254038f, -0.8660254038f, -0.8660254038f },
        { 0.8660254038f,  0.8660254038f,  0.8660254038f, -0.8660254038f },
        { 0.8660254038f,  0.8660254038f, -0.8660254038f,  0.8660254038f }
/*        { 0.8660254038f,  1.2247448714f,           0.0f,  0.8660254038f },
        { 0.8660254038f,           0.0f, -1.2247448714f, -0.8660254038f },
        { 0.8660254038f,           0.0f,  1.2247448714f, -0.8660254038f },
        { 0.8660254038f, -1.2247448714f,           0.0f,  0.8660254038f }*/
    }};
    aluMatrixf transform, rot;
    ALuint i;

    STATIC_CAST(ALeffectState,State)->OutBuffer = Device->FOAOut.Buffer;
    STATIC_CAST(ALeffectState,State)->OutChannels = Device->FOAOut.NumChannels;

    /* Note: Both _m2 and _res are transposed. */
#define MATRIX_MULT(_res, _m1, _m2) do {                                      \
    int row, col;                                                             \
    for(col = 0;col < 4;col++)                                                \
    {                                                                         \
        for(row = 0;row < 4;row++)                                            \
            _res.m[col][row] = _m1.m[row][0]*_m2.m[col][0] + _m1.m[row][1]*_m2.m[col][1] + \
                               _m1.m[row][2]*_m2.m[col][2] + _m1.m[row][3]*_m2.m[col][3];  \
    }                                                                         \
} while(0)
    /* Create a matrix that first converts A-Format to B-Format, then rotates
     * the B-Format soundfield according to the panning vector.
     */
    rot = GetTransformFromVector(ReflectionsPan);
    MATRIX_MULT(transform, rot, EarlyA2B);
    memset(&State->Early.PanGain, 0, sizeof(State->Early.PanGain));
    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
        ComputeFirstOrderGains(Device->FOAOut, transform.m[i], Gain*EarlyGain, State->Early.PanGain[i]);

    rot = GetTransformFromVector(LateReverbPan);
    MATRIX_MULT(transform, rot, LateA2B);
    memset(&State->Late.PanGain, 0, sizeof(State->Late.PanGain));
    for(i = 0;i < MAX_EFFECT_CHANNELS;i++)
        ComputeFirstOrderGains(Device->FOAOut, transform.m[i], Gain*LateGain, State->Late.PanGain[i]);
#undef MATRIX_MULT
}

static ALvoid ALreverbState_update(ALreverbState *State, const ALCdevice *Device, const ALeffectslot *Slot, const ALeffectProps *props)
{
    ALuint frequency = Device->Frequency;
    ALfloat lfscale, hfscale, hfRatio;
    ALfloat gain, gainlf, gainhf;
    ALfloat cw, x, y;
    ALuint i;

    if(Slot->Params.EffectType == AL_EFFECT_EAXREVERB && !EmulateEAXReverb)
        State->IsEax = AL_TRUE;
    else if(Slot->Params.EffectType == AL_EFFECT_REVERB || EmulateEAXReverb)
        State->IsEax = AL_FALSE;

    // Calculate the master filters
    hfscale = props->Reverb.HFReference / frequency;
    gainhf = maxf(props->Reverb.GainHF, 0.0001f);
    ALfilterState_setParams(&State->Filter[0].Lp, ALfilterType_HighShelf,
                            gainhf, hfscale, calc_rcpQ_from_slope(gainhf, 0.75f));
    lfscale = props->Reverb.LFReference / frequency;
    gainlf = maxf(props->Reverb.GainLF, 0.0001f);
    ALfilterState_setParams(&State->Filter[0].Hp, ALfilterType_LowShelf,
                            gainlf, lfscale, calc_rcpQ_from_slope(gainlf, 0.75f));
    for(i = 1;i < 4;i++)
    {
        State->Filter[i].Lp.a1 = State->Filter[0].Lp.a1;
        State->Filter[i].Lp.a2 = State->Filter[0].Lp.a2;
        State->Filter[i].Lp.b0 = State->Filter[0].Lp.b0;
        State->Filter[i].Lp.b1 = State->Filter[0].Lp.b1;
        State->Filter[i].Lp.b2 = State->Filter[0].Lp.b2;

        State->Filter[i].Hp.a1 = State->Filter[0].Hp.a1;
        State->Filter[i].Hp.a2 = State->Filter[0].Hp.a2;
        State->Filter[i].Hp.b0 = State->Filter[0].Hp.b0;
        State->Filter[i].Hp.b1 = State->Filter[0].Hp.b1;
        State->Filter[i].Hp.b2 = State->Filter[0].Hp.b2;
    }

    // Update the modulator line.
    UpdateModulator(props->Reverb.ModulationTime, props->Reverb.ModulationDepth,
                    frequency, State);

    // Update the main effect delay.
    UpdateDelayLine(props->Reverb.ReflectionsDelay, props->Reverb.LateReverbDelay,
                    props->Reverb.Density, frequency, State);

    // Update the early lines.
    UpdateEarlyLines(props->Reverb.LateReverbDelay, State);

    // Get the mixing matrix coefficients (x and y).
    CalcMatrixCoeffs(props->Reverb.Diffusion, &x, &y);
    // Then divide x into y to simplify the matrix calculation.
    State->Late.MixCoeff = y / x;

    // If the HF limit parameter is flagged, calculate an appropriate limit
    // based on the air absorption parameter.
    hfRatio = props->Reverb.DecayHFRatio;
    if(props->Reverb.DecayHFLimit && props->Reverb.AirAbsorptionGainHF < 1.0f)
        hfRatio = CalcLimitedHfRatio(hfRatio, props->Reverb.AirAbsorptionGainHF,
                                     props->Reverb.DecayTime);

    cw = cosf(F_TAU * hfscale);
    // Update the late lines.
    UpdateLateLines(x, props->Reverb.Density, props->Reverb.DecayTime,
                    props->Reverb.Diffusion, props->Reverb.EchoDepth,
                    hfRatio, cw, frequency, State);

    // Update the echo line.
    UpdateEchoLine(props->Reverb.EchoTime, props->Reverb.DecayTime,
                   props->Reverb.Diffusion, props->Reverb.EchoDepth,
                   hfRatio, cw, frequency, State);

    gain = props->Reverb.Gain * Slot->Params.Gain * ReverbBoost;
    // Update early and late 3D panning.
    Update3DPanning(Device, props->Reverb.ReflectionsPan,
                    props->Reverb.LateReverbPan, gain,
                    props->Reverb.ReflectionsGain,
                    props->Reverb.LateReverbGain, State);
}


/**************************************
 *  Effect Processing                 *
 **************************************/

// Basic delay line input/output routines.
static inline ALfloat DelayLineOut(DelayLine *Delay, ALuint offset)
{
    return Delay->Line[offset&Delay->Mask];
}

static inline ALvoid DelayLineIn(DelayLine *Delay, ALuint offset, ALfloat in)
{
    Delay->Line[offset&Delay->Mask] = in;
}

static inline ALfloat DelayLineInOut(DelayLine *Delay, ALuint offset, ALuint outoffset, ALfloat in)
{
    Delay->Line[offset&Delay->Mask] = in;
    return Delay->Line[(offset-outoffset)&Delay->Mask];
}

static void CalcModulationDelays(ALreverbState *State, ALfloat *restrict delays, ALuint todo)
{
    ALfloat sinus, range;
    ALuint index, i;

    index = State->Mod.Index;
    range = State->Mod.Filter;
    for(i = 0;i < todo;i++)
    {
        /* Calculate the sinus rythm (dependent on modulation time and the
         * sampling rate).  The center of the sinus is moved to reduce the
         * delay of the effect when the time or depth are low.
         */
        sinus = 1.0f - cosf(F_TAU * index / State->Mod.Range);

        /* Step the modulation index forward, keeping it bound to its range. */
        index = (index+1) % State->Mod.Range;

        /* The depth determines the range over which to read the input samples
         * from, so it must be filtered to reduce the distortion caused by even
         * small parameter changes.
         */
        range = lerp(range, State->Mod.Depth, State->Mod.Coeff);

        /* Calculate the read offset with fraction. */
        delays[i] = range*sinus;
    }
    State->Mod.Index = index;
    State->Mod.Filter = range;
}

// Given some input samples, this function produces modulation for the late
// reverb.
static void EAXModulation(DelayLine *ModDelay, ALuint offset, const ALfloat *restrict delays, ALfloat*restrict dst, const ALfloat*restrict src, ALuint todo)
{
    ALfloat frac, fdelay;
    ALfloat out0, out1;
    ALuint delay, i;

    for(i = 0;i < todo;i++)
    {
        /* Separate the integer offset and fraction between it and the next
         * sample.
         */
        frac = modff(delays[i], &fdelay);
        delay = fastf2u(fdelay);

        /* Add the incoming sample to the delay line, and get the two samples
         * crossed by the offset delay.
         */
        out0 = DelayLineInOut(ModDelay, offset, delay, src[i]);
        out1 = DelayLineOut(ModDelay, offset - delay - 1);
        offset++;

        /* The output is obtained by linearly interpolating the two samples
         * that were acquired above.
         */
        dst[i] = lerp(out0, out1, frac);
    }
}

/* Given some input samples from the main delay line, this function produces
 * four-channel outputs for the early reflections.
 */
static ALvoid EarlyReflection(ALreverbState *State, ALuint todo, ALfloat (*restrict out)[MAX_UPDATE_SAMPLES])
{
    ALfloat d[4], v, f[4];
    ALuint i;

    for(i = 0;i < todo;i++)
    {
        ALuint offset = State->Offset+i;

        /* Obtain the first reflection samples from the main delay line. */
        f[0] = DelayLineOut(&State->Delay, (offset-State->EarlyDelayTap[0])*4 + 0);
        f[1] = DelayLineOut(&State->Delay, (offset-State->EarlyDelayTap[1])*4 + 1);
        f[2] = DelayLineOut(&State->Delay, (offset-State->EarlyDelayTap[2])*4 + 2);
        f[3] = DelayLineOut(&State->Delay, (offset-State->EarlyDelayTap[3])*4 + 3);

        /* The following uses a lossless scattering junction from waveguide
         * theory.  It actually amounts to a householder mixing matrix, which
         * will produce a maximally diffuse response, and means this can
         * probably be considered a simple feed-back delay network (FDN).
         *          N
         *         ---
         *         \
         * v = 2/N /   d_i
         *         ---
         *         i=1
         */
        v = (f[0] + f[1] + f[2] + f[3]) * 0.5f;

        /* Calculate the feed values for the early delay lines. */
        d[0] = v - f[0];
        d[1] = v - f[1];
        d[2] = v - f[2];
        d[3] = v - f[3];

        /* Feed the early delay lines, and load the delayed results. */
        d[0] = DelayLineInOut(&State->Early.Delay[0], offset, State->Early.Offset[0], d[0]);
        d[1] = DelayLineInOut(&State->Early.Delay[1], offset, State->Early.Offset[1], d[1]);
        d[2] = DelayLineInOut(&State->Early.Delay[2], offset, State->Early.Offset[2], d[2]);
        d[3] = DelayLineInOut(&State->Early.Delay[3], offset, State->Early.Offset[3], d[3]);

        /* Output the initial reflection taps and the results of the delayed
         * and decayed junction for all four channels.
         */
        out[0][i] = f[0] + d[0]*State->Early.Coeff[0];
        out[1][i] = f[1] + d[1]*State->Early.Coeff[1];
        out[2][i] = f[2] + d[2]*State->Early.Coeff[2];
        out[3][i] = f[3] + d[3]*State->Early.Coeff[3];
    }
}

// Basic attenuated all-pass input/output routine.
static inline ALfloat AllpassInOut(DelayLine *Delay, ALuint outOffset, ALuint inOffset, ALfloat in, ALfloat feedCoeff, ALfloat coeff)
{
    ALfloat out, feed;

    out = DelayLineOut(Delay, outOffset);
    feed = feedCoeff * in;
    DelayLineIn(Delay, inOffset, (feedCoeff * (out - feed)) + in);

    // The time-based attenuation is only applied to the delay output to
    // keep it from affecting the feed-back path (which is already controlled
    // by the all-pass feed coefficient).
    return (coeff * out) - feed;
}

// All-pass input/output routine for late reverb.
static inline ALfloat LateAllPassInOut(ALreverbState *State, ALuint offset, ALuint index, ALfloat in)
{
    return AllpassInOut(&State->Late.Ap[index].Delay,
                        offset - State->Late.Ap[index].Offset,
                        offset, in, State->Late.ApFeedCoeff,
                        State->Late.Ap[index].Coeff);
}

// Low-pass filter input/output routine for late reverb.
static inline ALfloat LateLowPassInOut(ALreverbState *State, ALuint index, ALfloat in)
{
    in = lerp(in, State->Late.Lp[index].Sample, State->Late.Lp[index].Coeff);
    State->Late.Lp[index].Sample = in;
    return in;
}

// Given four decorrelated input samples, this function produces four-channel
// output for the late reverb.
static ALvoid LateReverb(ALreverbState *State, ALuint todo, ALfloat (*restrict out)[MAX_UPDATE_SAMPLES])
{
    ALfloat d[4], f[4];
    ALuint offset;
    ALuint base, i;

    offset = State->Offset;
    for(base = 0;base < todo;)
    {
        ALfloat tmp[MAX_UPDATE_SAMPLES/4][4];
        ALuint tmp_todo = minu(todo, MAX_UPDATE_SAMPLES/4);

        for(i = 0;i < tmp_todo;i++)
        {
            /* Obtain four decorrelated input samples. */
            f[0] = DelayLineOut(&State->Delay, (offset-State->LateDelayTap[0])*4 + 0) * State->Late.DensityGain;
            f[1] = DelayLineOut(&State->Delay, (offset-State->LateDelayTap[1])*4 + 1) * State->Late.DensityGain;
            f[2] = DelayLineOut(&State->Delay, (offset-State->LateDelayTap[2])*4 + 2) * State->Late.DensityGain;
            f[3] = DelayLineOut(&State->Delay, (offset-State->LateDelayTap[3])*4 + 3) * State->Late.DensityGain;

            /* Add the decayed results of the cyclical delay lines, then pass
             * the results through the low-pass filters.
             */
            f[0] += DelayLineOut(&State->Late.Delay[0], offset-State->Late.Offset[0]) * State->Late.Coeff[0];
            f[1] += DelayLineOut(&State->Late.Delay[1], offset-State->Late.Offset[1]) * State->Late.Coeff[1];
            f[2] += DelayLineOut(&State->Late.Delay[2], offset-State->Late.Offset[2]) * State->Late.Coeff[2];
            f[3] += DelayLineOut(&State->Late.Delay[3], offset-State->Late.Offset[3]) * State->Late.Coeff[3];

            /* This is where the feed-back cycles from line 0 to 3 to 1 to 2
             * and back to 0.
             */
            d[0] = LateLowPassInOut(State, 2, f[2]);
            d[1] = LateLowPassInOut(State, 3, f[3]);
            d[2] = LateLowPassInOut(State, 1, f[1]);
            d[3] = LateLowPassInOut(State, 0, f[0]);

            /* To help increase diffusion, run each line through an all-pass
             * filter. When there is no diffusion, the shortest all-pass filter
             * will feed the shortest delay line.
             */
            d[0] = LateAllPassInOut(State, offset, 0, d[0]);
            d[1] = LateAllPassInOut(State, offset, 1, d[1]);
            d[2] = LateAllPassInOut(State, offset, 2, d[2]);
            d[3] = LateAllPassInOut(State, offset, 3, d[3]);

            /* Late reverb is done with a modified feed-back delay network (FDN)
             * topology.  Four input lines are each fed through their own all-pass
             * filter and then into the mixing matrix.  The four outputs of the
             * mixing matrix are then cycled back to the inputs.  Each output feeds
             * a different input to form a circlular feed cycle.
             *
             * The mixing matrix used is a 4D skew-symmetric rotation matrix
             * derived using a single unitary rotational parameter:
             *
             *  [  d,  a,  b,  c ]          1 = a^2 + b^2 + c^2 + d^2
             *  [ -a,  d,  c, -b ]
             *  [ -b, -c,  d,  a ]
             *  [ -c,  b, -a,  d ]
             *
             * The rotation is constructed from the effect's diffusion parameter,
             * yielding: 1 = x^2 + 3 y^2; where a, b, and c are the coefficient y
             * with differing signs, and d is the coefficient x.  The matrix is
             * thus:
             *
             *  [  x,  y, -y,  y ]          n = sqrt(matrix_order - 1)
             *  [ -y,  x,  y,  y ]          t = diffusion_parameter * atan(n)
             *  [  y, -y,  x,  y ]          x = cos(t)
             *  [ -y, -y, -y,  x ]          y = sin(t) / n
             *
             * To reduce the number of multiplies, the x coefficient is applied
             * with the cyclical delay line coefficients. Thus only the y
             * coefficient is applied when mixing, and is modified to be: y / x.
             */
            f[0] = d[0] + (State->Late.MixCoeff * (         d[1] + -d[2] + d[3]));
            f[1] = d[1] + (State->Late.MixCoeff * (-d[0]         +  d[2] + d[3]));
            f[2] = d[2] + (State->Late.MixCoeff * ( d[0] + -d[1]         + d[3]));
            f[3] = d[3] + (State->Late.MixCoeff * (-d[0] + -d[1] + -d[2]       ));

            /* Re-feed the cyclical delay lines. */
            DelayLineIn(&State->Late.Delay[0], offset, f[0]);
            DelayLineIn(&State->Late.Delay[1], offset, f[1]);
            DelayLineIn(&State->Late.Delay[2], offset, f[2]);
            DelayLineIn(&State->Late.Delay[3], offset, f[3]);
            offset++;

            /* Output the results of the matrix for all four channels,
             * attenuated by the late reverb gain (which is attenuated by the
             * 'x' mix coefficient).
             */
            tmp[i][0] = State->Late.Gain * f[0];
            tmp[i][1] = State->Late.Gain * f[1];
            tmp[i][2] = State->Late.Gain * f[2];
            tmp[i][3] = State->Late.Gain * f[3];
        }

        /* Deinterlace to output */
        for(i = 0;i < tmp_todo;i++) out[0][base+i] = tmp[i][0];
        for(i = 0;i < tmp_todo;i++) out[1][base+i] = tmp[i][1];
        for(i = 0;i < tmp_todo;i++) out[2][base+i] = tmp[i][2];
        for(i = 0;i < tmp_todo;i++) out[3][base+i] = tmp[i][3];

        base += tmp_todo;
    }
}

// Given an input sample, this function mixes echo into the four-channel late
// reverb.
static ALvoid EAXEcho(ALreverbState *State, ALuint todo, ALfloat (*restrict late)[MAX_UPDATE_SAMPLES])
{
    ALfloat feed;
    ALuint offset;
    ALuint c, i;

    for(c = 0;c < 4;c++)
    {
        offset = State->Offset;
        for(i = 0;i < todo;i++)
        {
            // Get the latest attenuated echo sample for output.
            feed = DelayLineOut(&State->Echo.Delay[c].Feedback, offset-State->Echo.Offset) *
                   State->Echo.Coeff;

            // Write the output.
            late[c][i] += State->Echo.MixCoeff * feed;

            // Mix the energy-attenuated input with the output and pass it through
            // the echo low-pass filter.
            feed += DelayLineOut(&State->Delay, (offset-State->LateDelayTap[0])*4 + c) *
                    State->Echo.DensityGain;
            feed = lerp(feed, State->Echo.LpSample[c], State->Echo.LpCoeff);
            State->Echo.LpSample[c] = feed;

            // Then the echo all-pass filter.
            feed = AllpassInOut(&State->Echo.Delay[c].Ap, offset-State->Echo.ApOffset,
                                offset, feed, State->Echo.ApFeedCoeff,
                                State->Echo.ApCoeff);

            // Feed the delay with the mixed and filtered sample.
            DelayLineIn(&State->Echo.Delay[c].Feedback, offset, feed);
            offset++;
        }
    }
}

// Perform the non-EAX reverb pass on a given input sample, resulting in
// four-channel output.
static ALvoid VerbPass(ALreverbState *State, ALuint todo, ALfloat (*restrict input)[MAX_UPDATE_SAMPLES], ALfloat (*restrict early)[MAX_UPDATE_SAMPLES], ALfloat (*restrict late)[MAX_UPDATE_SAMPLES])
{
    ALuint i, c;

    for(c = 0;c < 4;c++)
    {
        /* Low-pass filter the incoming samples (use the early buffer as temp
         * storage).
         */
        ALfilterState_process(&State->Filter[c].Lp, &early[0][0], input[c], todo);
        for(i = 0;i < todo;i++)
            DelayLineIn(&State->Delay, (State->Offset+i)*4 + c, early[0][i]);
    }

    // Calculate the early reflection from the first delay tap.
    EarlyReflection(State, todo, early);

    // Calculate the late reverb from the decorrelator taps.
    LateReverb(State, todo, late);

    // Step all delays forward one sample.
    State->Offset += todo;
}

// Perform the EAX reverb pass on a given input sample, resulting in four-
// channel output.
static ALvoid EAXVerbPass(ALreverbState *State, ALuint todo, ALfloat (*restrict input)[MAX_UPDATE_SAMPLES], ALfloat (*restrict early)[MAX_UPDATE_SAMPLES], ALfloat (*restrict late)[MAX_UPDATE_SAMPLES])
{
    ALuint i, c;

    /* Perform any modulation on the input (use the early and late buffers as
     * temp storage).
     */
    CalcModulationDelays(State, &late[0][0], todo);
    for(c = 0;c < 4;c++)
    {
        EAXModulation(&State->Mod.Delay[c], State->Offset, &late[0][0],
                      &early[0][0], input[c], todo);

        /* Band-pass the incoming samples */
        ALfilterState_process(&State->Filter[c].Lp, &early[1][0], &early[0][0], todo);
        ALfilterState_process(&State->Filter[c].Hp, &early[2][0], &early[1][0], todo);

        /* Feed the initial delay line. */
        for(i = 0;i < todo;i++)
            DelayLineIn(&State->Delay, (State->Offset+i)*4 + c, early[2][i]);
    }

    // Calculate the early reflection from the first delay tap.
    EarlyReflection(State, todo, early);

    // Calculate the late reverb from the decorrelator taps.
    LateReverb(State, todo, late);

    // Calculate and mix in any echo.
    EAXEcho(State, todo, late);

    // Step all delays forward.
    State->Offset += todo;
}


static ALvoid ALreverbState_processStandard(ALreverbState *State, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    static const aluMatrixf B2A = {{
        { 0.288675134595f,  0.288675134595f,  0.288675134595f,  0.288675134595f },
        { 0.288675134595f,  0.288675134595f, -0.288675134595f, -0.288675134595f },
        { 0.288675134595f, -0.288675134595f,  0.288675134595f, -0.288675134595f },
        { 0.288675134595f, -0.288675134595f, -0.288675134595f,  0.288675134595f }
    }};
    ALfloat (*restrict afmt)[MAX_UPDATE_SAMPLES] = State->AFormatSamples;
    ALfloat (*restrict early)[MAX_UPDATE_SAMPLES] = State->EarlySamples;
    ALfloat (*restrict late)[MAX_UPDATE_SAMPLES] = State->ReverbSamples;
    ALuint base, c;

    /* Process reverb for these samples. */
    for(base = 0;base < SamplesToDo;)
    {
        ALuint todo = minu(SamplesToDo-base, MAX_UPDATE_SAMPLES);

        /* Convert B-Foramt to A-Format for processing. */
        memset(afmt, 0, sizeof(*afmt)*4);
        for(c = 0;c < 4;c++)
            MixRowSamples(afmt[c], B2A.m[c],
                SamplesIn, MAX_EFFECT_CHANNELS, base, todo
            );

        VerbPass(State, todo, afmt, early, late);

        /* Mix the A-Format results to output, implicitly converting back to
         * B-Format.
         */
        for(c = 0;c < 4;c++)
            MixSamples(early[c], NumChannels, SamplesOut,
                State->Early.CurrentGain[c], State->Early.PanGain[c],
                SamplesToDo-base, base, todo
            );
        for(c = 0;c < 4;c++)
            MixSamples(late[c], NumChannels, SamplesOut,
                State->Late.CurrentGain[c], State->Late.PanGain[c],
                SamplesToDo-base, base, todo
            );

        base += todo;
    }
}

static ALvoid ALreverbState_processEax(ALreverbState *State, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    static const aluMatrixf B2A = {{
        { 0.288675134595f,  0.288675134595f,  0.288675134595f,  0.288675134595f },
        { 0.288675134595f,  0.288675134595f, -0.288675134595f, -0.288675134595f },
        { 0.288675134595f, -0.288675134595f,  0.288675134595f, -0.288675134595f },
        { 0.288675134595f, -0.288675134595f, -0.288675134595f,  0.288675134595f }
    }};
    ALfloat (*restrict afmt)[MAX_UPDATE_SAMPLES] = State->AFormatSamples;
    ALfloat (*restrict early)[MAX_UPDATE_SAMPLES] = State->EarlySamples;
    ALfloat (*restrict late)[MAX_UPDATE_SAMPLES] = State->ReverbSamples;
    ALuint base, c;

    /* Process reverb for these samples. */
    for(base = 0;base < SamplesToDo;)
    {
        ALuint todo = minu(SamplesToDo-base, MAX_UPDATE_SAMPLES);

        memset(afmt, 0, 4*MAX_UPDATE_SAMPLES*sizeof(float));
        for(c = 0;c < 4;c++)
            MixRowSamples(afmt[c], B2A.m[c],
                SamplesIn, MAX_EFFECT_CHANNELS, base, todo
            );

        EAXVerbPass(State, todo, afmt, early, late);

        for(c = 0;c < 4;c++)
            MixSamples(early[c], NumChannels, SamplesOut,
                State->Early.CurrentGain[c], State->Early.PanGain[c],
                SamplesToDo-base, base, todo
            );
        for(c = 0;c < 4;c++)
            MixSamples(late[c], NumChannels, SamplesOut,
                State->Late.CurrentGain[c], State->Late.PanGain[c],
                SamplesToDo-base, base, todo
            );

        base += todo;
    }
}

static ALvoid ALreverbState_process(ALreverbState *State, ALuint SamplesToDo, const ALfloat (*restrict SamplesIn)[BUFFERSIZE], ALfloat (*restrict SamplesOut)[BUFFERSIZE], ALuint NumChannels)
{
    if(State->IsEax)
        ALreverbState_processEax(State, SamplesToDo, SamplesIn, SamplesOut, NumChannels);
    else
        ALreverbState_processStandard(State, SamplesToDo, SamplesIn, SamplesOut, NumChannels);
}


typedef struct ALreverbStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALreverbStateFactory;

static ALeffectState *ALreverbStateFactory_create(ALreverbStateFactory* UNUSED(factory))
{
    ALreverbState *state;

    alcall_once(&mixfunc_inited, init_mixfunc);

    NEW_OBJ0(state, ALreverbState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALreverbStateFactory);

ALeffectStateFactory *ALreverbStateFactory_getFactory(void)
{
    static ALreverbStateFactory ReverbFactory = { { GET_VTABLE2(ALreverbStateFactory, ALeffectStateFactory) } };

    return STATIC_CAST(ALeffectStateFactory, &ReverbFactory);
}


void ALeaxreverb_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_DECAY_HFLIMIT:
            if(!(val >= AL_EAXREVERB_MIN_DECAY_HFLIMIT && val <= AL_EAXREVERB_MAX_DECAY_HFLIMIT))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayHFLimit = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALeaxreverb_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALeaxreverb_setParami(effect, context, param, vals[0]);
}
void ALeaxreverb_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_DENSITY:
            if(!(val >= AL_EAXREVERB_MIN_DENSITY && val <= AL_EAXREVERB_MAX_DENSITY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Density = val;
            break;

        case AL_EAXREVERB_DIFFUSION:
            if(!(val >= AL_EAXREVERB_MIN_DIFFUSION && val <= AL_EAXREVERB_MAX_DIFFUSION))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Diffusion = val;
            break;

        case AL_EAXREVERB_GAIN:
            if(!(val >= AL_EAXREVERB_MIN_GAIN && val <= AL_EAXREVERB_MAX_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Gain = val;
            break;

        case AL_EAXREVERB_GAINHF:
            if(!(val >= AL_EAXREVERB_MIN_GAINHF && val <= AL_EAXREVERB_MAX_GAINHF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.GainHF = val;
            break;

        case AL_EAXREVERB_GAINLF:
            if(!(val >= AL_EAXREVERB_MIN_GAINLF && val <= AL_EAXREVERB_MAX_GAINLF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.GainLF = val;
            break;

        case AL_EAXREVERB_DECAY_TIME:
            if(!(val >= AL_EAXREVERB_MIN_DECAY_TIME && val <= AL_EAXREVERB_MAX_DECAY_TIME))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayTime = val;
            break;

        case AL_EAXREVERB_DECAY_HFRATIO:
            if(!(val >= AL_EAXREVERB_MIN_DECAY_HFRATIO && val <= AL_EAXREVERB_MAX_DECAY_HFRATIO))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayHFRatio = val;
            break;

        case AL_EAXREVERB_DECAY_LFRATIO:
            if(!(val >= AL_EAXREVERB_MIN_DECAY_LFRATIO && val <= AL_EAXREVERB_MAX_DECAY_LFRATIO))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayLFRatio = val;
            break;

        case AL_EAXREVERB_REFLECTIONS_GAIN:
            if(!(val >= AL_EAXREVERB_MIN_REFLECTIONS_GAIN && val <= AL_EAXREVERB_MAX_REFLECTIONS_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ReflectionsGain = val;
            break;

        case AL_EAXREVERB_REFLECTIONS_DELAY:
            if(!(val >= AL_EAXREVERB_MIN_REFLECTIONS_DELAY && val <= AL_EAXREVERB_MAX_REFLECTIONS_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ReflectionsDelay = val;
            break;

        case AL_EAXREVERB_LATE_REVERB_GAIN:
            if(!(val >= AL_EAXREVERB_MIN_LATE_REVERB_GAIN && val <= AL_EAXREVERB_MAX_LATE_REVERB_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LateReverbGain = val;
            break;

        case AL_EAXREVERB_LATE_REVERB_DELAY:
            if(!(val >= AL_EAXREVERB_MIN_LATE_REVERB_DELAY && val <= AL_EAXREVERB_MAX_LATE_REVERB_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LateReverbDelay = val;
            break;

        case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
            if(!(val >= AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF && val <= AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.AirAbsorptionGainHF = val;
            break;

        case AL_EAXREVERB_ECHO_TIME:
            if(!(val >= AL_EAXREVERB_MIN_ECHO_TIME && val <= AL_EAXREVERB_MAX_ECHO_TIME))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.EchoTime = val;
            break;

        case AL_EAXREVERB_ECHO_DEPTH:
            if(!(val >= AL_EAXREVERB_MIN_ECHO_DEPTH && val <= AL_EAXREVERB_MAX_ECHO_DEPTH))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.EchoDepth = val;
            break;

        case AL_EAXREVERB_MODULATION_TIME:
            if(!(val >= AL_EAXREVERB_MIN_MODULATION_TIME && val <= AL_EAXREVERB_MAX_MODULATION_TIME))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ModulationTime = val;
            break;

        case AL_EAXREVERB_MODULATION_DEPTH:
            if(!(val >= AL_EAXREVERB_MIN_MODULATION_DEPTH && val <= AL_EAXREVERB_MAX_MODULATION_DEPTH))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ModulationDepth = val;
            break;

        case AL_EAXREVERB_HFREFERENCE:
            if(!(val >= AL_EAXREVERB_MIN_HFREFERENCE && val <= AL_EAXREVERB_MAX_HFREFERENCE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.HFReference = val;
            break;

        case AL_EAXREVERB_LFREFERENCE:
            if(!(val >= AL_EAXREVERB_MIN_LFREFERENCE && val <= AL_EAXREVERB_MAX_LFREFERENCE))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LFReference = val;
            break;

        case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
            if(!(val >= AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR && val <= AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.RoomRolloffFactor = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALeaxreverb_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_REFLECTIONS_PAN:
            if(!(isfinite(vals[0]) && isfinite(vals[1]) && isfinite(vals[2])))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ReflectionsPan[0] = vals[0];
            props->Reverb.ReflectionsPan[1] = vals[1];
            props->Reverb.ReflectionsPan[2] = vals[2];
            break;
        case AL_EAXREVERB_LATE_REVERB_PAN:
            if(!(isfinite(vals[0]) && isfinite(vals[1]) && isfinite(vals[2])))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LateReverbPan[0] = vals[0];
            props->Reverb.LateReverbPan[1] = vals[1];
            props->Reverb.LateReverbPan[2] = vals[2];
            break;

        default:
            ALeaxreverb_setParamf(effect, context, param, vals[0]);
            break;
    }
}

void ALeaxreverb_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_DECAY_HFLIMIT:
            *val = props->Reverb.DecayHFLimit;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALeaxreverb_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALeaxreverb_getParami(effect, context, param, vals);
}
void ALeaxreverb_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_DENSITY:
            *val = props->Reverb.Density;
            break;

        case AL_EAXREVERB_DIFFUSION:
            *val = props->Reverb.Diffusion;
            break;

        case AL_EAXREVERB_GAIN:
            *val = props->Reverb.Gain;
            break;

        case AL_EAXREVERB_GAINHF:
            *val = props->Reverb.GainHF;
            break;

        case AL_EAXREVERB_GAINLF:
            *val = props->Reverb.GainLF;
            break;

        case AL_EAXREVERB_DECAY_TIME:
            *val = props->Reverb.DecayTime;
            break;

        case AL_EAXREVERB_DECAY_HFRATIO:
            *val = props->Reverb.DecayHFRatio;
            break;

        case AL_EAXREVERB_DECAY_LFRATIO:
            *val = props->Reverb.DecayLFRatio;
            break;

        case AL_EAXREVERB_REFLECTIONS_GAIN:
            *val = props->Reverb.ReflectionsGain;
            break;

        case AL_EAXREVERB_REFLECTIONS_DELAY:
            *val = props->Reverb.ReflectionsDelay;
            break;

        case AL_EAXREVERB_LATE_REVERB_GAIN:
            *val = props->Reverb.LateReverbGain;
            break;

        case AL_EAXREVERB_LATE_REVERB_DELAY:
            *val = props->Reverb.LateReverbDelay;
            break;

        case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
            *val = props->Reverb.AirAbsorptionGainHF;
            break;

        case AL_EAXREVERB_ECHO_TIME:
            *val = props->Reverb.EchoTime;
            break;

        case AL_EAXREVERB_ECHO_DEPTH:
            *val = props->Reverb.EchoDepth;
            break;

        case AL_EAXREVERB_MODULATION_TIME:
            *val = props->Reverb.ModulationTime;
            break;

        case AL_EAXREVERB_MODULATION_DEPTH:
            *val = props->Reverb.ModulationDepth;
            break;

        case AL_EAXREVERB_HFREFERENCE:
            *val = props->Reverb.HFReference;
            break;

        case AL_EAXREVERB_LFREFERENCE:
            *val = props->Reverb.LFReference;
            break;

        case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
            *val = props->Reverb.RoomRolloffFactor;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALeaxreverb_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_EAXREVERB_REFLECTIONS_PAN:
            vals[0] = props->Reverb.ReflectionsPan[0];
            vals[1] = props->Reverb.ReflectionsPan[1];
            vals[2] = props->Reverb.ReflectionsPan[2];
            break;
        case AL_EAXREVERB_LATE_REVERB_PAN:
            vals[0] = props->Reverb.LateReverbPan[0];
            vals[1] = props->Reverb.LateReverbPan[1];
            vals[2] = props->Reverb.LateReverbPan[2];
            break;

        default:
            ALeaxreverb_getParamf(effect, context, param, vals);
            break;
    }
}

DEFINE_ALEFFECT_VTABLE(ALeaxreverb);

void ALreverb_setParami(ALeffect *effect, ALCcontext *context, ALenum param, ALint val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_REVERB_DECAY_HFLIMIT:
            if(!(val >= AL_REVERB_MIN_DECAY_HFLIMIT && val <= AL_REVERB_MAX_DECAY_HFLIMIT))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayHFLimit = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALreverb_setParamiv(ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals)
{
    ALreverb_setParami(effect, context, param, vals[0]);
}
void ALreverb_setParamf(ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val)
{
    ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_REVERB_DENSITY:
            if(!(val >= AL_REVERB_MIN_DENSITY && val <= AL_REVERB_MAX_DENSITY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Density = val;
            break;

        case AL_REVERB_DIFFUSION:
            if(!(val >= AL_REVERB_MIN_DIFFUSION && val <= AL_REVERB_MAX_DIFFUSION))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Diffusion = val;
            break;

        case AL_REVERB_GAIN:
            if(!(val >= AL_REVERB_MIN_GAIN && val <= AL_REVERB_MAX_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.Gain = val;
            break;

        case AL_REVERB_GAINHF:
            if(!(val >= AL_REVERB_MIN_GAINHF && val <= AL_REVERB_MAX_GAINHF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.GainHF = val;
            break;

        case AL_REVERB_DECAY_TIME:
            if(!(val >= AL_REVERB_MIN_DECAY_TIME && val <= AL_REVERB_MAX_DECAY_TIME))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayTime = val;
            break;

        case AL_REVERB_DECAY_HFRATIO:
            if(!(val >= AL_REVERB_MIN_DECAY_HFRATIO && val <= AL_REVERB_MAX_DECAY_HFRATIO))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.DecayHFRatio = val;
            break;

        case AL_REVERB_REFLECTIONS_GAIN:
            if(!(val >= AL_REVERB_MIN_REFLECTIONS_GAIN && val <= AL_REVERB_MAX_REFLECTIONS_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ReflectionsGain = val;
            break;

        case AL_REVERB_REFLECTIONS_DELAY:
            if(!(val >= AL_REVERB_MIN_REFLECTIONS_DELAY && val <= AL_REVERB_MAX_REFLECTIONS_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.ReflectionsDelay = val;
            break;

        case AL_REVERB_LATE_REVERB_GAIN:
            if(!(val >= AL_REVERB_MIN_LATE_REVERB_GAIN && val <= AL_REVERB_MAX_LATE_REVERB_GAIN))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LateReverbGain = val;
            break;

        case AL_REVERB_LATE_REVERB_DELAY:
            if(!(val >= AL_REVERB_MIN_LATE_REVERB_DELAY && val <= AL_REVERB_MAX_LATE_REVERB_DELAY))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.LateReverbDelay = val;
            break;

        case AL_REVERB_AIR_ABSORPTION_GAINHF:
            if(!(val >= AL_REVERB_MIN_AIR_ABSORPTION_GAINHF && val <= AL_REVERB_MAX_AIR_ABSORPTION_GAINHF))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.AirAbsorptionGainHF = val;
            break;

        case AL_REVERB_ROOM_ROLLOFF_FACTOR:
            if(!(val >= AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR && val <= AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR))
                SET_ERROR_AND_RETURN(context, AL_INVALID_VALUE);
            props->Reverb.RoomRolloffFactor = val;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALreverb_setParamfv(ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals)
{
    ALreverb_setParamf(effect, context, param, vals[0]);
}

void ALreverb_getParami(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_REVERB_DECAY_HFLIMIT:
            *val = props->Reverb.DecayHFLimit;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALreverb_getParamiv(const ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals)
{
    ALreverb_getParami(effect, context, param, vals);
}
void ALreverb_getParamf(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val)
{
    const ALeffectProps *props = &effect->Props;
    switch(param)
    {
        case AL_REVERB_DENSITY:
            *val = props->Reverb.Density;
            break;

        case AL_REVERB_DIFFUSION:
            *val = props->Reverb.Diffusion;
            break;

        case AL_REVERB_GAIN:
            *val = props->Reverb.Gain;
            break;

        case AL_REVERB_GAINHF:
            *val = props->Reverb.GainHF;
            break;

        case AL_REVERB_DECAY_TIME:
            *val = props->Reverb.DecayTime;
            break;

        case AL_REVERB_DECAY_HFRATIO:
            *val = props->Reverb.DecayHFRatio;
            break;

        case AL_REVERB_REFLECTIONS_GAIN:
            *val = props->Reverb.ReflectionsGain;
            break;

        case AL_REVERB_REFLECTIONS_DELAY:
            *val = props->Reverb.ReflectionsDelay;
            break;

        case AL_REVERB_LATE_REVERB_GAIN:
            *val = props->Reverb.LateReverbGain;
            break;

        case AL_REVERB_LATE_REVERB_DELAY:
            *val = props->Reverb.LateReverbDelay;
            break;

        case AL_REVERB_AIR_ABSORPTION_GAINHF:
            *val = props->Reverb.AirAbsorptionGainHF;
            break;

        case AL_REVERB_ROOM_ROLLOFF_FACTOR:
            *val = props->Reverb.RoomRolloffFactor;
            break;

        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALreverb_getParamfv(const ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals)
{
    ALreverb_getParamf(effect, context, param, vals);
}

DEFINE_ALEFFECT_VTABLE(ALreverb);
