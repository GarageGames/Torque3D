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

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alAuxEffectSlot.h"
#include "alThunk.h"
#include "alError.h"
#include "alListener.h"
#include "alSource.h"

#include "almalloc.h"


extern inline void LockEffectSlotsRead(ALCcontext *context);
extern inline void UnlockEffectSlotsRead(ALCcontext *context);
extern inline void LockEffectSlotsWrite(ALCcontext *context);
extern inline void UnlockEffectSlotsWrite(ALCcontext *context);
extern inline struct ALeffectslot *LookupEffectSlot(ALCcontext *context, ALuint id);
extern inline struct ALeffectslot *RemoveEffectSlot(ALCcontext *context, ALuint id);

static void RemoveEffectSlotList(ALCcontext *Context, ALeffectslot *slot);

static UIntMap EffectStateFactoryMap;
static inline ALeffectStateFactory *getFactoryByType(ALenum type)
{
    ALeffectStateFactory* (*getFactory)(void) = LookupUIntMapKey(&EffectStateFactoryMap, type);
    if(getFactory != NULL)
        return getFactory();
    return NULL;
}

static void ALeffectState_IncRef(ALeffectState *state);
static void ALeffectState_DecRef(ALeffectState *state);

#define DO_UPDATEPROPS() do {                                                 \
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))          \
        UpdateEffectSlotProps(slot);                                          \
    else                                                                      \
        slot->NeedsUpdate = AL_TRUE;                                          \
} while(0)


