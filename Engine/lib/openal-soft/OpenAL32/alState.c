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

#include "version.h"

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

/* Resampler strings */
static const ALchar alPointResampler[] = "Nearest";
static const ALchar alLinearResampler[] = "Linear";
static const ALchar alCubicResampler[] = "Cubic";
static const ALchar alBSinc12Resampler[] = "11th order Sinc";
static const ALchar alBSinc24Resampler[] = "23rd order Sinc";

/* WARNING: Non-standard export! Not part of any extension, or exposed in the
 * alcFunctions list.
 */
AL_API const ALchar* AL_APIENTRY alsoft_get_version(void)
{
    const char *spoof = getenv("ALSOFT_SPOOF_VERSION");
    if(spoof && spoof[0] != '\0') return spoof;
    return ALSOFT_VERSION;
}

#define DO_UPDATEPROPS() do {                                                 \
    if(!ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))          \
        UpdateContextProps(context);                                          \
    else                                                                      \
        ATOMIC_FLAG_CLEAR(&context->PropsClean, almemory_order_release);      \
} while(0)


AL_API ALvoid AL_APIENTRY alEnable(ALenum capability)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    almtx_lock(&context->PropLock);
    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        context->SourceDistanceModel = AL_TRUE;
        DO_UPDATEPROPS();
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid enable property 0x%04x", capability);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDisable(ALenum capability)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    almtx_lock(&context->PropLock);
    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        context->SourceDistanceModel = AL_FALSE;
        DO_UPDATEPROPS();
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid disable property 0x%04x", capability);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability)
{
    ALCcontext *context;
    ALboolean value=AL_FALSE;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    almtx_lock(&context->PropLock);
    switch(capability)
    {
    case AL_SOURCE_DISTANCE_MODEL:
        value = context->SourceDistanceModel;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid is enabled property 0x%04x", capability);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum pname)
{
    ALCcontext *context;
    ALboolean value=AL_FALSE;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    almtx_lock(&context->PropLock);
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
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            value = AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        if(GAIN_MIX_MAX/context->GainBoost != 0.0f)
            value = AL_TRUE;
        break;

    case AL_NUM_RESAMPLERS_SOFT:
        /* Always non-0. */
        value = AL_TRUE;
        break;

    case AL_DEFAULT_RESAMPLER_SOFT:
        value = ResamplerDefault ? AL_TRUE : AL_FALSE;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid boolean property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALdouble AL_APIENTRY alGetDouble(ALenum pname)
{
    ALCcontext *context;
    ALdouble value = 0.0;

    context = GetContextRef();
    if(!context) return 0.0;

    almtx_lock(&context->PropLock);
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
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            value = (ALdouble)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALdouble)GAIN_MIX_MAX/context->GainBoost;
        break;

    case AL_NUM_RESAMPLERS_SOFT:
        value = (ALdouble)(ResamplerMax + 1);
        break;

    case AL_DEFAULT_RESAMPLER_SOFT:
        value = (ALdouble)ResamplerDefault;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid double property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALfloat AL_APIENTRY alGetFloat(ALenum pname)
{
    ALCcontext *context;
    ALfloat value = 0.0f;

    context = GetContextRef();
    if(!context) return 0.0f;

    almtx_lock(&context->PropLock);
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
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            value = (ALfloat)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = GAIN_MIX_MAX/context->GainBoost;
        break;

    case AL_NUM_RESAMPLERS_SOFT:
        value = (ALfloat)(ResamplerMax + 1);
        break;

    case AL_DEFAULT_RESAMPLER_SOFT:
        value = (ALfloat)ResamplerDefault;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid float property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALint AL_APIENTRY alGetInteger(ALenum pname)
{
    ALCcontext *context;
    ALint value = 0;

    context = GetContextRef();
    if(!context) return 0;

    almtx_lock(&context->PropLock);
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
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            value = (ALint)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALint)(GAIN_MIX_MAX/context->GainBoost);
        break;

    case AL_NUM_RESAMPLERS_SOFT:
        value = ResamplerMax + 1;
        break;

    case AL_DEFAULT_RESAMPLER_SOFT:
        value = ResamplerDefault;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid integer property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALint64SOFT AL_APIENTRY alGetInteger64SOFT(ALenum pname)
{
    ALCcontext *context;
    ALint64SOFT value = 0;

    context = GetContextRef();
    if(!context) return 0;

    almtx_lock(&context->PropLock);
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
        if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
            value = (ALint64SOFT)AL_TRUE;
        break;

    case AL_GAIN_LIMIT_SOFT:
        value = (ALint64SOFT)(GAIN_MIX_MAX/context->GainBoost);
        break;

    case AL_NUM_RESAMPLERS_SOFT:
        value = (ALint64SOFT)(ResamplerMax + 1);
        break;

    case AL_DEFAULT_RESAMPLER_SOFT:
        value = (ALint64SOFT)ResamplerDefault;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid integer64 property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

    ALCcontext_DecRef(context);
    return value;
}

AL_API void* AL_APIENTRY alGetPointerSOFT(ALenum pname)
{
    ALCcontext *context;
    void *value = NULL;

    context = GetContextRef();
    if(!context) return NULL;

    almtx_lock(&context->PropLock);
    switch(pname)
    {
    case AL_EVENT_CALLBACK_FUNCTION_SOFT:
        value = context->EventCb;
        break;

    case AL_EVENT_CALLBACK_USER_PARAM_SOFT:
        value = context->EventParam;
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid pointer property 0x%04x", pname);
    }
    almtx_unlock(&context->PropLock);

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
            case AL_NUM_RESAMPLERS_SOFT:
            case AL_DEFAULT_RESAMPLER_SOFT:
                values[0] = alGetBoolean(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid boolean-vector property 0x%04x", pname);
    }

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
            case AL_NUM_RESAMPLERS_SOFT:
            case AL_DEFAULT_RESAMPLER_SOFT:
                values[0] = alGetDouble(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid double-vector property 0x%04x", pname);
    }

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
            case AL_NUM_RESAMPLERS_SOFT:
            case AL_DEFAULT_RESAMPLER_SOFT:
                values[0] = alGetFloat(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid float-vector property 0x%04x", pname);
    }

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
            case AL_NUM_RESAMPLERS_SOFT:
            case AL_DEFAULT_RESAMPLER_SOFT:
                values[0] = alGetInteger(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid integer-vector property 0x%04x", pname);
    }

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
            case AL_NUM_RESAMPLERS_SOFT:
            case AL_DEFAULT_RESAMPLER_SOFT:
                values[0] = alGetInteger64SOFT(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid integer64-vector property 0x%04x", pname);
    }

    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alGetPointervSOFT(ALenum pname, void **values)
{
    ALCcontext *context;

    if(values)
    {
        switch(pname)
        {
            case AL_EVENT_CALLBACK_FUNCTION_SOFT:
            case AL_EVENT_CALLBACK_USER_PARAM_SOFT:
                values[0] = alGetPointerSOFT(pname);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    if(!values)
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    switch(pname)
    {
    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid pointer-vector property 0x%04x", pname);
    }

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
        alSetError(context, AL_INVALID_VALUE, "Invalid string property 0x%04x", pname);
    }

    ALCcontext_DecRef(context);
    return value;
}

AL_API ALvoid AL_APIENTRY alDopplerFactor(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value >= 0.0f && isfinite(value)))
        alSetError(context, AL_INVALID_VALUE, "Doppler factor %f out of range", value);
    else
    {
        almtx_lock(&context->PropLock);
        context->DopplerFactor = value;
        DO_UPDATEPROPS();
        almtx_unlock(&context->PropLock);
    }

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDopplerVelocity(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if((ATOMIC_LOAD(&context->EnabledEvts, almemory_order_relaxed)&EventType_Deprecated))
    {
        static const ALCchar msg[] =
            "alDopplerVelocity is deprecated in AL1.1, use alSpeedOfSound";
        const ALsizei msglen = (ALsizei)strlen(msg);
        ALbitfieldSOFT enabledevts;
        almtx_lock(&context->EventCbLock);
        enabledevts = ATOMIC_LOAD(&context->EnabledEvts, almemory_order_relaxed);
        if((enabledevts&EventType_Deprecated) && context->EventCb)
            (*context->EventCb)(AL_EVENT_TYPE_DEPRECATED_SOFT, 0, 0, msglen, msg,
                                context->EventParam);
        almtx_unlock(&context->EventCbLock);
    }

    if(!(value >= 0.0f && isfinite(value)))
        alSetError(context, AL_INVALID_VALUE, "Doppler velocity %f out of range", value);
    else
    {
        almtx_lock(&context->PropLock);
        context->DopplerVelocity = value;
        DO_UPDATEPROPS();
        almtx_unlock(&context->PropLock);
    }

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSpeedOfSound(ALfloat value)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    if(!(value > 0.0f && isfinite(value)))
        alSetError(context, AL_INVALID_VALUE, "Speed of sound %f out of range", value);
    else
    {
        almtx_lock(&context->PropLock);
        context->SpeedOfSound = value;
        DO_UPDATEPROPS();
        almtx_unlock(&context->PropLock);
    }

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
        alSetError(context, AL_INVALID_VALUE, "Distance model 0x%04x out of range", value);
    else
    {
        almtx_lock(&context->PropLock);
        context->DistanceModel = value;
        if(!context->SourceDistanceModel)
            DO_UPDATEPROPS();
        almtx_unlock(&context->PropLock);
    }

    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alDeferUpdatesSOFT(void)
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    ALCcontext_DeferUpdates(context);

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


AL_API const ALchar* AL_APIENTRY alGetStringiSOFT(ALenum pname, ALsizei index)
{
    const char *ResamplerNames[] = {
        alPointResampler, alLinearResampler,
        alCubicResampler, alBSinc12Resampler,
        alBSinc24Resampler,
    };
    const ALchar *value = NULL;
    ALCcontext *context;

    static_assert(COUNTOF(ResamplerNames) == ResamplerMax+1, "Incorrect ResamplerNames list");

    context = GetContextRef();
    if(!context) return NULL;

    switch(pname)
    {
    case AL_RESAMPLER_NAME_SOFT:
        if(index < 0 || (size_t)index >= COUNTOF(ResamplerNames))
            SETERR_GOTO(context, AL_INVALID_VALUE, done, "Resampler name index %d out of range",
                        index);
        value = ResamplerNames[index];
        break;

    default:
        alSetError(context, AL_INVALID_VALUE, "Invalid string indexed property");
    }

done:
    ALCcontext_DecRef(context);
    return value;
}


void UpdateContextProps(ALCcontext *context)
{
    struct ALcontextProps *props;

    /* Get an unused proprty container, or allocate a new one as needed. */
    props = ATOMIC_LOAD(&context->FreeContextProps, almemory_order_acquire);
    if(!props)
        props = al_calloc(16, sizeof(*props));
    else
    {
        struct ALcontextProps *next;
        do {
            next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_PTR_WEAK(&context->FreeContextProps, &props, next,
                almemory_order_seq_cst, almemory_order_acquire) == 0);
    }

    /* Copy in current property values. */
    props->MetersPerUnit = context->MetersPerUnit;

    props->DopplerFactor = context->DopplerFactor;
    props->DopplerVelocity = context->DopplerVelocity;
    props->SpeedOfSound = context->SpeedOfSound;

    props->SourceDistanceModel = context->SourceDistanceModel;
    props->DistanceModel = context->DistanceModel;

    /* Set the new container for updating internal parameters. */
    props = ATOMIC_EXCHANGE_PTR(&context->Update, props, almemory_order_acq_rel);
    if(props)
    {
        /* If there was an unused update container, put it back in the
         * freelist.
         */
        ATOMIC_REPLACE_HEAD(struct ALcontextProps*, &context->FreeContextProps, props);
    }
}
