#ifndef _AL_EFFECT_H_
#define _AL_EFFECT_H_

#include "alMain.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALeffect;

enum {
    EAXREVERB = 0,
    REVERB,
    CHORUS,
    COMPRESSOR,
    DISTORTION,
    ECHO,
    EQUALIZER,
    FLANGER,
    MODULATOR,
    DEDICATED,

    MAX_EFFECTS
};
extern ALboolean DisabledEffects[MAX_EFFECTS];

extern ALfloat ReverbBoost;
extern ALboolean EmulateEAXReverb;

struct ALeffectVtable {
    void (*const setParami)(struct ALeffect *effect, ALCcontext *context, ALenum param, ALint val);
    void (*const setParamiv)(struct ALeffect *effect, ALCcontext *context, ALenum param, const ALint *vals);
    void (*const setParamf)(struct ALeffect *effect, ALCcontext *context, ALenum param, ALfloat val);
    void (*const setParamfv)(struct ALeffect *effect, ALCcontext *context, ALenum param, const ALfloat *vals);

    void (*const getParami)(const struct ALeffect *effect, ALCcontext *context, ALenum param, ALint *val);
    void (*const getParamiv)(const struct ALeffect *effect, ALCcontext *context, ALenum param, ALint *vals);
    void (*const getParamf)(const struct ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *val);
    void (*const getParamfv)(const struct ALeffect *effect, ALCcontext *context, ALenum param, ALfloat *vals);
};

#define DEFINE_ALEFFECT_VTABLE(T)           \
const struct ALeffectVtable T##_vtable = {  \
    T##_setParami, T##_setParamiv,          \
    T##_setParamf, T##_setParamfv,          \
    T##_getParami, T##_getParamiv,          \
    T##_getParamf, T##_getParamfv,          \
}

extern const struct ALeffectVtable ALeaxreverb_vtable;
extern const struct ALeffectVtable ALreverb_vtable;
extern const struct ALeffectVtable ALchorus_vtable;
extern const struct ALeffectVtable ALcompressor_vtable;
extern const struct ALeffectVtable ALdistortion_vtable;
extern const struct ALeffectVtable ALecho_vtable;
extern const struct ALeffectVtable ALequalizer_vtable;
extern const struct ALeffectVtable ALflanger_vtable;
extern const struct ALeffectVtable ALmodulator_vtable;
extern const struct ALeffectVtable ALnull_vtable;
extern const struct ALeffectVtable ALdedicated_vtable;


typedef union ALeffectProps {
    struct {
        // Shared Reverb Properties
        ALfloat Density;
        ALfloat Diffusion;
        ALfloat Gain;
        ALfloat GainHF;
        ALfloat DecayTime;
        ALfloat DecayHFRatio;
        ALfloat ReflectionsGain;
        ALfloat ReflectionsDelay;
        ALfloat LateReverbGain;
        ALfloat LateReverbDelay;
        ALfloat AirAbsorptionGainHF;
        ALfloat RoomRolloffFactor;
        ALboolean DecayHFLimit;

        // Additional EAX Reverb Properties
        ALfloat GainLF;
        ALfloat DecayLFRatio;
        ALfloat ReflectionsPan[3];
        ALfloat LateReverbPan[3];
        ALfloat EchoTime;
        ALfloat EchoDepth;
        ALfloat ModulationTime;
        ALfloat ModulationDepth;
        ALfloat HFReference;
        ALfloat LFReference;
    } Reverb;

    struct {
        ALint Waveform;
        ALint Phase;
        ALfloat Rate;
        ALfloat Depth;
        ALfloat Feedback;
        ALfloat Delay;
    } Chorus;

    struct {
        ALboolean OnOff;
    } Compressor;

    struct {
        ALfloat Edge;
        ALfloat Gain;
        ALfloat LowpassCutoff;
        ALfloat EQCenter;
        ALfloat EQBandwidth;
    } Distortion;

    struct {
        ALfloat Delay;
        ALfloat LRDelay;

        ALfloat Damping;
        ALfloat Feedback;

        ALfloat Spread;
    } Echo;

    struct {
        ALfloat LowCutoff;
        ALfloat LowGain;
        ALfloat Mid1Center;
        ALfloat Mid1Gain;
        ALfloat Mid1Width;
        ALfloat Mid2Center;
        ALfloat Mid2Gain;
        ALfloat Mid2Width;
        ALfloat HighCutoff;
        ALfloat HighGain;
    } Equalizer;

    struct {
        ALint Waveform;
        ALint Phase;
        ALfloat Rate;
        ALfloat Depth;
        ALfloat Feedback;
        ALfloat Delay;
    } Flanger;

    struct {
        ALfloat Frequency;
        ALfloat HighPassCutoff;
        ALint Waveform;
    } Modulator;

    struct {
        ALfloat Gain;
    } Dedicated;
} ALeffectProps;

typedef struct ALeffect {
    // Effect type (AL_EFFECT_NULL, ...)
    ALenum type;

    ALeffectProps Props;

    const struct ALeffectVtable *vtbl;

    /* Self ID */
    ALuint id;
} ALeffect;

inline void LockEffectsRead(ALCdevice *device)
{ LockUIntMapRead(&device->EffectMap); }
inline void UnlockEffectsRead(ALCdevice *device)
{ UnlockUIntMapRead(&device->EffectMap); }
inline void LockEffectsWrite(ALCdevice *device)
{ LockUIntMapWrite(&device->EffectMap); }
inline void UnlockEffectsWrite(ALCdevice *device)
{ UnlockUIntMapWrite(&device->EffectMap); }

inline struct ALeffect *LookupEffect(ALCdevice *device, ALuint id)
{ return (struct ALeffect*)LookupUIntMapKeyNoLock(&device->EffectMap, id); }
inline struct ALeffect *RemoveEffect(ALCdevice *device, ALuint id)
{ return (struct ALeffect*)RemoveUIntMapKeyNoLock(&device->EffectMap, id); }

inline ALboolean IsReverbEffect(ALenum type)
{ return type == AL_EFFECT_REVERB || type == AL_EFFECT_EAXREVERB; }

ALenum InitEffect(ALeffect *effect);
ALvoid ReleaseALEffects(ALCdevice *device);

ALvoid LoadReverbPreset(const char *name, ALeffect *effect);

#ifdef __cplusplus
}
#endif

#endif
