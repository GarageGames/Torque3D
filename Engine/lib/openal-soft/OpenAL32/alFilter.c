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

#include "alMain.h"
#include "alu.h"
#include "alFilter.h"
#include "alError.h"


extern inline void LockFilterList(ALCdevice *device);
extern inline void UnlockFilterList(ALCdevice *device);

static ALfilter *AllocFilter(ALCcontext *context);
static void FreeFilter(ALCdevice *device, ALfilter *filter);
static void InitFilterParams(ALfilter *filter, ALenum type);

static inline ALfilter *LookupFilter(ALCdevice *device, ALuint id)
{
    FilterSubList *sublist;
    ALuint lidx = (id-1) >> 6;
    ALsizei slidx = (id-1) & 0x3f;

    if(UNLIKELY(lidx >= VECTOR_SIZE(device->FilterList)))
        return NULL;
    sublist = &VECTOR_ELEM(device->FilterList, lidx);
    if(UNLIKELY(sublist->FreeMask & (U64(1)<<slidx)))
        return NULL;
    return sublist->Filters + slidx;
}


AL_API ALvoid AL_APIENTRY alGenFilters(ALsizei n, ALuint *filters)
{
    ALCcontext *context;
    ALsizei cur = 0;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        alSetError(context, AL_INVALID_VALUE, "Generating %d filters", n);
    else for(cur = 0;cur < n;cur++)
    {
        ALfilter *filter = AllocFilter(context);
        if(!filter)
        {
            alDeleteFilters(cur, filters);
            break;
        }

        filters[cur] = filter->id;
    }

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDeleteFilters(ALsizei n, const ALuint *filters)
{
    ALCdevice *device;
    ALCcontext *context;
    ALfilter *filter;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockFilterList(device);
    if(!(n >= 0))
        SETERR_GOTO(context, AL_INVALID_VALUE, done, "Deleting %d filters", n);
    for(i = 0;i < n;i++)
    {
        if(filters[i] && LookupFilter(device, filters[i]) == NULL)
            SETERR_GOTO(context, AL_INVALID_NAME, done, "Invalid filter ID %u", filters[i]);
    }
    for(i = 0;i < n;i++)
    {
        if((filter=LookupFilter(device, filters[i])) != NULL)
            FreeFilter(device, filter);
    }

done:
    UnlockFilterList(device);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsFilter(ALuint filter)
{
    ALCcontext *Context;
    ALboolean  result;

    Context = GetContextRef();
    if(!Context) return AL_FALSE;

    LockFilterList(Context->Device);
    result = ((!filter || LookupFilter(Context->Device, filter)) ?
              AL_TRUE : AL_FALSE);
    UnlockFilterList(Context->Device);

    ALCcontext_DecRef(Context);

    return result;
}

AL_API ALvoid AL_APIENTRY alFilteri(ALuint filter, ALenum param, ALint value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        if(param == AL_FILTER_TYPE)
        {
            if(value == AL_FILTER_NULL || value == AL_FILTER_LOWPASS ||
               value == AL_FILTER_HIGHPASS || value == AL_FILTER_BANDPASS)
                InitFilterParams(ALFilter, value);
            else
                alSetError(Context, AL_INVALID_VALUE, "Invalid filter type 0x%04x", value);
        }
        else
        {
            /* Call the appropriate handler */
            ALfilter_setParami(ALFilter, Context, param, value);
        }
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alFilteriv(ALuint filter, ALenum param, const ALint *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    switch(param)
    {
        case AL_FILTER_TYPE:
            alFilteri(filter, param, values[0]);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_setParamiv(ALFilter, Context, param, values);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alFilterf(ALuint filter, ALenum param, ALfloat value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_setParamf(ALFilter, Context, param, value);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alFilterfv(ALuint filter, ALenum param, const ALfloat *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_setParamfv(ALFilter, Context, param, values);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetFilteri(ALuint filter, ALenum param, ALint *value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        if(param == AL_FILTER_TYPE)
            *value = ALFilter->type;
        else
        {
            /* Call the appropriate handler */
            ALfilter_getParami(ALFilter, Context, param, value);
        }
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetFilteriv(ALuint filter, ALenum param, ALint *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    switch(param)
    {
        case AL_FILTER_TYPE:
            alGetFilteri(filter, param, values);
            return;
    }

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_getParamiv(ALFilter, Context, param, values);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetFilterf(ALuint filter, ALenum param, ALfloat *value)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_getParamf(ALFilter, Context, param, value);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alGetFilterfv(ALuint filter, ALenum param, ALfloat *values)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALfilter   *ALFilter;

    Context = GetContextRef();
    if(!Context) return;

    Device = Context->Device;
    LockFilterList(Device);
    if((ALFilter=LookupFilter(Device, filter)) == NULL)
        alSetError(Context, AL_INVALID_NAME, "Invalid filter ID %u", filter);
    else
    {
        /* Call the appropriate handler */
        ALfilter_getParamfv(ALFilter, Context, param, values);
    }
    UnlockFilterList(Device);

    ALCcontext_DecRef(Context);
}


static void ALlowpass_setParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid low-pass integer property 0x%04x", param); }
static void ALlowpass_setParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid low-pass integer-vector property 0x%04x", param); }
static void ALlowpass_setParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val)
{
    switch(param)
    {
        case AL_LOWPASS_GAIN:
            if(!(val >= AL_LOWPASS_MIN_GAIN && val <= AL_LOWPASS_MAX_GAIN))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Low-pass gain %f out of range", val);
            filter->Gain = val;
            break;

        case AL_LOWPASS_GAINHF:
            if(!(val >= AL_LOWPASS_MIN_GAINHF && val <= AL_LOWPASS_MAX_GAINHF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Low-pass gainhf %f out of range", val);
            filter->GainHF = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid low-pass float property 0x%04x", param);
    }
}
static void ALlowpass_setParamfv(ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALlowpass_setParamf(filter, context, param, vals[0]); }

static void ALlowpass_getParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid low-pass integer property 0x%04x", param); }
static void ALlowpass_getParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid low-pass integer-vector property 0x%04x", param); }
static void ALlowpass_getParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val)
{
    switch(param)
    {
        case AL_LOWPASS_GAIN:
            *val = filter->Gain;
            break;

        case AL_LOWPASS_GAINHF:
            *val = filter->GainHF;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid low-pass float property 0x%04x", param);
    }
}
static void ALlowpass_getParamfv(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALlowpass_getParamf(filter, context, param, vals); }

DEFINE_ALFILTER_VTABLE(ALlowpass);


static void ALhighpass_setParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid high-pass integer property 0x%04x", param); }
static void ALhighpass_setParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid high-pass integer-vector property 0x%04x", param); }
static void ALhighpass_setParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val)
{
    switch(param)
    {
        case AL_HIGHPASS_GAIN:
            if(!(val >= AL_HIGHPASS_MIN_GAIN && val <= AL_HIGHPASS_MAX_GAIN))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "High-pass gain out of range");
            filter->Gain = val;
            break;

        case AL_HIGHPASS_GAINLF:
            if(!(val >= AL_HIGHPASS_MIN_GAINLF && val <= AL_HIGHPASS_MAX_GAINLF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "High-pass gainlf out of range");
            filter->GainLF = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid high-pass float property 0x%04x", param);
    }
}
static void ALhighpass_setParamfv(ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALhighpass_setParamf(filter, context, param, vals[0]); }

static void ALhighpass_getParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid high-pass integer property 0x%04x", param); }
static void ALhighpass_getParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid high-pass integer-vector property 0x%04x", param); }
static void ALhighpass_getParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val)
{
    switch(param)
    {
        case AL_HIGHPASS_GAIN:
            *val = filter->Gain;
            break;

        case AL_HIGHPASS_GAINLF:
            *val = filter->GainLF;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid high-pass float property 0x%04x", param);
    }
}
static void ALhighpass_getParamfv(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALhighpass_getParamf(filter, context, param, vals); }

DEFINE_ALFILTER_VTABLE(ALhighpass);


static void ALbandpass_setParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid band-pass integer property 0x%04x", param); }
static void ALbandpass_setParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid band-pass integer-vector property 0x%04x", param); }
static void ALbandpass_setParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val)
{
    switch(param)
    {
        case AL_BANDPASS_GAIN:
            if(!(val >= AL_BANDPASS_MIN_GAIN && val <= AL_BANDPASS_MAX_GAIN))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Band-pass gain out of range");
            filter->Gain = val;
            break;

        case AL_BANDPASS_GAINHF:
            if(!(val >= AL_BANDPASS_MIN_GAINHF && val <= AL_BANDPASS_MAX_GAINHF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Band-pass gainhf out of range");
            filter->GainHF = val;
            break;

        case AL_BANDPASS_GAINLF:
            if(!(val >= AL_BANDPASS_MIN_GAINLF && val <= AL_BANDPASS_MAX_GAINLF))
                SETERR_RETURN(context, AL_INVALID_VALUE,, "Band-pass gainlf out of range");
            filter->GainLF = val;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid band-pass float property 0x%04x", param);
    }
}
static void ALbandpass_setParamfv(ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals)
{ ALbandpass_setParamf(filter, context, param, vals[0]); }

static void ALbandpass_getParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid band-pass integer property 0x%04x", param); }
static void ALbandpass_getParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid band-pass integer-vector property 0x%04x", param); }
static void ALbandpass_getParamf(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val)
{
    switch(param)
    {
        case AL_BANDPASS_GAIN:
            *val = filter->Gain;
            break;

        case AL_BANDPASS_GAINHF:
            *val = filter->GainHF;
            break;

        case AL_BANDPASS_GAINLF:
            *val = filter->GainLF;
            break;

        default:
            alSetError(context, AL_INVALID_ENUM, "Invalid band-pass float property 0x%04x", param);
    }
}
static void ALbandpass_getParamfv(ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals)
{ ALbandpass_getParamf(filter, context, param, vals); }

DEFINE_ALFILTER_VTABLE(ALbandpass);


static void ALnullfilter_setParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_setParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, const ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_setParamf(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALfloat UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_setParamfv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, const ALfloat *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }

static void ALnullfilter_getParami(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_getParamiv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALint *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_getParamf(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALfloat *UNUSED(val))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }
static void ALnullfilter_getParamfv(ALfilter *UNUSED(filter), ALCcontext *context, ALenum param, ALfloat *UNUSED(vals))
{ alSetError(context, AL_INVALID_ENUM, "Invalid null filter property 0x%04x", param); }

DEFINE_ALFILTER_VTABLE(ALnullfilter);


static ALfilter *AllocFilter(ALCcontext *context)
{
    ALCdevice *device = context->Device;
    FilterSubList *sublist, *subend;
    ALfilter *filter = NULL;
    ALsizei lidx = 0;
    ALsizei slidx;

    almtx_lock(&device->FilterLock);
    sublist = VECTOR_BEGIN(device->FilterList);
    subend = VECTOR_END(device->FilterList);
    for(;sublist != subend;++sublist)
    {
        if(sublist->FreeMask)
        {
            slidx = CTZ64(sublist->FreeMask);
            filter = sublist->Filters + slidx;
            break;
        }
        ++lidx;
    }
    if(UNLIKELY(!filter))
    {
        const FilterSubList empty_sublist = { 0, NULL };
        /* Don't allocate so many list entries that the 32-bit ID could
         * overflow...
         */
        if(UNLIKELY(VECTOR_SIZE(device->FilterList) >= 1<<25))
        {
            almtx_unlock(&device->FilterLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Too many filters allocated");
            return NULL;
        }
        lidx = (ALsizei)VECTOR_SIZE(device->FilterList);
        VECTOR_PUSH_BACK(device->FilterList, empty_sublist);
        sublist = &VECTOR_BACK(device->FilterList);
        sublist->FreeMask = ~U64(0);
        sublist->Filters = al_calloc(16, sizeof(ALfilter)*64);
        if(UNLIKELY(!sublist->Filters))
        {
            VECTOR_POP_BACK(device->FilterList);
            almtx_unlock(&device->FilterLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Failed to allocate filter batch");
            return NULL;
        }

        slidx = 0;
        filter = sublist->Filters + slidx;
    }

    memset(filter, 0, sizeof(*filter));
    InitFilterParams(filter, AL_FILTER_NULL);

    /* Add 1 to avoid filter ID 0. */
    filter->id = ((lidx<<6) | slidx) + 1;

    sublist->FreeMask &= ~(U64(1)<<slidx);
    almtx_unlock(&device->FilterLock);

    return filter;
}

static void FreeFilter(ALCdevice *device, ALfilter *filter)
{
    ALuint id = filter->id - 1;
    ALsizei lidx = id >> 6;
    ALsizei slidx = id & 0x3f;

    memset(filter, 0, sizeof(*filter));

    VECTOR_ELEM(device->FilterList, lidx).FreeMask |= U64(1) << slidx;
}

void ReleaseALFilters(ALCdevice *device)
{
    FilterSubList *sublist = VECTOR_BEGIN(device->FilterList);
    FilterSubList *subend = VECTOR_END(device->FilterList);
    size_t leftover = 0;
    for(;sublist != subend;++sublist)
    {
        ALuint64 usemask = ~sublist->FreeMask;
        while(usemask)
        {
            ALsizei idx = CTZ64(usemask);
            ALfilter *filter = sublist->Filters + idx;

            memset(filter, 0, sizeof(*filter));
            ++leftover;

            usemask &= ~(U64(1) << idx);
        }
        sublist->FreeMask = ~usemask;
    }
    if(leftover > 0)
        WARN("(%p) Deleted "SZFMT" Filter%s\n", device, leftover, (leftover==1)?"":"s");
}


static void InitFilterParams(ALfilter *filter, ALenum type)
{
    if(type == AL_FILTER_LOWPASS)
    {
        filter->Gain = AL_LOWPASS_DEFAULT_GAIN;
        filter->GainHF = AL_LOWPASS_DEFAULT_GAINHF;
        filter->HFReference = LOWPASSFREQREF;
        filter->GainLF = 1.0f;
        filter->LFReference = HIGHPASSFREQREF;
        filter->vtab = &ALlowpass_vtable;
    }
    else if(type == AL_FILTER_HIGHPASS)
    {
        filter->Gain = AL_HIGHPASS_DEFAULT_GAIN;
        filter->GainHF = 1.0f;
        filter->HFReference = LOWPASSFREQREF;
        filter->GainLF = AL_HIGHPASS_DEFAULT_GAINLF;
        filter->LFReference = HIGHPASSFREQREF;
        filter->vtab = &ALhighpass_vtable;
    }
    else if(type == AL_FILTER_BANDPASS)
    {
        filter->Gain = AL_BANDPASS_DEFAULT_GAIN;
        filter->GainHF = AL_BANDPASS_DEFAULT_GAINHF;
        filter->HFReference = LOWPASSFREQREF;
        filter->GainLF = AL_BANDPASS_DEFAULT_GAINLF;
        filter->LFReference = HIGHPASSFREQREF;
        filter->vtab = &ALbandpass_vtable;
    }
    else
    {
        filter->Gain = 1.0f;
        filter->GainHF = 1.0f;
        filter->HFReference = LOWPASSFREQREF;
        filter->GainLF = 1.0f;
        filter->LFReference = HIGHPASSFREQREF;
        filter->vtab = &ALnullfilter_vtable;
    }
    filter->type = type;
}