AL_API ALvoid AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots)
{
    ALCcontext *context;
    ALeffectslot *first, *last;
    ALsizei cur;
    ALenum err;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    first = last = NULL;
    for(cur = 0;cur < n;cur++)
    {
        ALeffectslot *slot = al_calloc(16, sizeof(ALeffectslot));
        err = AL_OUT_OF_MEMORY;
        if(!slot || (err=InitEffectSlot(slot)) != AL_NO_ERROR)
        {
            al_free(slot);
            alDeleteAuxiliaryEffectSlots(cur, effectslots);
            SET_ERROR_AND_GOTO(context, err, done);
        }

        err = NewThunkEntry(&slot->id);
        if(err == AL_NO_ERROR)
            err = InsertUIntMapEntry(&context->EffectSlotMap, slot->id, slot);
        if(err != AL_NO_ERROR)
        {
            FreeThunkEntry(slot->id);
            ALeffectState_DecRef(slot->Effect.State);
            if(slot->Params.EffectState)
                ALeffectState_DecRef(slot->Params.EffectState);
            al_free(slot);

            alDeleteAuxiliaryEffectSlots(cur, effectslots);
            SET_ERROR_AND_GOTO(context, err, done);
        }

        aluInitEffectPanning(slot);

        if(!first) first = slot;
        if(last) ATOMIC_STORE(&last->next, slot, almemory_order_relaxed);
        last = slot;

        effectslots[cur] = slot->id;
    }
    if(last != NULL)
    {
        ALeffectslot *root = ATOMIC_LOAD(&context->ActiveAuxSlotList);
        do {
            ATOMIC_STORE(&last->next, root, almemory_order_relaxed);
        } while(!ATOMIC_COMPARE_EXCHANGE_WEAK(ALeffectslot*, &context->ActiveAuxSlotList,
                                              &root, first));
    }

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, const ALuint *effectslots)
{
    ALCcontext *context;
    ALeffectslot *slot;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsWrite(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(i = 0;i < n;i++)
    {
        if((slot=LookupEffectSlot(context, effectslots[i])) == NULL)
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
        if(ReadRef(&slot->ref) != 0)
            SET_ERROR_AND_GOTO(context, AL_INVALID_OPERATION, done);
    }

    // All effectslots are valid
    for(i = 0;i < n;i++)
    {
        if((slot=RemoveEffectSlot(context, effectslots[i])) == NULL)
            continue;
        FreeThunkEntry(slot->id);

        RemoveEffectSlotList(context, slot);
        DeinitEffectSlot(slot);

        memset(slot, 0, sizeof(*slot));
        al_free(slot);
    }

done:
    UnlockEffectSlotsWrite(context);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint effectslot)
{
    ALCcontext *context;
    ALboolean  ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    LockEffectSlotsRead(context);
    ret = (LookupEffectSlot(context, effectslot) ? AL_TRUE : AL_FALSE);
    UnlockEffectSlotsRead(context);

    ALCcontext_DecRef(context);

    return ret;
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint value)
{
    ALCdevice *device;
    ALCcontext *context;
    ALeffectslot *slot;
    ALeffect *effect = NULL;
    ALenum err;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    LockEffectSlotsRead(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    case AL_EFFECTSLOT_EFFECT:
        device = context->Device;

        LockEffectsRead(device);
        effect = (value ? LookupEffect(device, value) : NULL);
        if(!(value == 0 || effect != NULL))
        {
            UnlockEffectsRead(device);
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        }
        err = InitializeEffect(device, slot, effect);
        UnlockEffectsRead(device);

        if(err != AL_NO_ERROR)
            SET_ERROR_AND_GOTO(context, err, done);
        break;

    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        if(!(value == AL_TRUE || value == AL_FALSE))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        slot->AuxSendAuto = value;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    DO_UPDATEPROPS();

done:
    UnlockEffectSlotsRead(context);
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, const ALint *values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_EFFECTSLOT_EFFECT:
    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        alAuxiliaryEffectSloti(effectslot, param, values[0]);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    LockEffectSlotsRead(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        if(!(value >= 0.0f && value <= 1.0f))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        slot->Gain = value;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    DO_UPDATEPROPS();

done:
    UnlockEffectSlotsRead(context);
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, const ALfloat *values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        alAuxiliaryEffectSlotf(effectslot, param, values[0]);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        *value = slot->AuxSendAuto;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_EFFECTSLOT_EFFECT:
    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        alGetAuxiliaryEffectSloti(effectslot, param, values);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        *value = slot->Gain;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        alGetAuxiliaryEffectSlotf(effectslot, param, values);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotsRead(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockEffectSlotsRead(context);
    ALCcontext_DecRef(context);
}


static void RemoveEffectSlotList(ALCcontext *context, ALeffectslot *slot)
{
    ALCdevice *device = context->Device;
    ALeffectslot *root, *next;

    root = slot;
    next = ATOMIC_LOAD(&slot->next);
    if(!ATOMIC_COMPARE_EXCHANGE_STRONG(ALeffectslot*, &context->ActiveAuxSlotList, &root, next))
    {
        ALeffectslot *cur;
        do {
            cur = root;
            root = slot;
        } while(!ATOMIC_COMPARE_EXCHANGE_STRONG(ALeffectslot*, &cur->next, &root, next));
    }
    /* Wait for any mix that may be using these effect slots to finish. */
    while((ReadRef(&device->MixCount)&1) != 0)
        althrd_yield();
}


void InitEffectFactoryMap(void)
{
    InitUIntMap(&EffectStateFactoryMap, ~0);

    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_NULL, ALnullStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_EAXREVERB, ALreverbStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_REVERB, ALreverbStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_CHORUS, ALchorusStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_COMPRESSOR, ALcompressorStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_DISTORTION, ALdistortionStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_ECHO, ALechoStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_EQUALIZER, ALequalizerStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_FLANGER, ALflangerStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_RING_MODULATOR, ALmodulatorStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_DEDICATED_DIALOGUE, ALdedicatedStateFactory_getFactory);
    InsertUIntMapEntry(&EffectStateFactoryMap, AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT, ALdedicatedStateFactory_getFactory);
}

void DeinitEffectFactoryMap(void)
{
    ResetUIntMap(&EffectStateFactoryMap);
}


ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect)
{
    ALenum newtype = (effect ? effect->type : AL_EFFECT_NULL);
    struct ALeffectslotProps *props;
    ALeffectState *State;

    if(newtype != EffectSlot->Effect.Type)
    {
        ALeffectStateFactory *factory;
        FPUCtl oldMode;

        factory = getFactoryByType(newtype);
        if(!factory)
        {
            ERR("Failed to find factory for effect type 0x%04x\n", newtype);
            return AL_INVALID_ENUM;
        }
        State = V0(factory,create)();
        if(!State) return AL_OUT_OF_MEMORY;

        SetMixerFPUMode(&oldMode);
        almtx_lock(&Device->BackendLock);
        State->OutBuffer = Device->Dry.Buffer;
        State->OutChannels = Device->Dry.NumChannels;
        if(V(State,deviceUpdate)(Device) == AL_FALSE)
        {
            almtx_unlock(&Device->BackendLock);
            RestoreFPUMode(&oldMode);
            ALeffectState_DecRef(State);
            return AL_OUT_OF_MEMORY;
        }
        almtx_unlock(&Device->BackendLock);
        RestoreFPUMode(&oldMode);

        if(!effect)
        {
            EffectSlot->Effect.Type = AL_EFFECT_NULL;
            memset(&EffectSlot->Effect.Props, 0, sizeof(EffectSlot->Effect.Props));
        }
        else
        {
            EffectSlot->Effect.Type = effect->type;
            EffectSlot->Effect.Props = effect->Props;
        }

        ALeffectState_DecRef(EffectSlot->Effect.State);
        EffectSlot->Effect.State = State;
    }
    else if(effect)
        EffectSlot->Effect.Props = effect->Props;

    /* Remove state references from old effect slot property updates. */
    props = ATOMIC_LOAD(&EffectSlot->FreeList);
    while(props)
    {
        State = ATOMIC_EXCHANGE(ALeffectState*, &props->State, NULL, almemory_order_relaxed);
        if(State) ALeffectState_DecRef(State);
        props = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
    }

    return AL_NO_ERROR;
}


static void ALeffectState_IncRef(ALeffectState *state)
{
    uint ref;
    ref = IncrementRef(&state->Ref);
    TRACEREF("%p increasing refcount to %u\n", state, ref);
}

static void ALeffectState_DecRef(ALeffectState *state)
{
    uint ref;
    ref = DecrementRef(&state->Ref);
    TRACEREF("%p decreasing refcount to %u\n", state, ref);
    if(ref == 0) DELETE_OBJ(state);
}


void ALeffectState_Construct(ALeffectState *state)
{
    InitRef(&state->Ref, 1);

    state->OutBuffer = NULL;
    state->OutChannels = 0;
}

void ALeffectState_Destruct(ALeffectState *UNUSED(state))
{
}


ALenum InitEffectSlot(ALeffectslot *slot)
{
    ALeffectStateFactory *factory;

    slot->Effect.Type = AL_EFFECT_NULL;

    factory = getFactoryByType(AL_EFFECT_NULL);
    if(!(slot->Effect.State=V0(factory,create)()))
        return AL_OUT_OF_MEMORY;

    slot->NeedsUpdate = AL_FALSE;
    slot->Gain = 1.0;
    slot->AuxSendAuto = AL_TRUE;
    InitRef(&slot->ref, 0);

    ATOMIC_INIT(&slot->Update, NULL);
    ATOMIC_INIT(&slot->FreeList, NULL);

    slot->Params.Gain = 1.0f;
    slot->Params.AuxSendAuto = AL_TRUE;
    ALeffectState_IncRef(slot->Effect.State);
    slot->Params.EffectState = slot->Effect.State;
    slot->Params.RoomRolloff = 0.0f;
    slot->Params.DecayTime = 0.0f;
    slot->Params.AirAbsorptionGainHF = 1.0f;

    ATOMIC_INIT(&slot->next, NULL);

    return AL_NO_ERROR;
}

void DeinitEffectSlot(ALeffectslot *slot)
{
    struct ALeffectslotProps *props;
    ALeffectState *state;
    size_t count = 0;

    props = ATOMIC_LOAD(&slot->Update);
    if(props)
    {
        state = ATOMIC_LOAD(&props->State, almemory_order_relaxed);
        if(state) ALeffectState_DecRef(state);
        TRACE("Freed unapplied AuxiliaryEffectSlot update %p\n", props);
        al_free(props);
    }
    props = ATOMIC_LOAD(&slot->FreeList, almemory_order_relaxed);
    while(props)
    {
        struct ALeffectslotProps *next;
        state = ATOMIC_LOAD(&props->State, almemory_order_relaxed);
        next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        if(state) ALeffectState_DecRef(state);
        al_free(props);
        props = next;
        ++count;
    }
    TRACE("Freed "SZFMT" AuxiliaryEffectSlot property object%s\n", count, (count==1)?"":"s");

    ALeffectState_DecRef(slot->Effect.State);
    if(slot->Params.EffectState)
        ALeffectState_DecRef(slot->Params.EffectState);
}

void UpdateEffectSlotProps(ALeffectslot *slot)
{
    struct ALeffectslotProps *props;
    ALeffectState *oldstate;

    /* Get an unused property container, or allocate a new one as needed. */
    props = ATOMIC_LOAD(&slot->FreeList, almemory_order_relaxed);
    if(!props)
        props = al_calloc(16, sizeof(*props));
    else
    {
        struct ALeffectslotProps *next;
        do {
            next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALeffectslotProps*,
                &slot->FreeList, &props, next, almemory_order_seq_cst,
                almemory_order_consume) == 0);
    }

    /* Copy in current property values. */
    ATOMIC_STORE(&props->Gain, slot->Gain, almemory_order_relaxed);
    ATOMIC_STORE(&props->AuxSendAuto, slot->AuxSendAuto, almemory_order_relaxed);

    ATOMIC_STORE(&props->Type, slot->Effect.Type, almemory_order_relaxed);
    props->Props = slot->Effect.Props;
    /* Swap out any stale effect state object there may be in the container, to
     * delete it.
     */
    ALeffectState_IncRef(slot->Effect.State);
    oldstate = ATOMIC_EXCHANGE(ALeffectState*, &props->State, slot->Effect.State,
                               almemory_order_relaxed);

    /* Set the new container for updating internal parameters. */
    props = ATOMIC_EXCHANGE(struct ALeffectslotProps*, &slot->Update, props,
                            almemory_order_acq_rel);
    if(props)
    {
        /* If there was an unused update container, put it back in the
         * freelist.
         */
        struct ALeffectslotProps *first = ATOMIC_LOAD(&slot->FreeList);
        do {
            ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALeffectslotProps*,
                &slot->FreeList, &first, props) == 0);
    }

    if(oldstate)
        ALeffectState_DecRef(oldstate);
}

void UpdateAllEffectSlotProps(ALCcontext *context)
{
    ALeffectslot *slot;

    LockEffectSlotsRead(context);
    slot = ATOMIC_LOAD(&context->ActiveAuxSlotList);
    while(slot)
    {
        if(slot->NeedsUpdate)
            UpdateEffectSlotProps(slot);
        slot->NeedsUpdate = AL_FALSE;
        slot = ATOMIC_LOAD(&slot->next, almemory_order_relaxed);
    }
    UnlockEffectSlotsRead(context);
}

ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context)
{
    ALsizei pos;
    for(pos = 0;pos < Context->EffectSlotMap.size;pos++)
    {
        ALeffectslot *temp = Context->EffectSlotMap.values[pos];
        Context->EffectSlotMap.values[pos] = NULL;

        DeinitEffectSlot(temp);

        FreeThunkEntry(temp->id);
        memset(temp, 0, sizeof(ALeffectslot));
        al_free(temp);
    }
}
