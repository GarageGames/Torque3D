#ifndef _AL_AUXEFFECTSLOT_H_
#define _AL_AUXEFFECTSLOT_H_

#include "alMain.h"
#include "alEffect.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ALeffectState {
    ALvoid (*Destroy)(struct ALeffectState *State);
    ALboolean (*DeviceUpdate)(struct ALeffectState *State, ALCdevice *Device);
    ALvoid (*Update)(struct ALeffectState *State, ALCdevice *Device, const struct ALeffectslot *Slot);
    ALvoid (*Process)(struct ALeffectState *State, ALuint SamplesToDo, const ALfloat *RESTRICT SamplesIn, ALfloat (*RESTRICT SamplesOut)[BUFFERSIZE]);
} ALeffectState;


typedef struct ALeffectslot
{
    ALeffect effect;

    volatile ALfloat   Gain;
    volatile ALboolean AuxSendAuto;

    volatile ALenum NeedsUpdate;
    ALeffectState *EffectState;

    ALIGN(16) ALfloat WetBuffer[1][BUFFERSIZE];

    ALfloat ClickRemoval[1];
    ALfloat PendingClicks[1];

    RefCount ref;

    /* Self ID */
    ALuint id;
} ALeffectslot;


ALenum InitEffectSlot(ALeffectslot *slot);
ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context);

ALeffectState *NoneCreate(void);
ALeffectState *ReverbCreate(void);
ALeffectState *EchoCreate(void);
ALeffectState *ModulatorCreate(void);
ALeffectState *DedicatedCreate(void);

#define ALeffectState_Destroy(a)        ((a)->Destroy((a)))
#define ALeffectState_DeviceUpdate(a,b) ((a)->DeviceUpdate((a),(b)))
#define ALeffectState_Update(a,b,c)     ((a)->Update((a),(b),(c)))
#define ALeffectState_Process(a,b,c,d)  ((a)->Process((a),(b),(c),(d)))

ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect);

#ifdef __cplusplus
}
#endif

#endif
