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

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alEffect.h"
#include "alError.h"


extern inline void LockEffectList(ALCdevice *device);
extern inline void UnlockEffectList(ALCdevice *device);
extern inline ALboolean IsReverbEffect(ALenum type);

const struct EffectList EffectList[EFFECTLIST_SIZE] = {
    { "eaxreverb",  EAXREVERB_EFFECT,  AL_EFFECT_EAXREVERB },
    { "reverb",     REVERB_EFFECT,     AL_EFFECT_REVERB },
    { "chorus",     CHORUS_EFFECT,     AL_EFFECT_CHORUS },
    { "compressor", COMPRESSOR_EFFECT, AL_EFFECT_COMPRESSOR },
    { "distortion", DISTORTION_EFFECT, AL_EFFECT_DISTORTION },
    { "echo",       ECHO_EFFECT,       AL_EFFECT_ECHO },
    { "equalizer",  EQUALIZER_EFFECT,  AL_EFFECT_EQUALIZER },
    { "flanger",    FLANGER_EFFECT,    AL_EFFECT_FLANGER },
    { "modulator",  MODULATOR_EFFECT,  AL_EFFECT_RING_MODULATOR },
    { "pshifter",   PSHIFTER_EFFECT,   AL_EFFECT_PITCH_SHIFTER },
    { "dedicated",  DEDICATED_EFFECT,  AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT },
    { "dedicated",  DEDICATED_EFFECT,  AL_EFFECT_DEDICATED_DIALOGUE },
};

ALboolean DisabledEffects[MAX_EFFECTS];

static ALeffect *AllocEffect(ALCcontext *context);
static void FreeEffect(ALCdevice *device, ALeffect *effect);
static void InitEffectParams(ALeffect *effect, ALenum type);

static inline ALeffect *LookupEffect(ALCdevice *device, ALuint id)
{
    EffectSubList *sublist;
    ALuint lidx = (id-1) >> 6;
    ALsizei slidx = (id-1) & 0x3f;

    if(UNLIKELY(lidx >= VECTOR_SIZE(device->EffectList)))
        return NULL;
    sublist = &VECTOR_ELEM(device->EffectList, lidx);
    if(UNLIKELY(sublist->FreeMask & (U64(1)<<slidx)))
        return NULL;
    return sublist->Effects + slidx;
}


