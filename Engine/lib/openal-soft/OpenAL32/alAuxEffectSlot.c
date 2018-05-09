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
#include "alError.h"
#include "alListener.h"
#include "alSource.h"

#include "fpu_modes.h"
#include "almalloc.h"


extern inline void LockEffectSlotList(ALCcontext *context);
extern inline void UnlockEffectSlotList(ALCcontext *context);

static void AddActiveEffectSlots(const ALuint *slotids, ALsizei count, ALCcontext *context);
static void RemoveActiveEffectSlots(const ALuint *slotids, ALsizei count, ALCcontext *context);

static const struct {
    ALenum Type;
    EffectStateFactory* (*GetFactory)(void);
} FactoryList[] = {
    { AL_EFFECT_NULL, NullStateFactory_getFactory },
    { AL_EFFECT_EAXREVERB, ReverbStateFactory_getFactory },
    { AL_EFFECT_REVERB, ReverbStateFactory_getFactory },
    { AL_EFFECT_CHORUS, ChorusStateFactory_getFactory },
    { AL_EFFECT_COMPRESSOR, CompressorStateFactory_getFactory },
    { AL_EFFECT_DISTORTION, DistortionStateFactory_getFactory },
    { AL_EFFECT_ECHO, EchoStateFactory_getFactory },
    { AL_EFFECT_EQUALIZER, EqualizerStateFactory_getFactory },
    { AL_EFFECT_FLANGER, FlangerStateFactory_getFactory },
    { AL_EFFECT_RING_MODULATOR, ModulatorStateFactory_getFactory },
    { AL_EFFECT_PITCH_SHIFTER, PshifterStateFactory_getFactory},
    { AL_EFFECT_DEDICATED_DIALOGUE, DedicatedStateFactory_getFactory },
    { AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT, DedicatedStateFactory_getFactory }
};

static inline EffectStateFactory *getFactoryByType(ALenum type)
{
    size_t i;
    for(i = 0;i < COUNTOF(FactoryList);i++)
    {
        if(FactoryList[i].Type == type)
            return FactoryList[i].GetFactory();
    }
    return NULL;
}

static void ALeffectState_IncRef(ALeffectState *state);


static inline ALeffectslot *LookupEffectSlot(ALCcontext *context, ALuint id)
{
    id--;
    if(UNLIKELY(id >= VECTOR_SIZE(context->EffectSlotList)))
        return NULL;
    return VECTOR_ELEM(context->EffectSlotList, id);
}

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


#define DO_UPDATEPROPS() do {                                                 \
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))          \
        UpdateEffectSlotProps(slot, context);                                 \
    else                                                                      \
        ATOMIC_FLAG_CLEAR(&slot->PropsClean, almemory_order_release);         \
} while(0)


AL_API ALvoid AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots)
{
    ALCdevice *device;
    ALCcontext *context;
    ALsizei cur;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        SETERR_GOTO(context, AL_INVALID_VALUE, done, "Generating %d effect slots", n);
    if(n == 0) goto done;

    LockEffectSlotList(context);
    device = context->Device;
    if(device->AuxiliaryEffectSlotMax - VECTOR_SIZE(context->EffectSlotList) < (ALuint)n)
    {
        UnlockEffectSlotList(context);
        SETERR_GOTO(context, AL_OUT_OF_MEMORY, done, "Exceeding %u auxiliary effect slot limit",
                    device->AuxiliaryEffectSlotMax);
    }
    for(cur = 0;cur < n;cur++)
    {
        ALeffectslotPtr *iter = VECTOR_BEGIN(context->EffectSlotList);
        ALeffectslotPtr *end = VECTOR_END(context->EffectSlotList);
        ALeffectslot *slot = NULL;
        ALenum err = AL_OUT_OF_MEMORY;

        for(;iter != end;iter++)
        {
            if(!*iter)
                break;
        }
        if(iter == end)
        {
            VECTOR_PUSH_BACK(context->EffectSlotList, NULL);
            iter = &VECTOR_BACK(context->EffectSlotList);
        }
        slot = al_calloc(16, sizeof(ALeffectslot));
        if(!slot || (err=InitEffectSlot(slot)) != AL_NO_ERROR)
        {
            al_free(slot);
            UnlockEffectSlotList(context);

            alDeleteAuxiliaryEffectSlots(cur, effectslots);
            SETERR_GOTO(context, err, done, "Effect slot object allocation failed");
        }
        aluInitEffectPanning(slot);

        slot->id = (iter - VECTOR_BEGIN(context->EffectSlotList)) + 1;
        *iter = slot;

        effectslots[cur] = slot->id;
    }
    AddActiveEffectSlots(effectslots, n, context);
    UnlockEffectSlotList(context);

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

    LockEffectSlotList(context);
    if(!(n >= 0))
        SETERR_GOTO(context, AL_INVALID_VALUE, done, "Deleting %d effect slots", n);
    if(n == 0) goto done;

    for(i = 0;i < n;i++)
    {
        if((slot=LookupEffectSlot(context, effectslots[i])) == NULL)
            SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u",
                        effectslots[i]);
        if(ReadRef(&slot->ref) != 0)
            SETERR_GOTO(context, AL_INVALID_NAME, done, "Deleting in-use effect slot %u",
                        effectslots[i]);
    }

    // All effectslots are valid
    RemoveActiveEffectSlots(effectslots, n, context);
    for(i = 0;i < n;i++)
    {
        if((slot=LookupEffectSlot(context, effectslots[i])) == NULL)
            continue;
        VECTOR_ELEM(context->EffectSlotList, effectslots[i]-1) = NULL;

        DeinitEffectSlot(slot);

        memset(slot, 0, sizeof(*slot));
        al_free(slot);
    }

