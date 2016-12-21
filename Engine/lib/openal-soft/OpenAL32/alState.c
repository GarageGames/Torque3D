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

#include <stdlib.h>
#include "alMain.h"
#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"
#include "alError.h"
#include "alListener.h"
#include "alSource.h"
#include "alAuxEffectSlot.h"

#include "backends/base.h"


static const ALchar alVendor[] = "OpenAL Community";
static const ALchar alVersion[] = "1.1 ALSOFT "ALSOFT_VERSION;
static const ALchar alRenderer[] = "OpenAL Soft";

// Error Messages
static const ALchar alNoError[] = "No Error";
static const ALchar alErrInvalidName[] = "Invalid Name";
static const ALchar alErrInvalidEnum[] = "Invalid Enum";
static const ALchar alErrInvalidValue[] = "Invalid Value";
static const ALchar alErrInvalidOp[] = "Invalid Operation";
static const ALchar alErrOutOfMemory[] = "Out of Memory";

AL_API ALvoid AL_APIENTRY alEnable(ALenum capability)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        context->SourceDistanceModel = AL_TRUE;
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

AL_API ALvoid AL_APIENTRY alDisable(ALenum capability)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    WriteLock(&context->PropLock);
    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        context->SourceDistanceModel = AL_FALSE;
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

AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability)
{
    ALCcontext *context;
    ALboolean value=AL_FALSE;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        value = context->SourceDistanceModel;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum pname)
{
    ALCcontext *context;
    ALboolean value=AL_FALSE;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    switch(pname)
    {
    case AL_DOPPLER_FACTOR:
        if(context->DopplerFactor != 0.0f)
            value = AL_TRUE;
        break;

    case AL_DOPPLER_VELOCITY:
        if(context->DopplerVelocity != 0.0f)
            value = AL_TRUE;
        break;

    case AL_DISTANCE_MODEL:
        if(context->DistanceModel == AL_INVERSE_DISTANCE_CLAMPED)
            value = AL_TRUE;
        break;

    case AL_SPEED_OF_SOUND:
        if(context->SpeedOfSound != 0.0f)
            value = AL_TRUE;
        break;

    case AL_DEFERRED_UPDATES_SOFT:
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
            value = AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        if(GAIN_MIX_MAX/context->GainBoost != 0.0f)
            value = AL_TRUE;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALdouble AL_APIENTRY alGetDouble(ALenum pname)
{
    ALCcontext *context;
    ALdouble value = 0.0;

    context = GetContextRef();
    if(!context) return 0.0;

    switch(pname)
    {
    case AL_DOPPLER_FACTOR:
        value = (ALdouble)context->DopplerFactor;
        break;

    case AL_DOPPLER_VELOCITY:
        value = (ALdouble)context->DopplerVelocity;
        break;

    case AL_DISTANCE_MODEL:
        value = (ALdouble)context->DistanceModel;
        break;

    case AL_SPEED_OF_SOUND:
        value = (ALdouble)context->SpeedOfSound;
        break;

    case AL_DEFERRED_UPDATES_SOFT:
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
            value = (ALdouble)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALdouble)GAIN_MIX_MAX/context->GainBoost;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALfloat AL_APIENTRY alGetFloat(ALenum pname)
{
    ALCcontext *context;
    ALfloat value = 0.0f;

    context = GetContextRef();
    if(!context) return 0.0f;

    switch(pname)
    {
    case AL_DOPPLER_FACTOR:
        value = context->DopplerFactor;
        break;

    case AL_DOPPLER_VELOCITY:
        value = context->DopplerVelocity;
        break;

    case AL_DISTANCE_MODEL:
        value = (ALfloat)context->DistanceModel;
        break;

    case AL_SPEED_OF_SOUND:
        value = context->SpeedOfSound;
        break;

    case AL_DEFERRED_UPDATES_SOFT:
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
            value = (ALfloat)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = GAIN_MIX_MAX/context->GainBoost;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALint AL_APIENTRY alGetInteger(ALenum pname)
{
    ALCcontext *context;
    ALint value = 0;

    context = GetContextRef();
    if(!context) return 0;

    switch(pname)
    {
    case AL_DOPPLER_FACTOR:
        value = (ALint)context->DopplerFactor;
        break;

    case AL_DOPPLER_VELOCITY:
        value = (ALint)context->DopplerVelocity;
        break;

    case AL_DISTANCE_MODEL:
        value = (ALint)context->DistanceModel;
        break;

    case AL_SPEED_OF_SOUND:
        value = (ALint)context->SpeedOfSound;
        break;

    case AL_DEFERRED_UPDATES_SOFT:
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
            value = (ALint)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALint)(GAIN_MIX_MAX/context->GainBoost);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALint64SOFT AL_APIENTRY alGetInteger64SOFT(ALenum pname)
{
    ALCcontext *context;
    ALint64SOFT value = 0;

    context = GetContextRef();
    if(!context) return 0;

    switch(pname)
    {
    case AL_DOPPLER_FACTOR:
        value = (ALint64SOFT)context->DopplerFactor;
        break;

    case AL_DOPPLER_VELOCITY:
        value = (ALint64SOFT)context->DopplerVelocity;
        break;

    case AL_DISTANCE_MODEL:
        value = (ALint64SOFT)context->DistanceModel;
        break;

    case AL_SPEED_OF_SOUND:
        value = (ALint64SOFT)context->SpeedOfSound;
        break;

    case AL_DEFERRED_UPDATES_SOFT:
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
            value = (ALint64SOFT)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALint64SOFT)(GAIN_MIX_MAX/context->GainBoost);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALvoid AL_APIENTRY alGetBooleanv(ALenum pname, ALboolean *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
            case AL_DOPPLER_VELOCITY:
            case AL_DISTANCE_MODEL:
            case AL_SPEED_OF_SOUND:
            case AL_DEFERRED_UPDATES_SOFT:
            case AL_GAIN_LIMIT_SOFT:
                values[0] = alGetBoolean(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(pname)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetDoublev(ALenum pname, ALdouble *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
            case AL_DOPPLER_VELOCITY:
            case AL_DISTANCE_MODEL:
            case AL_SPEED_OF_SOUND:
            case AL_DEFERRED_UPDATES_SOFT:
            case AL_GAIN_LIMIT_SOFT:
                values[0] = alGetDouble(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(pname)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetFloatv(ALenum pname, ALfloat *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
            case AL_DOPPLER_VELOCITY:
            case AL_DISTANCE_MODEL:
            case AL_SPEED_OF_SOUND:
            case AL_DEFERRED_UPDATES_SOFT:
            case AL_GAIN_LIMIT_SOFT:
                values[0] = alGetFloat(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(pname)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alGetIntegerv(ALenum pname, ALint *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
            case AL_DOPPLER_VELOCITY:
            case AL_DISTANCE_MODEL:
            case AL_SPEED_OF_SOUND:
            case AL_DEFERRED_UPDATES_SOFT:
            case AL_GAIN_LIMIT_SOFT:
                values[0] = alGetInteger(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    switch(pname)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alGetInteger64vSOFT(ALenum pname, ALint64SOFT *values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_DOPPLER_FACTOR:
            case AL_DOPPLER_VELOCITY:
            case AL_DISTANCE_MODEL:
            case AL_SPEED_OF_SOUND:
            case AL_DEFERRED_UPDATES_SOFT:
            case AL_GAIN_LIMIT_SOFT:
                values[0] = alGetInteger64SOFT(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    switch(pname)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}

AL_API const ALchar* AL_APIENTRY alGetString(ALenum pname)
{
    const ALchar *value = NULL;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return NULL;

    switch(pname)
    {
    case AL_VENDOR:
        value = alVendor;
        break;

    case AL_VERSION:
        value = alVersion;
        break;

    case AL_RENDERER:
        value = alRenderer;
        break;

    case AL_EXTENSIONS:
        value = context->ExtensionList;
        break;

    case AL_NO_ERROR:
        value = alNoError;
        break;

    case AL_INVALID_NAME:
        value = alErrInvalidName;
        break;

    case AL_INVALID_ENUM:
        value = alErrInvalidEnum;
        break;

    case AL_INVALID_VALUE:
        value = alErrInvalidValue;
        break;

    case AL_INVALID_OPERATION:
        value = alErrInvalidOp;
        break;

    case AL_OUT_OF_MEMORY:
        value = alErrOutOfMemory;
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);

    return value;
}

AL_API ALvoid AL_APIENTRY alDopplerFactor(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value >= 0.0f && isfinite(value)))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    WriteLock(&context->PropLock);
    context->DopplerFactor = value;
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);
    WriteUnlock(&context->PropLock);

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDopplerVelocity(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value >= 0.0f && isfinite(value)))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    WriteLock(&context->PropLock);
    context->DopplerVelocity = value;
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);
    WriteUnlock(&context->PropLock);

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSpeedOfSound(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value > 0.0f && isfinite(value)))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    WriteLock(&context->PropLock);
    context->SpeedOfSound = value;
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
        UpdateListenerProps(context);
    WriteUnlock(&context->PropLock);

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDistanceModel(ALenum value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value == AL_INVERSE_DISTANCE || value == AL_INVERSE_DISTANCE_CLAMPED ||
         value == AL_LINEAR_DISTANCE || value == AL_LINEAR_DISTANCE_CLAMPED ||
         value == AL_EXPONENT_DISTANCE || value == AL_EXPONENT_DISTANCE_CLAMPED ||
         value == AL_NONE))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    WriteLock(&context->PropLock);
    context->DistanceModel = value;
    if(!context->SourceDistanceModel)
    {
        if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            UpdateListenerProps(context);
    }
    WriteUnlock(&context->PropLock);

done:
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alDeferUpdatesSOFT(void)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ALCcontext_DeferUpdates(context, DeferAll);

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alProcessUpdatesSOFT(void)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ALCcontext_ProcessUpdates(context);

    ALCcontext_DecRef(context);
}
