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
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
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
#include "alSource.h"


static ALenum AddEffectSlotArray(ALCcontext *Context, ALsizei count, const ALuint *slots);
static ALvoid RemoveEffectSlotArray(ALCcontext *Context, ALeffectslot *slot);


AL_API ALvoid AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots)
{
    ALCcontext *Context;
    ALsizei    cur = 0;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        ALenum err;

        CHECK_VALUE(Context, n >= 0);
        for(cur = 0;cur < n;cur++)
        {
            ALeffectslot *slot = al_calloc(16, sizeof(ALeffectslot));
            err = AL_OUT_OF_MEMORY;
            if(!slot || (err=InitEffectSlot(slot)) != AL_NO_ERROR)
            {
                al_free(slot);
                al_throwerr(Context, err);
                break;
            }

            err = NewThunkEntry(&slot->id);
            if(err == AL_NO_ERROR)
                err = InsertUIntMapEntry(&Context->EffectSlotMap, slot->id, slot);
            if(err != AL_NO_ERROR)
            {
                FreeThunkEntry(slot->id);
                ALeffectState_Destroy(slot->EffectState);
                al_free(slot);

                al_throwerr(Context, err);
            }

            effectslots[cur] = slot->id;
        }
        err = AddEffectSlotArray(Context, n, effectslots);
        if(err != AL_NO_ERROR)
            al_throwerr(Context, err);
    }
    al_catchany()
    {
        if(cur > 0)
            alDeleteAuxiliaryEffectSlots(cur, effectslots);
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, const ALuint *effectslots)
{
    ALCcontext *Context;
    ALeffectslot *slot;
    ALsizei i;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        CHECK_VALUE(Context, n >= 0);
        for(i = 0;i < n;i++)
        {
            if((slot=LookupEffectSlot(Context, effectslots[i])) == NULL)
                al_throwerr(Context, AL_INVALID_NAME);
            if(slot->ref != 0)
                al_throwerr(Context, AL_INVALID_OPERATION);
        }

        // All effectslots are valid
        for(i = 0;i < n;i++)
        {
            if((slot=RemoveEffectSlot(Context, effectslots[i])) == NULL)
                continue;
            FreeThunkEntry(slot->id);

            RemoveEffectSlotArray(Context, slot);
            ALeffectState_Destroy(slot->EffectState);

            memset(slot, 0, sizeof(*slot));
            al_free(slot);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint effectslot)
{
    ALCcontext *Context;
    ALboolean  result;

    Context = GetContextRef();
    if(!Context) return AL_FALSE;

    result = (LookupEffectSlot(Context, effectslot) ? AL_TRUE : AL_FALSE);

    ALCcontext_DecRef(Context);

    return result;
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint value)
{
    ALCcontext *Context;
    ALeffectslot *Slot;
    ALeffect *effect = NULL;
    ALenum err;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        ALCdevice *device = Context->Device;
        if((Slot=LookupEffectSlot(Context, effectslot)) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT:
            CHECK_VALUE(Context, value == 0 || (effect=LookupEffect(device, value)) != NULL);

            err = InitializeEffect(device, Slot, effect);
            if(err != AL_NO_ERROR)
                al_throwerr(Context, err);
            Context->UpdateSources = AL_TRUE;
            break;

        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            CHECK_VALUE(Context, value == AL_TRUE || value == AL_FALSE);

            Slot->AuxSendAuto = value;
            Context->UpdateSources = AL_TRUE;
            break;

        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, const ALint *values)
{
    ALCcontext *Context;

    switch(param)
    {
        case AL_EFFECTSLOT_EFFECT:
        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            alAuxiliaryEffectSloti(effectslot, param, values[0]);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if(LookupEffectSlot(Context, effectslot) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat value)
{
    ALCcontext *Context;
    ALeffectslot *Slot;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if((Slot=LookupEffectSlot(Context, effectslot)) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            CHECK_VALUE(Context, value >= 0.0f && value <= 1.0f);

            Slot->Gain = value;
            Slot->NeedsUpdate = AL_TRUE;
            break;

        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, const ALfloat *values)
{
    ALCcontext *Context;

    switch(param)
    {
        case AL_EFFECTSLOT_GAIN:
            alAuxiliaryEffectSlotf(effectslot, param, values[0]);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if(LookupEffectSlot(Context, effectslot) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *value)
{
    ALCcontext *Context;
    ALeffectslot *Slot;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if((Slot=LookupEffectSlot(Context, effectslot)) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT:
            *value = Slot->effect.id;
            break;

        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            *value = Slot->AuxSendAuto;
            break;

        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *values)
{
    ALCcontext *Context;

    switch(param)
    {
        case AL_EFFECTSLOT_EFFECT:
        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            alGetAuxiliaryEffectSloti(effectslot, param, values);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if(LookupEffectSlot(Context, effectslot) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *value)
{
    ALCcontext *Context;
    ALeffectslot *Slot;

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if((Slot=LookupEffectSlot(Context, effectslot)) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            *value = Slot->Gain;
            break;

        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *values)
{
    ALCcontext *Context;

    switch(param)
    {
        case AL_EFFECTSLOT_GAIN:
            alGetAuxiliaryEffectSlotf(effectslot, param, values);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    al_try
    {
        if(LookupEffectSlot(Context, effectslot) == NULL)
            al_throwerr(Context, AL_INVALID_NAME);
        switch(param)
        {
        default:
            al_throwerr(Context, AL_INVALID_ENUM);
        }
    }
    al_endtry;

    ALCcontext_DecRef(Context);
}


static ALvoid NoneDestroy(ALeffectState *State)
{ free(State); }
static ALboolean NoneDeviceUpdate(ALeffectState *State, ALCdevice *Device)
{
    return AL_TRUE;
    (void)State;
    (void)Device;
}
static ALvoid NoneUpdate(ALeffectState *State, ALCdevice *Device, const ALeffectslot *Slot)
{
    (void)State;
    (void)Device;
    (void)Slot;
}
static ALvoid NoneProcess(ALeffectState *State, ALuint SamplesToDo, const ALfloat *RESTRICT SamplesIn, ALfloat (*RESTRICT SamplesOut)[BUFFERSIZE])
{
    (void)State;
    (void)SamplesToDo;
    (void)SamplesIn;
    (void)SamplesOut;
}
ALeffectState *NoneCreate(void)
{
    ALeffectState *state;

    state = calloc(1, sizeof(*state));
    if(!state)
        return NULL;

    state->Destroy = NoneDestroy;
    state->DeviceUpdate = NoneDeviceUpdate;
    state->Update = NoneUpdate;
    state->Process = NoneProcess;

    return state;
}


static ALvoid RemoveEffectSlotArray(ALCcontext *Context, ALeffectslot *slot)
{
    ALeffectslot **slotlist, **slotlistend;

    LockContext(Context);
    slotlist = Context->ActiveEffectSlots;
    slotlistend = slotlist + Context->ActiveEffectSlotCount;
    while(slotlist != slotlistend)
    {
        if(*slotlist == slot)
        {
            *slotlist = *(--slotlistend);
            Context->ActiveEffectSlotCount--;
            break;
        }
        slotlist++;
    }
    UnlockContext(Context);
}

static ALenum AddEffectSlotArray(ALCcontext *Context, ALsizei count, const ALuint *slots)
{
    ALsizei i;

    LockContext(Context);
    if(count > Context->MaxActiveEffectSlots-Context->ActiveEffectSlotCount)
    {
        ALsizei newcount;
        void *temp = NULL;

        newcount = Context->MaxActiveEffectSlots ? (Context->MaxActiveEffectSlots<<1) : 1;
        if(newcount > Context->MaxActiveEffectSlots)
            temp = realloc(Context->ActiveEffectSlots,
                           newcount * sizeof(*Context->ActiveEffectSlots));
        if(!temp)
        {
            UnlockContext(Context);
            return AL_OUT_OF_MEMORY;
        }
        Context->ActiveEffectSlots = temp;
        Context->MaxActiveEffectSlots = newcount;
    }
    for(i = 0;i < count;i++)
    {
        ALeffectslot *slot = LookupEffectSlot(Context, slots[i]);
        assert(slot != NULL);
        Context->ActiveEffectSlots[Context->ActiveEffectSlotCount++] = slot;
    }
    UnlockContext(Context);
    return AL_NO_ERROR;
}

ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect)
{
    ALenum newtype = (effect ? effect->type : AL_EFFECT_NULL);
    ALeffectState *State = NULL;
    ALenum err = AL_NO_ERROR;

    ALCdevice_Lock(Device);
    if(newtype == AL_EFFECT_NULL && EffectSlot->effect.type != AL_EFFECT_NULL)
    {
        State = NoneCreate();
        if(!State) err = AL_OUT_OF_MEMORY;
    }
    else if(newtype == AL_EFFECT_EAXREVERB || newtype == AL_EFFECT_REVERB)
    {
        if(EffectSlot->effect.type != AL_EFFECT_EAXREVERB && EffectSlot->effect.type != AL_EFFECT_REVERB)
        {
            State = ReverbCreate();
            if(!State) err = AL_OUT_OF_MEMORY;
        }
    }
    else if(newtype == AL_EFFECT_ECHO && EffectSlot->effect.type != AL_EFFECT_ECHO)
    {
        State = EchoCreate();
        if(!State) err = AL_OUT_OF_MEMORY;
    }
    else if(newtype == AL_EFFECT_RING_MODULATOR && EffectSlot->effect.type != AL_EFFECT_RING_MODULATOR)
    {
        State = ModulatorCreate();
        if(!State) err = AL_OUT_OF_MEMORY;
    }
    else if(newtype == AL_EFFECT_DEDICATED_DIALOGUE || newtype == AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT)
    {
        if(EffectSlot->effect.type != AL_EFFECT_DEDICATED_DIALOGUE && EffectSlot->effect.type != AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT)
        {
            State = DedicatedCreate();
            if(!State) err = AL_OUT_OF_MEMORY;
        }
    }

    if(err != AL_NO_ERROR)
    {
        ALCdevice_Unlock(Device);
        return err;
    }

    if(State)
    {
        FPUCtl oldMode;
        SetMixerFPUMode(&oldMode);

        if(ALeffectState_DeviceUpdate(State, Device) == AL_FALSE)
        {
            RestoreFPUMode(&oldMode);
            ALCdevice_Unlock(Device);
            ALeffectState_Destroy(State);
            return AL_OUT_OF_MEMORY;
        }
        State = ExchangePtr((XchgPtr*)&EffectSlot->EffectState, State);

        if(!effect)
            memset(&EffectSlot->effect, 0, sizeof(EffectSlot->effect));
        else
            memcpy(&EffectSlot->effect, effect, sizeof(*effect));
        /* FIXME: This should be done asynchronously, but since the EffectState
         * object was changed, it needs an update before its Process method can
         * be called. */
        EffectSlot->NeedsUpdate = AL_FALSE;
        ALeffectState_Update(EffectSlot->EffectState, Device, EffectSlot);
        ALCdevice_Unlock(Device);

        RestoreFPUMode(&oldMode);

        ALeffectState_Destroy(State);
        State = NULL;
    }
    else
    {
        if(!effect)
            memset(&EffectSlot->effect, 0, sizeof(EffectSlot->effect));
        else
            memcpy(&EffectSlot->effect, effect, sizeof(*effect));
        ALCdevice_Unlock(Device);
        EffectSlot->NeedsUpdate = AL_TRUE;
    }

    return AL_NO_ERROR;
}


ALenum InitEffectSlot(ALeffectslot *slot)
{
    ALint i, c;

    if(!(slot->EffectState=NoneCreate()))
        return AL_OUT_OF_MEMORY;

    slot->Gain = 1.0;
    slot->AuxSendAuto = AL_TRUE;
    slot->NeedsUpdate = AL_FALSE;
    for(c = 0;c < 1;c++)
    {
        for(i = 0;i < BUFFERSIZE;i++)
            slot->WetBuffer[c][i] = 0.0f;
        slot->ClickRemoval[c] = 0.0f;
        slot->PendingClicks[c] = 0.0f;
    }
    slot->ref = 0;

    return AL_NO_ERROR;
}

ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context)
{
    ALsizei pos;
    for(pos = 0;pos < Context->EffectSlotMap.size;pos++)
    {
        ALeffectslot *temp = Context->EffectSlotMap.array[pos].value;
        Context->EffectSlotMap.array[pos].value = NULL;

        ALeffectState_Destroy(temp->EffectState);

        FreeThunkEntry(temp->id);
        memset(temp, 0, sizeof(ALeffectslot));
        al_free(temp);
    }
}