done:
    UnlockEffectSlotList(context);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint effectslot)
{
    ALCcontext *context;
    ALboolean  ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    LockEffectSlotList(context);
    ret = (LookupEffectSlot(context, effectslot) ? AL_TRUE : AL_FALSE);
    UnlockEffectSlotList(context);

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

    almtx_lock(&context->PropLock);
    LockEffectSlotList(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    case AL_EFFECTSLOT_EFFECT:
        device = context->Device;

        LockEffectList(device);
        effect = (value ? LookupEffect(device, value) : NULL);
        if(!(value == 0 || effect != NULL))
        {
            UnlockEffectList(device);
            SETERR_GOTO(context, AL_INVALID_VALUE, done, "Invalid effect ID %u", value);
        }
        err = InitializeEffect(context, slot, effect);
        UnlockEffectList(device);

        if(err != AL_NO_ERROR)
            SETERR_GOTO(context, err, done, "Effect initialization failed");
        break;

    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        if(!(value == AL_TRUE || value == AL_FALSE))
            SETERR_GOTO(context, AL_INVALID_VALUE, done,
                        "Effect slot auxiliary send auto out of range");
        slot->AuxSendAuto = value;
        break;

    default:
        SETERR_GOTO(context, AL_INVALID_ENUM, done, "Invalid effect slot integer property 0x%04x",
                    param);
    }
    DO_UPDATEPROPS();

done:
    UnlockEffectSlotList(context);
    almtx_unlock(&context->PropLock);
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

    LockEffectSlotList(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot integer-vector property 0x%04x",
                   param);
    }

done:
    UnlockEffectSlotList(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    almtx_lock(&context->PropLock);
    LockEffectSlotList(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        if(!(value >= 0.0f && value <= 1.0f))
            SETERR_GOTO(context, AL_INVALID_VALUE, done, "Effect slot gain out of range");
        slot->Gain = value;
        break;

    default:
        SETERR_GOTO(context, AL_INVALID_ENUM, done, "Invalid effect slot float property 0x%04x",
                    param);
    }
    DO_UPDATEPROPS();

done:
    UnlockEffectSlotList(context);
    almtx_unlock(&context->PropLock);
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

    LockEffectSlotList(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot float-vector property 0x%04x",
                   param);
    }

done:
    UnlockEffectSlotList(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotList(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
        *value = slot->AuxSendAuto;
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot integer property 0x%04x", param);
    }

done:
    UnlockEffectSlotList(context);
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

    LockEffectSlotList(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot integer-vector property 0x%04x",
                   param);
    }

done:
    UnlockEffectSlotList(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *value)
{
    ALCcontext *context;
    ALeffectslot *slot;

    context = GetContextRef();
    if(!context) return;

    LockEffectSlotList(context);
    if((slot=LookupEffectSlot(context, effectslot)) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    case AL_EFFECTSLOT_GAIN:
        *value = slot->Gain;
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot float property 0x%04x", param);
    }

done:
    UnlockEffectSlotList(context);
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

    LockEffectSlotList(context);
    if(LookupEffectSlot(context, effectslot) == NULL)
        SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid effect slot ID %u", effectslot);
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid effect slot float-vector property 0x%04x",
                   param);
    }

done:
    UnlockEffectSlotList(context);
    ALCcontext_DecRef(context);
}


