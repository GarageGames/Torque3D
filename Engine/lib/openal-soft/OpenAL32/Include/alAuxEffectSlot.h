#ifndef _AL_AUXEFFECTSLOT_H_
#define _AL_AUXEFFECTSLOT_H_

#include "alMain.h"
#include "alEffect.h"

#include "align.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALeffectStateVtable;
struct ALeffectslot;

typedef struct ALeffectState {
    RefCount Ref;
    const struct ALeffectStateVtable *vtbl;

    ALfloat (*OutBuffer)[BUFFERSIZE];
    ALuint OutChannels;
} ALeffectState;

void ALeffectState_Construct(ALeffectState *state);
void ALeffectState_Destruct(ALeffectState *state);

struct ALeffectStateVtable {
    void (*const Destruct)(ALeffectState *state);

    ALboolean (*const deviceUpdate)(ALeffectState *state, ALCdevice *device);
    void (*const update)(ALeffectState *state, const ALCdevice *device, const struct ALeffectslot *slot, const union ALeffectProps *props);
    void (*const process)(ALeffectState *state, ALuint samplesToDo, const ALfloat (*restrict samplesIn)[BUFFERSIZE], ALfloat (*restrict samplesOut)[BUFFERSIZE], ALuint numChannels);

    void (*const Delete)(void *ptr);
};

#define DEFINE_ALEFFECTSTATE_VTABLE(T)                                        \
DECLARE_THUNK(T, ALeffectState, void, Destruct)                               \
DECLARE_THUNK1(T, ALeffectState, ALboolean, deviceUpdate, ALCdevice*)         \
DECLARE_THUNK3(T, ALeffectState, void, update, const ALCdevice*, const ALeffectslot*, const ALeffectProps*) \
DECLARE_THUNK4(T, ALeffectState, void, process, ALuint, const ALfloatBUFFERSIZE*restrict, ALfloatBUFFERSIZE*restrict, ALuint) \
static void T##_ALeffectState_Delete(void *ptr)                               \
{ return T##_Delete(STATIC_UPCAST(T, ALeffectState, (ALeffectState*)ptr)); }  \
                                                                              \
static const struct ALeffectStateVtable T##_ALeffectState_vtable = {          \
    T##_ALeffectState_Destruct,                                               \
                                                                              \
    T##_ALeffectState_deviceUpdate,                                           \
    T##_ALeffectState_update,                                                 \
    T##_ALeffectState_process,                                                \
                                                                              \
    T##_ALeffectState_Delete,                                                 \
}


struct ALeffectStateFactoryVtable;

typedef struct ALeffectStateFactory {
    const struct ALeffectStateFactoryVtable *vtbl;
} ALeffectStateFactory;

struct ALeffectStateFactoryVtable {
    ALeffectState *(*const create)(ALeffectStateFactory *factory);
};

#define DEFINE_ALEFFECTSTATEFACTORY_VTABLE(T)                                 \
DECLARE_THUNK(T, ALeffectStateFactory, ALeffectState*, create)                \
                                                                              \
static const struct ALeffectStateFactoryVtable T##_ALeffectStateFactory_vtable = { \
    T##_ALeffectStateFactory_create,                                          \
}


#define MAX_EFFECT_CHANNELS (4)


struct ALeffectslotProps {
    ATOMIC(ALfloat)   Gain;
    ATOMIC(ALboolean) AuxSendAuto;

    ATOMIC(ALenum) Type;
    ALeffectProps Props;

    ATOMIC(ALeffectState*) State;

    ATOMIC(struct ALeffectslotProps*) next;
};


typedef struct ALeffectslot {
    ALboolean NeedsUpdate;

    ALfloat   Gain;
    ALboolean AuxSendAuto;

    struct {
        ALenum Type;
        ALeffectProps Props;

        ALeffectState *State;
    } Effect;

    RefCount ref;

    ATOMIC(struct ALeffectslotProps*) Update;
    ATOMIC(struct ALeffectslotProps*) FreeList;

    struct {
        ALfloat   Gain;
        ALboolean AuxSendAuto;

        ALenum EffectType;
        ALeffectState *EffectState;

        ALfloat RoomRolloff; /* Added to the source's room rolloff, not multiplied. */
        ALfloat DecayTime;
        ALfloat AirAbsorptionGainHF;
    } Params;

    /* Self ID */
    ALuint id;

    ALuint NumChannels;
    BFChannelConfig ChanMap[MAX_EFFECT_CHANNELS];
    /* Wet buffer configuration is ACN channel order with N3D scaling:
     * * Channel 0 is the unattenuated mono signal.
     * * Channel 1 is OpenAL -X
     * * Channel 2 is OpenAL Y
     * * Channel 3 is OpenAL -Z
     * Consequently, effects that only want to work with mono input can use
     * channel 0 by itself. Effects that want multichannel can process the
     * ambisonics signal and make a B-Format pan (ComputeFirstOrderGains) for
     * first-order device output (FOAOut).
     */
    alignas(16) ALfloat WetBuffer[MAX_EFFECT_CHANNELS][BUFFERSIZE];

    ATOMIC(struct ALeffectslot*) next;
} ALeffectslot;

inline void LockEffectSlotsRead(ALCcontext *context)
{ LockUIntMapRead(&context->EffectSlotMap); }
inline void UnlockEffectSlotsRead(ALCcontext *context)
{ UnlockUIntMapRead(&context->EffectSlotMap); }
inline void LockEffectSlotsWrite(ALCcontext *context)
{ LockUIntMapWrite(&context->EffectSlotMap); }
inline void UnlockEffectSlotsWrite(ALCcontext *context)
{ UnlockUIntMapWrite(&context->EffectSlotMap); }

inline struct ALeffectslot *LookupEffectSlot(ALCcontext *context, ALuint id)
{ return (struct ALeffectslot*)LookupUIntMapKeyNoLock(&context->EffectSlotMap, id); }
inline struct ALeffectslot *RemoveEffectSlot(ALCcontext *context, ALuint id)
{ return (struct ALeffectslot*)RemoveUIntMapKeyNoLock(&context->EffectSlotMap, id); }

ALenum InitEffectSlot(ALeffectslot *slot);
void DeinitEffectSlot(ALeffectslot *slot);
void UpdateEffectSlotProps(ALeffectslot *slot);
void UpdateAllEffectSlotProps(ALCcontext *context);
ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context);


ALeffectStateFactory *ALnullStateFactory_getFactory(void);
ALeffectStateFactory *ALreverbStateFactory_getFactory(void);
ALeffectStateFactory *ALchorusStateFactory_getFactory(void);
ALeffectStateFactory *ALcompressorStateFactory_getFactory(void);
ALeffectStateFactory *ALdistortionStateFactory_getFactory(void);
ALeffectStateFactory *ALechoStateFactory_getFactory(void);
ALeffectStateFactory *ALequalizerStateFactory_getFactory(void);
ALeffectStateFactory *ALflangerStateFactory_getFactory(void);
ALeffectStateFactory *ALmodulatorStateFactory_getFactory(void);

ALeffectStateFactory *ALdedicatedStateFactory_getFactory(void);


ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect);

void InitEffectFactoryMap(void);
void DeinitEffectFactoryMap(void);

#ifdef __cplusplus
}
#endif

#endif