AL_API ALvoid AL_APIENTRY alGenEffects(ALsizei n, ALuint *effects)
{
    ALCcontext *context;
    ALsizei cur;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        alSetError(context, AL_INVALID_VALUE, "Generating %d effects", n);
    else for(cur = 0;cur < n;cur++)
    {
        ALeffect *effect = AllocEffect(context);
        if(!effect)
        {
            alDeleteEffects(cur, effects);
            break;
        }
        effects[cur] = effect->id;
    }

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDeleteEffects(ALsizei n, const ALuint *effects)
{
    ALCdevice *device;
    ALCcontext *context;
    ALeffect *effect;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockEffectList(device);
    if(!(n >= 0))
        SETERR_GOTO(context, AL_INVALID_VALUE, done, "Deleting %d effects", n);
    for(i = 0;i < n;i++)
    {
        if(effects[i] && LookupEffect(device, effects[i]) == NULL)
            SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect ID %u", effects[i]);
    }
    for(i = 0;i < n;i++)
    {
        if((effect=LookupEffect(device, effects[i])) != NULL)
            FreeEffect(device, effect);
    }

done:
    UnlockEffectList(device);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsEffect(ALuint effect)
{
    ALCcontext *Context;
    ALboolean  result;

    Context = GetContextRef();
    if(!Context) return AL_FALSE;

    LockEffectList(Context->Device);
    result = ((!effect || LookupEffect(Context->Device, effect)) ?
              AL_TRUE : AL_FALSE);
    UnlockEffectList(Context->Device);

    ALCcontext_DecRef(Context);

    return result;
}

AL_API ALvoid AL_APIENTRY alEffecti(ALuint effect, ALenum param, ALint value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        if(param == AL_EFFECT_TYPE)
        {
            ALboolean isOk = (value == AL_EFFECT_NULL);
            ALint i;
            for(i = 0;!isOk && i < EFFECTLIST_SIZE;i++)
            {
                if(value == EffectList[i].val &&
                   !DisabledEffects[EffectList[i].type])
                    isOk = AL_TRUE;
            }

            if(isOk)
                InitEffectParams(ALEffect, value);
            else
                alSetError(Context, AL_INVALID_VALUE, "Effect type 0x%04x not supported", value);
        }
        else
        {
            /* Call the appropriate handler */
            ALeffect_setParami(ALEffect, Context, param, value);
        }
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alEffectiv(ALuint effect, ALenum param, const ALint *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    switch(param)
    {
        case AL_EFFECT_TYPE:
            alEffecti(effect, param, values[0]);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_setParamiv(ALEffect, Context, param, values);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alEffectf(ALuint effect, ALenum param, ALfloat value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_setParamf(ALEffect, Context, param, value);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alEffectfv(ALuint effect, ALenum param, const ALfloat *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_setParamfv(ALEffect, Context, param, values);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffecti(ALuint effect, ALenum param, ALint *value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        if(param == AL_EFFECT_TYPE)
            *value = ALEffect->type;
        else
        {
            /* Call the appropriate handler */
            ALeffect_getParami(ALEffect, Context, param, value);
        }
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectiv(ALuint effect, ALenum param, ALint *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    switch(param)
    {
        case AL_EFFECT_TYPE:
            alGetEffecti(effect, param, values);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_getParamiv(ALEffect, Context, param, values);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectf(ALuint effect, ALenum param, ALfloat *value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_getParamf(ALEffect, Context, param, value);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectfv(ALuint effect, ALenum param, ALfloat *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockEffectList(Device);
    if((ALEffect=LookupEffect(Device, effect)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid effect ID %u", effect);
    else
    {
        /* Call the appropriate handler */
        ALeffect_getParamfv(ALEffect, Context, param, values);
    }
    UnlockEffectList(Device);

    ALCcontext_DecRef(Context);
}


void InitEffect(ALeffect *effect)
{
    InitEffectParams(effect, AL_EFFECT_NULL);
}

static ALeffect *AllocEffect(ALCcontext *context)
{
    ALCdevice *device = context->Device;
    EffectSubList *sublist, *subend;
    ALeffect *effect = NULL;
    ALsizei lidx = 0;
    ALsizei slidx;

    almtx_lock(&device->EffectLock);
    sublist = VECTOR_BEGIN(device->EffectList);
    subend = VECTOR_END(device->EffectList);
    for(;sublist != subend;++sublist)
    {
        if(sublist->FreeMask)
        {
            slidx = CTZ64(sublist->FreeMask);
            effect = sublist->Effects + slidx;
            break;
        }
        ++lidx;
    }
    if(UNLIKELY(!effect))
    {
        const EffectSubList empty_sublist = { 0, NULL };
        /* Don't allocate so many list entries that the 32-bit ID could
         * overflow...
         */
        if(UNLIKELY(VECTOR_SIZE(device->EffectList) >= 1<<25))
        {
            almtx_unlock(&device->EffectLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Too many effects allocated");
            return NULL;
        }
        lidx = (ALsizei)VECTOR_SIZE(device->EffectList);
        VECTOR_PUSH_BACK(device->EffectList, empty_sublist);
        sublist = &VECTOR_BACK(device->EffectList);
        sublist->FreeMask = ~U64(0);
        sublist->Effects = al_calloc(16, sizeof(ALeffect)*64);
        if(UNLIKELY(!sublist->Effects))
        {
            VECTOR_POP_BACK(device->EffectList);
            almtx_unlock(&device->EffectLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Failed to allocate effect batch");
            return NULL;
        }

        slidx = 0;
        effect = sublist->Effects + slidx;
    }

    memset(effect, 0, sizeof(*effect));
    InitEffectParams(effect, AL_EFFECT_NULL);

    /* Add 1 to avoid effect ID 0. */
    effect->id = ((lidx<<6) | slidx) + 1;

    sublist->FreeMask &= ~(U64(1)<<slidx);
    almtx_unlock(&device->EffectLock);

    return effect;
}

static void FreeEffect(ALCdevice *device, ALeffect *effect)
{
    ALuint id = effect->id - 1;
    ALsizei lidx = id >> 6;
    ALsizei slidx = id & 0x3f;

    memset(effect, 0, sizeof(*effect));

    VECTOR_ELEM(device->EffectList, lidx).FreeMask |= U64(1) << slidx;
}

void ReleaseALEffects(ALCdevice *device)
{
    EffectSubList *sublist = VECTOR_BEGIN(device->EffectList);
    EffectSubList *subend = VECTOR_END(device->EffectList);
    size_t leftover = 0;
    for(;sublist != subend;++sublist)
    {
        ALuint64 usemask = ~sublist->FreeMask;
        while(usemask)
        {
            ALsizei idx = CTZ64(usemask);
            ALeffect *effect = sublist->Effects + idx;

            memset(effect, 0, sizeof(*effect));
            ++leftover;

            usemask &= ~(U64(1) << idx);
        }
        sublist->FreeMask = ~usemask;
    }
    if(leftover > 0)
        WARN("(%p) Deleted "SZFMT" Effect%s\n", device, leftover, (leftover==1)?"":"s");
}


static void InitEffectParams(ALeffect *effect, ALenum type)
{
    switch(type)
    {
    case AL_EFFECT_EAXREVERB:
        effect->Props.Reverb.Density   = AL_EAXREVERB_DEFAULT_DENSITY;
        effect->Props.Reverb.Diffusion = AL_EAXREVERB_DEFAULT_DIFFUSION;
        effect->Props.Reverb.Gain   = AL_EAXREVERB_DEFAULT_GAIN;
        effect->Props.Reverb.GainHF = AL_EAXREVERB_DEFAULT_GAINHF;
        effect->Props.Reverb.GainLF = AL_EAXREVERB_DEFAULT_GAINLF;
        effect->Props.Reverb.DecayTime    = AL_EAXREVERB_DEFAULT_DECAY_TIME;
        effect->Props.Reverb.DecayHFRatio = AL_EAXREVERB_DEFAULT_DECAY_HFRATIO;
        effect->Props.Reverb.DecayLFRatio = AL_EAXREVERB_DEFAULT_DECAY_LFRATIO;
        effect->Props.Reverb.ReflectionsGain   = AL_EAXREVERB_DEFAULT_REFLECTIONS_GAIN;
        effect->Props.Reverb.ReflectionsDelay  = AL_EAXREVERB_DEFAULT_REFLECTIONS_DELAY;
        effect->Props.Reverb.ReflectionsPan[0] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Props.Reverb.ReflectionsPan[1] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Props.Reverb.ReflectionsPan[2] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Props.Reverb.LateReverbGain   = AL_EAXREVERB_DEFAULT_LATE_REVERB_GAIN;
        effect->Props.Reverb.LateReverbDelay  = AL_EAXREVERB_DEFAULT_LATE_REVERB_DELAY;
        effect->Props.Reverb.LateReverbPan[0] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Props.Reverb.LateReverbPan[1] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Props.Reverb.LateReverbPan[2] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Props.Reverb.EchoTime  = AL_EAXREVERB_DEFAULT_ECHO_TIME;
        effect->Props.Reverb.EchoDepth = AL_EAXREVERB_DEFAULT_ECHO_DEPTH;
        effect->Props.Reverb.ModulationTime  = AL_EAXREVERB_DEFAULT_MODULATION_TIME;
        effect->Props.Reverb.ModulationDepth = AL_EAXREVERB_DEFAULT_MODULATION_DEPTH;
        effect->Props.Reverb.AirAbsorptionGainHF = AL_EAXREVERB_DEFAULT_AIR_ABSORPTION_GAINHF;
        effect->Props.Reverb.HFReference = AL_EAXREVERB_DEFAULT_HFREFERENCE;
        effect->Props.Reverb.LFReference = AL_EAXREVERB_DEFAULT_LFREFERENCE;
        effect->Props.Reverb.RoomRolloffFactor = AL_EAXREVERB_DEFAULT_ROOM_ROLLOFF_FACTOR;
        effect->Props.Reverb.DecayHFLimit = AL_EAXREVERB_DEFAULT_DECAY_HFLIMIT;
        effect->vtab = &ALeaxreverb_vtable;
        break;
    case AL_EFFECT_REVERB:
        effect->Props.Reverb.Density   = AL_REVERB_DEFAULT_DENSITY;
        effect->Props.Reverb.Diffusion = AL_REVERB_DEFAULT_DIFFUSION;
        effect->Props.Reverb.Gain   = AL_REVERB_DEFAULT_GAIN;
        effect->Props.Reverb.GainHF = AL_REVERB_DEFAULT_GAINHF;
        effect->Props.Reverb.GainLF = 1.0f;
        effect->Props.Reverb.DecayTime    = AL_REVERB_DEFAULT_DECAY_TIME;
        effect->Props.Reverb.DecayHFRatio = AL_REVERB_DEFAULT_DECAY_HFRATIO;
        effect->Props.Reverb.DecayLFRatio = 1.0f;
        effect->Props.Reverb.ReflectionsGain   = AL_REVERB_DEFAULT_REFLECTIONS_GAIN;
        effect->Props.Reverb.ReflectionsDelay  = AL_REVERB_DEFAULT_REFLECTIONS_DELAY;
        effect->Props.Reverb.ReflectionsPan[0] = 0.0f;
        effect->Props.Reverb.ReflectionsPan[1] = 0.0f;
        effect->Props.Reverb.ReflectionsPan[2] = 0.0f;
        effect->Props.Reverb.LateReverbGain   = AL_REVERB_DEFAULT_LATE_REVERB_GAIN;
        effect->Props.Reverb.LateReverbDelay  = AL_REVERB_DEFAULT_LATE_REVERB_DELAY;
        effect->Props.Reverb.LateReverbPan[0] = 0.0f;
        effect->Props.Reverb.LateReverbPan[1] = 0.0f;
        effect->Props.Reverb.LateReverbPan[2] = 0.0f;
        effect->Props.Reverb.EchoTime  = 0.25f;
        effect->Props.Reverb.EchoDepth = 0.0f;
        effect->Props.Reverb.ModulationTime  = 0.25f;
        effect->Props.Reverb.ModulationDepth = 0.0f;
        effect->Props.Reverb.AirAbsorptionGainHF = AL_REVERB_DEFAULT_AIR_ABSORPTION_GAINHF;
        effect->Props.Reverb.HFReference = 5000.0f;
        effect->Props.Reverb.LFReference = 250.0f;
        effect->Props.Reverb.RoomRolloffFactor = AL_REVERB_DEFAULT_ROOM_ROLLOFF_FACTOR;
        effect->Props.Reverb.DecayHFLimit = AL_REVERB_DEFAULT_DECAY_HFLIMIT;
        effect->vtab = &ALreverb_vtable;
        break;
    case AL_EFFECT_CHORUS:
        effect->Props.Chorus.Waveform = AL_CHORUS_DEFAULT_WAVEFORM;
        effect->Props.Chorus.Phase = AL_CHORUS_DEFAULT_PHASE;
        effect->Props.Chorus.Rate = AL_CHORUS_DEFAULT_RATE;
        effect->Props.Chorus.Depth = AL_CHORUS_DEFAULT_DEPTH;
        effect->Props.Chorus.Feedback = AL_CHORUS_DEFAULT_FEEDBACK;
        effect->Props.Chorus.Delay = AL_CHORUS_DEFAULT_DELAY;
        effect->vtab = &ALchorus_vtable;
        break;
    case AL_EFFECT_COMPRESSOR:
        effect->Props.Compressor.OnOff = AL_COMPRESSOR_DEFAULT_ONOFF;
        effect->vtab = &ALcompressor_vtable;
        break;
    case AL_EFFECT_DISTORTION:
        effect->Props.Distortion.Edge = AL_DISTORTION_DEFAULT_EDGE;
        effect->Props.Distortion.Gain = AL_DISTORTION_DEFAULT_GAIN;
        effect->Props.Distortion.LowpassCutoff = AL_DISTORTION_DEFAULT_LOWPASS_CUTOFF;
        effect->Props.Distortion.EQCenter = AL_DISTORTION_DEFAULT_EQCENTER;
        effect->Props.Distortion.EQBandwidth = AL_DISTORTION_DEFAULT_EQBANDWIDTH;
        effect->vtab = &ALdistortion_vtable;
        break;
    case AL_EFFECT_ECHO:
        effect->Props.Echo.Delay    = AL_ECHO_DEFAULT_DELAY;
        effect->Props.Echo.LRDelay  = AL_ECHO_DEFAULT_LRDELAY;
        effect->Props.Echo.Damping  = AL_ECHO_DEFAULT_DAMPING;
        effect->Props.Echo.Feedback = AL_ECHO_DEFAULT_FEEDBACK;
        effect->Props.Echo.Spread   = AL_ECHO_DEFAULT_SPREAD;
        effect->vtab = &ALecho_vtable;
        break;
    case AL_EFFECT_EQUALIZER:
        effect->Props.Equalizer.LowCutoff = AL_EQUALIZER_DEFAULT_LOW_CUTOFF;
        effect->Props.Equalizer.LowGain = AL_EQUALIZER_DEFAULT_LOW_GAIN;
        effect->Props.Equalizer.Mid1Center = AL_EQUALIZER_DEFAULT_MID1_CENTER;
        effect->Props.Equalizer.Mid1Gain = AL_EQUALIZER_DEFAULT_MID1_GAIN;
        effect->Props.Equalizer.Mid1Width = AL_EQUALIZER_DEFAULT_MID1_WIDTH;
        effect->Props.Equalizer.Mid2Center = AL_EQUALIZER_DEFAULT_MID2_CENTER;
        effect->Props.Equalizer.Mid2Gain = AL_EQUALIZER_DEFAULT_MID2_GAIN;
        effect->Props.Equalizer.Mid2Width = AL_EQUALIZER_DEFAULT_MID2_WIDTH;
        effect->Props.Equalizer.HighCutoff = AL_EQUALIZER_DEFAULT_HIGH_CUTOFF;
        effect->Props.Equalizer.HighGain = AL_EQUALIZER_DEFAULT_HIGH_GAIN;
        effect->vtab = &ALequalizer_vtable;
        break;
    case AL_EFFECT_FLANGER:
        effect->Props.Chorus.Waveform = AL_FLANGER_DEFAULT_WAVEFORM;
        effect->Props.Chorus.Phase = AL_FLANGER_DEFAULT_PHASE;
        effect->Props.Chorus.Rate = AL_FLANGER_DEFAULT_RATE;
        effect->Props.Chorus.Depth = AL_FLANGER_DEFAULT_DEPTH;
        effect->Props.Chorus.Feedback = AL_FLANGER_DEFAULT_FEEDBACK;
        effect->Props.Chorus.Delay = AL_FLANGER_DEFAULT_DELAY;
        effect->vtab = &ALflanger_vtable;
        break;
    case AL_EFFECT_RING_MODULATOR:
        effect->Props.Modulator.Frequency      = AL_RING_MODULATOR_DEFAULT_FREQUENCY;
        effect->Props.Modulator.HighPassCutoff = AL_RING_MODULATOR_DEFAULT_HIGHPASS_CUTOFF;
        effect->Props.Modulator.Waveform       = AL_RING_MODULATOR_DEFAULT_WAVEFORM;
        effect->vtab = &ALmodulator_vtable;
        break;
    case AL_EFFECT_PITCH_SHIFTER:
        effect->Props.Pshifter.CoarseTune      = AL_PITCH_SHIFTER_DEFAULT_COARSE_TUNE;
        effect->Props.Pshifter.FineTune        = AL_PITCH_SHIFTER_DEFAULT_FINE_TUNE;
        effect->vtab = &ALpshifter_vtable;
        break;
    case AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT:
    case AL_EFFECT_DEDICATED_DIALOGUE:
        effect->Props.Dedicated.Gain = 1.0f;
        effect->vtab = &ALdedicated_vtable;
        break;
    default:
        effect->vtab = &ALnull_vtable;
        break;
    }
    effect->type = type;
}


#include "AL/efx-presets.h"

#define DECL(x) { #x, EFX_REVERB_PRESET_##x }
static const struct {
    const char name[32];
    EFXEAXREVERBPROPERTIES props;
} reverblist[] = {
    DECL(GENERIC),
    DECL(PADDEDCELL),
    DECL(ROOM),
    DECL(BATHROOM),
    DECL(LIVINGROOM),
    DECL(STONEROOM),
    DECL(AUDITORIUM),
    DECL(CONCERTHALL),
    DECL(CAVE),
    DECL(ARENA),
    DECL(HANGAR),
    DECL(CARPETEDHALLWAY),
    DECL(HALLWAY),
    DECL(STONECORRIDOR),
    DECL(ALLEY),
    DECL(FOREST),
    DECL(CITY),
    DECL(MOUNTAINS),
    DECL(QUARRY),
    DECL(PLAIN),
    DECL(PARKINGLOT),
    DECL(SEWERPIPE),
    DECL(UNDERWATER),
    DECL(DRUGGED),
    DECL(DIZZY),
    DECL(PSYCHOTIC),

    DECL(CASTLE_SMALLROOM),
    DECL(CASTLE_SHORTPASSAGE),
    DECL(CASTLE_MEDIUMROOM),
    DECL(CASTLE_LARGEROOM),
    DECL(CASTLE_LONGPASSAGE),
    DECL(CASTLE_HALL),
    DECL(CASTLE_CUPBOARD),
    DECL(CASTLE_COURTYARD),
    DECL(CASTLE_ALCOVE),

    DECL(FACTORY_SMALLROOM),
    DECL(FACTORY_SHORTPASSAGE),
    DECL(FACTORY_MEDIUMROOM),
    DECL(FACTORY_LARGEROOM),
    DECL(FACTORY_LONGPASSAGE),
    DECL(FACTORY_HALL),
    DECL(FACTORY_CUPBOARD),
    DECL(FACTORY_COURTYARD),
    DECL(FACTORY_ALCOVE),

    DECL(ICEPALACE_SMALLROOM),
    DECL(ICEPALACE_SHORTPASSAGE),
    DECL(ICEPALACE_MEDIUMROOM),
    DECL(ICEPALACE_LARGEROOM),
    DECL(ICEPALACE_LONGPASSAGE),
    DECL(ICEPALACE_HALL),
    DECL(ICEPALACE_CUPBOARD),
    DECL(ICEPALACE_COURTYARD),
    DECL(ICEPALACE_ALCOVE),

    DECL(SPACESTATION_SMALLROOM),
    DECL(SPACESTATION_SHORTPASSAGE),
    DECL(SPACESTATION_MEDIUMROOM),
    DECL(SPACESTATION_LARGEROOM),
    DECL(SPACESTATION_LONGPASSAGE),
    DECL(SPACESTATION_HALL),
    DECL(SPACESTATION_CUPBOARD),
    DECL(SPACESTATION_ALCOVE),

    DECL(WOODEN_SMALLROOM),
    DECL(WOODEN_SHORTPASSAGE),
    DECL(WOODEN_MEDIUMROOM),
    DECL(WOODEN_LARGEROOM),
    DECL(WOODEN_LONGPASSAGE),
    DECL(WOODEN_HALL),
    DECL(WOODEN_CUPBOARD),
    DECL(WOODEN_COURTYARD),
    DECL(WOODEN_ALCOVE),

    DECL(SPORT_EMPTYSTADIUM),
    DECL(SPORT_SQUASHCOURT),
    DECL(SPORT_SMALLSWIMMINGPOOL),
    DECL(SPORT_LARGESWIMMINGPOOL),
    DECL(SPORT_GYMNASIUM),
    DECL(SPORT_FULLSTADIUM),
    DECL(SPORT_STADIUMTANNOY),

    DECL(PREFAB_WORKSHOP),
    DECL(PREFAB_SCHOOLROOM),
    DECL(PREFAB_PRACTISEROOM),
    DECL(PREFAB_OUTHOUSE),
    DECL(PREFAB_CARAVAN),

    DECL(DOME_TOMB),
    DECL(PIPE_SMALL),
    DECL(DOME_SAINTPAULS),
    DECL(PIPE_LONGTHIN),
    DECL(PIPE_LARGE),
    DECL(PIPE_RESONANT),

    DECL(OUTDOORS_BACKYARD),
    DECL(OUTDOORS_ROLLINGPLAINS),
    DECL(OUTDOORS_DEEPCANYON),
    DECL(OUTDOORS_CREEK),
    DECL(OUTDOORS_VALLEY),

    DECL(MOOD_HEAVEN),
    DECL(MOOD_HELL),
    DECL(MOOD_MEMORY),

    DECL(DRIVING_COMMENTATOR),
    DECL(DRIVING_PITGARAGE),
    DECL(DRIVING_INCAR_RACER),
    DECL(DRIVING_INCAR_SPORTS),
    DECL(DRIVING_INCAR_LUXURY),
    DECL(DRIVING_FULLGRANDSTAND),
    DECL(DRIVING_EMPTYGRANDSTAND),
    DECL(DRIVING_TUNNEL),

    DECL(CITY_STREETS),
    DECL(CITY_SUBWAY),
    DECL(CITY_MUSEUM),
    DECL(CITY_LIBRARY),
    DECL(CITY_UNDERPASS),
    DECL(CITY_ABANDONED),

    DECL(DUSTYROOM),
    DECL(CHAPEL),
    DECL(SMALLWATERROOM),
};
#undef DECL

void LoadReverbPreset(const char *name, ALeffect *effect)
{
    size_t i;

    if(strcasecmp(name, "NONE") == 0)
    {
        InitEffectParams(effect, AL_EFFECT_NULL);
        TRACE("Loading reverb '%s'\n", "NONE");
        return;
    }

    if(!DisabledEffects[EAXREVERB_EFFECT])
        InitEffectParams(effect, AL_EFFECT_EAXREVERB);
    else if(!DisabledEffects[REVERB_EFFECT])
        InitEffectParams(effect, AL_EFFECT_REVERB);
    else
        InitEffectParams(effect, AL_EFFECT_NULL);
    for(i = 0;i < COUNTOF(reverblist);i++)
    {
        const EFXEAXREVERBPROPERTIES *props;

        if(strcasecmp(name, reverblist[i].name) != 0)
            continue;

        TRACE("Loading reverb '%s'\n", reverblist[i].name);
        props = &reverblist[i].props;
        effect->Props.Reverb.Density   = props->flDensity;
        effect->Props.Reverb.Diffusion = props->flDiffusion;
        effect->Props.Reverb.Gain   = props->flGain;
        effect->Props.Reverb.GainHF = props->flGainHF;
        effect->Props.Reverb.GainLF = props->flGainLF;
        effect->Props.Reverb.DecayTime    = props->flDecayTime;
        effect->Props.Reverb.DecayHFRatio = props->flDecayHFRatio;
        effect->Props.Reverb.DecayLFRatio = props->flDecayLFRatio;
        effect->Props.Reverb.ReflectionsGain   = props->flReflectionsGain;
        effect->Props.Reverb.ReflectionsDelay  = props->flReflectionsDelay;
        effect->Props.Reverb.ReflectionsPan[0] = props->flReflectionsPan[0];
        effect->Props.Reverb.ReflectionsPan[1] = props->flReflectionsPan[1];
        effect->Props.Reverb.ReflectionsPan[2] = props->flReflectionsPan[2];
        effect->Props.Reverb.LateReverbGain   = props->flLateReverbGain;
        effect->Props.Reverb.LateReverbDelay  = props->flLateReverbDelay;
        effect->Props.Reverb.LateReverbPan[0] = props->flLateReverbPan[0];
        effect->Props.Reverb.LateReverbPan[1] = props->flLateReverbPan[1];
        effect->Props.Reverb.LateReverbPan[2] = props->flLateReverbPan[2];
        effect->Props.Reverb.EchoTime  = props->flEchoTime;
        effect->Props.Reverb.EchoDepth = props->flEchoDepth;
        effect->Props.Reverb.ModulationTime  = props->flModulationTime;
        effect->Props.Reverb.ModulationDepth = props->flModulationDepth;
        effect->Props.Reverb.AirAbsorptionGainHF = props->flAirAbsorptionGainHF;
        effect->Props.Reverb.HFReference = props->flHFReference;
        effect->Props.Reverb.LFReference = props->flLFReference;
        effect->Props.Reverb.RoomRolloffFactor = props->flRoomRolloffFactor;
        effect->Props.Reverb.DecayHFLimit = props->iDecayHFLimit;
        return;
    }

    WARN("Reverb preset '%s' not found\n", name);
}
