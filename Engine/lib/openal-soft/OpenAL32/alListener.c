/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
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

#include "alMain.h"
#include "AL/alc.h"
#include "alError.h"
#include "alListener.h"
#include "alSource.h"

AL_API ALvoid AL_APIENTRY alListenerf(ALenum param, ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(param)
    {
    case AL_GAIN:
        if(!(value >= 0.0f && isfinite(value)))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        context->Listener->Gain = value;
        break;

    case AL_METERS_PER_UNIT:
        if(!(value >= 0.0f && isfinite(value)))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        context->Listener->MetersPerUnit = value;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(param)
    {
    case AL_POSITION:
        if(!(isfinite(value1) && isfinite(value2) && isfinite(value3)))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        context->Listener->Position[0] = value1;
        context->Listener->Position[1] = value2;
        context->Listener->Position[2] = value3;
        break;

    case AL_VELOCITY:
        if(!(isfinite(value1) && isfinite(value2) && isfinite(value3)))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        context->Listener->Velocity[0] = value1;
        context->Listener->Velocity[1] = value2;
        context->Listener->Velocity[2] = value3;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alListenerfv(ALenum param, const ALfloat *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(param)
        {
        case AL_GAIN:
        case AL_METERS_PER_UNIT:
            alListenerf(param, values[0]);
            return;

        case AL_POSITION:
        case AL_VELOCITY:
            alListener3f(param, values[0], values[1], values[2]);
            return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_ORIENTATION:
        if(!(isfinite(values[0]) && isfinite(values[1]) && isfinite(values[2]) &&
             isfinite(values[3]) && isfinite(values[4]) && isfinite(values[5])))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        /* AT then UP */
        context->Listener->Forward[0] = values[0];
        context->Listener->Forward[1] = values[1];
        context->Listener->Forward[2] = values[2];
        context->Listener->Up[0] = values[3];
        context->Listener->Up[1] = values[4];
        context->Listener->Up[2] = values[5];
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alListeneri(ALenum param, ALint UNUSED(value))
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alListener3i(ALenum param, ALint value1, ALint value2, ALint value3)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_POSITION:
    case AL_VELOCITY:
        alListener3f(param, (ALfloat)value1, (ALfloat)value2, (ALfloat)value3);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alListeneriv(ALenum param, const ALint *values)
{
    ALCcontext *context;

    if(values)
    {
        ALfloat fvals[6];
        switch(param)
        {
        case AL_POSITION:
        case AL_VELOCITY:
            alListener3f(param, (ALfloat)values[0], (ALfloat)values[1], (ALfloat)values[2]);
            return;

        case AL_ORIENTATION:
            fvals[0] = (ALfloat)values[0];
            fvals[1] = (ALfloat)values[1];
            fvals[2] = (ALfloat)values[2];
            fvals[3] = (ALfloat)values[3];
            fvals[4] = (ALfloat)values[4];
            fvals[5] = (ALfloat)values[5];
            alListenerfv(param, fvals);
            return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);

done:
    WriteUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetListenerf(ALenum param, ALfloat *value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(value))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_GAIN:
        *value = context->Listener->Gain;
        break;

    case AL_METERS_PER_UNIT:
        *value = context->Listener->MetersPerUnit;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetListener3f(ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(value1 && value2 && value3))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_POSITION:
        *value1 = context->Listener->Position[0];
        *value2 = context->Listener->Position[1];
        *value3 = context->Listener->Position[2];
        break;

    case AL_VELOCITY:
        *value1 = context->Listener->Velocity[0];
        *value2 = context->Listener->Velocity[1];
        *value3 = context->Listener->Velocity[2];
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetListenerfv(ALenum param, ALfloat *values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_GAIN:
    case AL_METERS_PER_UNIT:
        alGetListenerf(param, values);
        return;

    case AL_POSITION:
    case AL_VELOCITY:
        alGetListener3f(param, values+0, values+1, values+2);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_ORIENTATION:
        // AT then UP
        values[0] = context->Listener->Forward[0];
        values[1] = context->Listener->Forward[1];
        values[2] = context->Listener->Forward[2];
        values[3] = context->Listener->Up[0];
        values[4] = context->Listener->Up[1];
        values[5] = context->Listener->Up[2];
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetListeneri(ALenum param, ALint *value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(value))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetListener3i(ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(value1 && value2 && value3))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch (param)
    {
    case AL_POSITION:
        *value1 = (ALint)context->Listener->Position[0];
        *value2 = (ALint)context->Listener->Position[1];
        *value3 = (ALint)context->Listener->Position[2];
        break;

    case AL_VELOCITY:
        *value1 = (ALint)context->Listener->Velocity[0];
        *value2 = (ALint)context->Listener->Velocity[1];
        *value3 = (ALint)context->Listener->Velocity[2];
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetListeneriv(ALenum param, ALint* values)
{
    ALCcontext *context;

    switch(param)
    {
    case AL_POSITION:
    case AL_VELOCITY:
        alGetListener3i(param, values+0, values+1, values+2);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    ReadLock(&context->PropLock);
    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_ORIENTATION:
        // AT then UP
        values[0] = (ALint)context->Listener->Forward[0];
        values[1] = (ALint)context->Listener->Forward[1];
        values[2] = (ALint)context->Listener->Forward[2];
        values[3] = (ALint)context->Listener->Up[0];
        values[4] = (ALint)context->Listener->Up[1];
        values[5] = (ALint)context->Listener->Up[2];
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ReadUnlock(&context->PropLock);
    ALCcontext_DecRef(context);
}


void UpdateListenerProps(ALCcontext *context)
{
    ALlistener *listener = context->Listener;
    struct ALlistenerProps *props;

    /* Get an unused proprty container, or allocate a new one as needed. */
    props = ATOMIC_LOAD(&listener->FreeList, almemory_order_acquire);
    if(!props)
        props = al_calloc(16, sizeof(*props));
    else
    {
        struct ALlistenerProps *next;
        do {
            next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALlistenerProps*,
                &listener->FreeList, &props, next, almemory_order_seq_cst,
                almemory_order_consume) == 0);
    }

    /* Copy in current property values. */
    ATOMIC_STORE(&props->Position[0], listener->Position[0], almemory_order_relaxed);
    ATOMIC_STORE(&props->Position[1], listener->Position[1], almemory_order_relaxed);
    ATOMIC_STORE(&props->Position[2], listener->Position[2], almemory_order_relaxed);

    ATOMIC_STORE(&props->Velocity[0], listener->Velocity[0], almemory_order_relaxed);
    ATOMIC_STORE(&props->Velocity[1], listener->Velocity[1], almemory_order_relaxed);
    ATOMIC_STORE(&props->Velocity[2], listener->Velocity[2], almemory_order_relaxed);

    ATOMIC_STORE(&props->Forward[0], listener->Forward[0], almemory_order_relaxed);
    ATOMIC_STORE(&props->Forward[1], listener->Forward[1], almemory_order_relaxed);
    ATOMIC_STORE(&props->Forward[2], listener->Forward[2], almemory_order_relaxed);
    ATOMIC_STORE(&props->Up[0], listener->Up[0], almemory_order_relaxed);
    ATOMIC_STORE(&props->Up[1], listener->Up[1], almemory_order_relaxed);
    ATOMIC_STORE(&props->Up[2], listener->Up[2], almemory_order_relaxed);

    ATOMIC_STORE(&props->Gain, listener->Gain, almemory_order_relaxed);
    ATOMIC_STORE(&props->MetersPerUnit, listener->MetersPerUnit, almemory_order_relaxed);

    ATOMIC_STORE(&props->DopplerFactor, context->DopplerFactor, almemory_order_relaxed);
    ATOMIC_STORE(&props->DopplerVelocity, context->DopplerVelocity, almemory_order_relaxed);
    ATOMIC_STORE(&props->SpeedOfSound, context->SpeedOfSound, almemory_order_relaxed);

    ATOMIC_STORE(&props->SourceDistanceModel, context->SourceDistanceModel, almemory_order_relaxed);
    ATOMIC_STORE(&props->DistanceModel, context->DistanceModel, almemory_order_relaxed);

    /* Set the new container for updating internal parameters. */
    props = ATOMIC_EXCHANGE(struct ALlistenerProps*, &listener->Update, props, almemory_order_acq_rel);
    if(props)
    {
        /* If there was an unused update container, put it back in the
         * freelist.
         */
        struct ALlistenerProps *first = ATOMIC_LOAD(&listener->FreeList);
        do {
            ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALlistenerProps*,
                &listener->FreeList, &first, props) == 0);
    }
}