ALenum InitializeEffect(ALCcontext *Context, ALeffectslot *EffectSlot, ALeffect *effect)
{
    ALCdevice *Device = Context->Device;
    ALenum newtype = (effect ? effect->type : AL_EFFECT_NULL);
    struct ALeffectslotProps *props;
    ALeffectState *State;

    if(newtype != EffectSlot->Effect.Type)
    {
        EffectStateFactory *factory;

        factory = getFactoryByType(newtype);
        if(!factory)
        {
            ERR("Failed to find factory for effect type 0x%04x\n", newtype);
            return AL_INVALID_ENUM;
        }
        State = EffectStateFactory_create(factory);
        if(!State) return AL_OUT_OF_MEMORY;

        START_MIXER_MODE();
        almtx_lock(&Device->BackendLock);
        State->OutBuffer = Device->Dry.Buffer;
        State->OutChannels = Device->Dry.NumChannels;
        if(V(State,deviceUpdate)(Device) == AL_FALSE)
        {
            almtx_unlock(&Device->BackendLock);
            LEAVE_MIXER_MODE();
            ALeffectState_DecRef(State);
            return AL_OUT_OF_MEMORY;
        }
        almtx_unlock(&Device->BackendLock);
        END_MIXER_MODE();

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
    props = ATOMIC_LOAD_SEQ(&Context->FreeEffectslotProps);
    while(props)
    {
        if(props->State)
            ALeffectState_DecRef(props->State);
        props->State = NULL;
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

void ALeffectState_DecRef(ALeffectState *state)
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


static void AddActiveEffectSlots(const ALuint *slotids, ALsizei count, ALCcontext *context)
{
    struct ALeffectslotArray *curarray = ATOMIC_LOAD(&context->ActiveAuxSlots,
                                                     almemory_order_acquire);
    struct ALeffectslotArray *newarray = NULL;
    ALsizei newcount = curarray->count + count;
    ALCdevice *device = context->Device;
    ALsizei i, j;

    /* Insert the new effect slots into the head of the array, followed by the
     * existing ones.
     */
    newarray = al_calloc(DEF_ALIGN, FAM_SIZE(struct ALeffectslotArray, slot, newcount));
    newarray->count = newcount;
    for(i = 0;i < count;i++)
        newarray->slot[i] = LookupEffectSlot(context, slotids[i]);
    for(j = 0;i < newcount;)
        newarray->slot[i++] = curarray->slot[j++];
    /* Remove any duplicates (first instance of each will be kept). */
    for(i = 1;i < newcount;i++)
    {
        for(j = i;j != 0;)
        {
            if(UNLIKELY(newarray->slot[i] == newarray->slot[--j]))
            {
                newcount--;
                for(j = i;j < newcount;j++)
                    newarray->slot[j] = newarray->slot[j+1];
                i--;
                break;
            }
        }
    }

    /* Reallocate newarray if the new size ended up smaller from duplicate
     * removal.
     */
    if(UNLIKELY(newcount < newarray->count))
    {
        struct ALeffectslotArray *tmpnewarray = al_calloc(DEF_ALIGN,
            FAM_SIZE(struct ALeffectslotArray, slot, newcount));
        memcpy(tmpnewarray, newarray, FAM_SIZE(struct ALeffectslotArray, slot, newcount));
        al_free(newarray);
        newarray = tmpnewarray;
        newarray->count = newcount;
    }

    curarray = ATOMIC_EXCHANGE_PTR(&context->ActiveAuxSlots, newarray, almemory_order_acq_rel);
    while((ATOMIC_LOAD(&device->MixCount, almemory_order_acquire)&1))
        althrd_yield();
    al_free(curarray);
}

static void RemoveActiveEffectSlots(const ALuint *slotids, ALsizei count, ALCcontext *context)
{
    struct ALeffectslotArray *curarray = ATOMIC_LOAD(&context->ActiveAuxSlots,
                                                     almemory_order_acquire);
    struct ALeffectslotArray *newarray = NULL;
    ALCdevice *device = context->Device;
    ALsizei i, j;

    /* Don't shrink the allocated array size since we don't know how many (if
     * any) of the effect slots to remove are in the array.
     */
    newarray = al_calloc(DEF_ALIGN, FAM_SIZE(struct ALeffectslotArray, slot, curarray->count));
    newarray->count = 0;
    for(i = 0;i < curarray->count;i++)
    {
        /* Insert this slot into the new array only if it's not one to remove. */
        ALeffectslot *slot = curarray->slot[i];
        for(j = count;j != 0;)
        {
            if(slot->id == slotids[--j])
                goto skip_ins;
        }
        newarray->slot[newarray->count++] = slot;
    skip_ins: ;
    }

    /* TODO: Could reallocate newarray now that we know it's needed size. */

    curarray = ATOMIC_EXCHANGE_PTR(&context->ActiveAuxSlots, newarray, almemory_order_acq_rel);
    while((ATOMIC_LOAD(&device->MixCount, almemory_order_acquire)&1))
        althrd_yield();
    al_free(curarray);
}


ALenum InitEffectSlot(ALeffectslot *slot)
{
    EffectStateFactory *factory;

    slot->Effect.Type = AL_EFFECT_NULL;

    factory = getFactoryByType(AL_EFFECT_NULL);
    slot->Effect.State = EffectStateFactory_create(factory);
    if(!slot->Effect.State) return AL_OUT_OF_MEMORY;

    slot->Gain = 1.0;
    slot->AuxSendAuto = AL_TRUE;
    ATOMIC_FLAG_TEST_AND_SET(&slot->PropsClean, almemory_order_relaxed);
    InitRef(&slot->ref, 0);

    ATOMIC_INIT(&slot->Update, NULL);

    slot->Params.Gain = 1.0f;
    slot->Params.AuxSendAuto = AL_TRUE;
    ALeffectState_IncRef(slot->Effect.State);
    slot->Params.EffectState = slot->Effect.State;
    slot->Params.RoomRolloff = 0.0f;
    slot->Params.DecayTime = 0.0f;
    slot->Params.DecayLFRatio = 0.0f;
    slot->Params.DecayHFRatio = 0.0f;
    slot->Params.DecayHFLimit = AL_FALSE;
    slot->Params.AirAbsorptionGainHF = 1.0f;

    return AL_NO_ERROR;
}

void DeinitEffectSlot(ALeffectslot *slot)
{
    struct ALeffectslotProps *props;

    props = ATOMIC_LOAD_SEQ(&slot->Update);
    if(props)
    {
        if(props->State) ALeffectState_DecRef(props->State);
        TRACE("Freed unapplied AuxiliaryEffectSlot update %p\n", props);
        al_free(props);
    }

    ALeffectState_DecRef(slot->Effect.State);
    if(slot->Params.EffectState)
        ALeffectState_DecRef(slot->Params.EffectState);
}

void UpdateEffectSlotProps(ALeffectslot *slot, ALCcontext *context)
{
    struct ALeffectslotProps *props;
    ALeffectState *oldstate;

    /* Get an unused property container, or allocate a new one as needed. */
    props = ATOMIC_LOAD(&context->FreeEffectslotProps, almemory_order_relaxed);
    if(!props)
        props = al_calloc(16, sizeof(*props));
    else
    {
        struct ALeffectslotProps *next;
        do {
            next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_PTR_WEAK(&context->FreeEffectslotProps, &props, next,
                almemory_order_seq_cst, almemory_order_acquire) == 0);
    }

    /* Copy in current property values. */
    props->Gain = slot->Gain;
    props->AuxSendAuto = slot->AuxSendAuto;

    props->Type = slot->Effect.Type;
    props->Props = slot->Effect.Props;
    /* Swap out any stale effect state object there may be in the container, to
     * delete it.
     */
    ALeffectState_IncRef(slot->Effect.State);
    oldstate = props->State;
    props->State = slot->Effect.State;

    /* Set the new container for updating internal parameters. */
    props = ATOMIC_EXCHANGE_PTR(&slot->Update, props, almemory_order_acq_rel);
    if(props)
    {
        /* If there was an unused update container, put it back in the
         * freelist.
         */
        ATOMIC_REPLACE_HEAD(struct ALeffectslotProps*, &context->FreeEffectslotProps, props);
    }

    if(oldstate)
        ALeffectState_DecRef(oldstate);
}

void UpdateAllEffectSlotProps(ALCcontext *context)
{
    struct ALeffectslotArray *auxslots;
    ALsizei i;

    LockEffectSlotList(context);
    auxslots = ATOMIC_LOAD(&context->ActiveAuxSlots, almemory_order_acquire);
    for(i = 0;i < auxslots->count;i++)
    {
        ALeffectslot *slot = auxslots->slot[i];
        if(!ATOMIC_FLAG_TEST_AND_SET(&slot->PropsClean, almemory_order_acq_rel))
            UpdateEffectSlotProps(slot, context);
    }
    UnlockEffectSlotList(context);
}

ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *context)
{
    ALeffectslotPtr *iter = VECTOR_BEGIN(context->EffectSlotList);
    ALeffectslotPtr *end = VECTOR_END(context->EffectSlotList);
    size_t leftover = 0;

    for(;iter != end;iter++)
    {
        ALeffectslot *slot = *iter;
        if(!slot) continue;
        *iter = NULL;

        DeinitEffectSlot(slot);

        memset(slot, 0, sizeof(*slot));
        al_free(slot);
        ++leftover;
    }
    if(leftover > 0)
        WARN("(%p) Deleted "SZFMT" AuxiliaryEffectSlot%s\n", context, leftover, (leftover==1)?"":"s");
}
