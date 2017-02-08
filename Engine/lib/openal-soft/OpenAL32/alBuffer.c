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
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "alMain.h"
#include "alu.h"
#include "alError.h"
#include "alBuffer.h"
#include "alThunk.h"
#include "sample_cvt.h"


extern inline void LockBuffersRead(ALCdevice *device);
extern inline void UnlockBuffersRead(ALCdevice *device);
extern inline void LockBuffersWrite(ALCdevice *device);
extern inline void UnlockBuffersWrite(ALCdevice *device);
extern inline struct ALbuffer *LookupBuffer(ALCdevice *device, ALuint id);
extern inline struct ALbuffer *RemoveBuffer(ALCdevice *device, ALuint id);
extern inline ALuint FrameSizeFromUserFmt(enum UserFmtChannels chans, enum UserFmtType type);
extern inline ALuint FrameSizeFromFmt(enum FmtChannels chans, enum FmtType type);

static ALboolean IsValidType(ALenum type);
static ALboolean IsValidChannels(ALenum channels);
static ALboolean DecomposeUserFormat(ALenum format, enum UserFmtChannels *chans, enum UserFmtType *type);
static ALboolean DecomposeFormat(ALenum format, enum FmtChannels *chans, enum FmtType *type);
static ALboolean SanitizeAlignment(enum UserFmtType type, ALsizei *align);


AL_API ALvoid AL_APIENTRY alGenBuffers(ALsizei n, ALuint *buffers)
{
    ALCcontext *context;
    ALsizei cur = 0;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    for(cur = 0;cur < n;cur++)
    {
        ALbuffer *buffer = NewBuffer(context);
        if(!buffer)
        {
            alDeleteBuffers(cur, buffers);
            break;
        }

        buffers[cur] = buffer->id;
    }

done:
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alDeleteBuffers(ALsizei n, const ALuint *buffers)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *ALBuf;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;

    LockBuffersWrite(device);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    for(i = 0;i < n;i++)
    {
        if(!buffers[i])
            continue;

        /* Check for valid Buffer ID */
        if((ALBuf=LookupBuffer(device, buffers[i])) == NULL)
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
        if(ReadRef(&ALBuf->ref) != 0)
            SET_ERROR_AND_GOTO(context, AL_INVALID_OPERATION, done);
    }

    for(i = 0;i < n;i++)
    {
        if((ALBuf=LookupBuffer(device, buffers[i])) != NULL)
            DeleteBuffer(device, ALBuf);
    }

done:
    UnlockBuffersWrite(device);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsBuffer(ALuint buffer)
{
    ALCcontext *context;
    ALboolean ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    LockBuffersRead(context->Device);
    ret = ((!buffer || LookupBuffer(context->Device, buffer)) ?
           AL_TRUE : AL_FALSE);
    UnlockBuffersRead(context->Device);

    ALCcontext_DecRef(context);

    return ret;
}


AL_API ALvoid AL_APIENTRY alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    enum UserFmtChannels srcchannels = UserFmtMono;
    enum UserFmtType srctype = UserFmtByte;
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;
    ALenum newformat = AL_NONE;
    ALuint framesize;
    ALsizei align;
    ALenum err;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    if(!(size >= 0 && freq > 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if(DecomposeUserFormat(format, &srcchannels, &srctype) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);

    align = ATOMIC_LOAD(&albuf->UnpackAlign);
    if(SanitizeAlignment(srctype, &align) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(srctype)
    {
        case UserFmtByte:
        case UserFmtUByte:
        case UserFmtShort:
        case UserFmtUShort:
        case UserFmtFloat:
            framesize = FrameSizeFromUserFmt(srcchannels, srctype) * align;
            if((size%framesize) != 0)
                SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

            err = LoadData(albuf, freq, format, size/framesize*align,
                           srcchannels, srctype, data, align, AL_TRUE);
            if(err != AL_NO_ERROR)
                SET_ERROR_AND_GOTO(context, err, done);
            break;

        case UserFmtInt:
        case UserFmtUInt:
        case UserFmtByte3:
        case UserFmtUByte3:
        case UserFmtDouble:
            framesize = FrameSizeFromUserFmt(srcchannels, srctype) * align;
            if((size%framesize) != 0)
                SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

            switch(srcchannels)
            {
                case UserFmtMono: newformat = AL_FORMAT_MONO_FLOAT32; break;
                case UserFmtStereo: newformat = AL_FORMAT_STEREO_FLOAT32; break;
                case UserFmtRear: newformat = AL_FORMAT_REAR32; break;
                case UserFmtQuad: newformat = AL_FORMAT_QUAD32; break;
                case UserFmtX51: newformat = AL_FORMAT_51CHN32; break;
                case UserFmtX61: newformat = AL_FORMAT_61CHN32; break;
                case UserFmtX71: newformat = AL_FORMAT_71CHN32; break;
                case UserFmtBFormat2D: newformat = AL_FORMAT_BFORMAT2D_FLOAT32; break;
                case UserFmtBFormat3D: newformat = AL_FORMAT_BFORMAT3D_FLOAT32; break;
            }
            err = LoadData(albuf, freq, newformat, size/framesize*align,
                           srcchannels, srctype, data, align, AL_TRUE);
            if(err != AL_NO_ERROR)
                SET_ERROR_AND_GOTO(context, err, done);
            break;

        case UserFmtMulaw:
        case UserFmtAlaw:
            framesize = FrameSizeFromUserFmt(srcchannels, srctype) * align;
            if((size%framesize) != 0)
                SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

            switch(srcchannels)
            {
                case UserFmtMono: newformat = AL_FORMAT_MONO16; break;
                case UserFmtStereo: newformat = AL_FORMAT_STEREO16; break;
                case UserFmtRear: newformat = AL_FORMAT_REAR16; break;
                case UserFmtQuad: newformat = AL_FORMAT_QUAD16; break;
                case UserFmtX51: newformat = AL_FORMAT_51CHN16; break;
                case UserFmtX61: newformat = AL_FORMAT_61CHN16; break;
                case UserFmtX71: newformat = AL_FORMAT_71CHN16; break;
                case UserFmtBFormat2D: newformat = AL_FORMAT_BFORMAT2D_16; break;
                case UserFmtBFormat3D: newformat = AL_FORMAT_BFORMAT3D_16; break;
            }
            err = LoadData(albuf, freq, newformat, size/framesize*align,
                           srcchannels, srctype, data, align, AL_TRUE);
            if(err != AL_NO_ERROR)
                SET_ERROR_AND_GOTO(context, err, done);
            break;

        case UserFmtIMA4:
            framesize  = (align-1)/2 + 4;
            framesize *= ChannelsFromUserFmt(srcchannels);
            if((size%framesize) != 0)
                SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

            switch(srcchannels)
            {
                case UserFmtMono: newformat = AL_FORMAT_MONO16; break;
                case UserFmtStereo: newformat = AL_FORMAT_STEREO16; break;
                case UserFmtRear: newformat = AL_FORMAT_REAR16; break;
                case UserFmtQuad: newformat = AL_FORMAT_QUAD16; break;
                case UserFmtX51: newformat = AL_FORMAT_51CHN16; break;
                case UserFmtX61: newformat = AL_FORMAT_61CHN16; break;
                case UserFmtX71: newformat = AL_FORMAT_71CHN16; break;
                case UserFmtBFormat2D: newformat = AL_FORMAT_BFORMAT2D_16; break;
                case UserFmtBFormat3D: newformat = AL_FORMAT_BFORMAT3D_16; break;
            }
            err = LoadData(albuf, freq, newformat, size/framesize*align,
                           srcchannels, srctype, data, align, AL_TRUE);
            if(err != AL_NO_ERROR)
                SET_ERROR_AND_GOTO(context, err, done);
            break;

        case UserFmtMSADPCM:
            framesize  = (align-2)/2 + 7;
            framesize *= ChannelsFromUserFmt(srcchannels);
            if((size%framesize) != 0)
                SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

            switch(srcchannels)
            {
                case UserFmtMono: newformat = AL_FORMAT_MONO16; break;
                case UserFmtStereo: newformat = AL_FORMAT_STEREO16; break;
                case UserFmtRear: newformat = AL_FORMAT_REAR16; break;
                case UserFmtQuad: newformat = AL_FORMAT_QUAD16; break;
                case UserFmtX51: newformat = AL_FORMAT_51CHN16; break;
                case UserFmtX61: newformat = AL_FORMAT_61CHN16; break;
                case UserFmtX71: newformat = AL_FORMAT_71CHN16; break;
                case UserFmtBFormat2D: newformat = AL_FORMAT_BFORMAT2D_16; break;
                case UserFmtBFormat3D: newformat = AL_FORMAT_BFORMAT3D_16; break;
            }
            err = LoadData(albuf, freq, newformat, size/framesize*align,
                           srcchannels, srctype, data, align, AL_TRUE);
            if(err != AL_NO_ERROR)
                SET_ERROR_AND_GOTO(context, err, done);
            break;
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alBufferSubDataSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei offset, ALsizei length)
{
    enum UserFmtChannels srcchannels = UserFmtMono;
    enum UserFmtType srctype = UserFmtByte;
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;
    ALuint byte_align;
    ALuint channels;
    ALuint bytes;
    ALsizei align;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    if(!(length >= 0 && offset >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if(DecomposeUserFormat(format, &srcchannels, &srctype) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);

    WriteLock(&albuf->lock);
    align = ATOMIC_LOAD(&albuf->UnpackAlign);
    if(SanitizeAlignment(srctype, &align) == AL_FALSE)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }
    if(srcchannels != albuf->OriginalChannels || srctype != albuf->OriginalType)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(align != albuf->OriginalAlign)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

    if(albuf->OriginalType == UserFmtIMA4)
    {
        byte_align  = (albuf->OriginalAlign-1)/2 + 4;
        byte_align *= ChannelsFromUserFmt(albuf->OriginalChannels);
    }
    else if(albuf->OriginalType == UserFmtMSADPCM)
    {
        byte_align  = (albuf->OriginalAlign-2)/2 + 7;
        byte_align *= ChannelsFromUserFmt(albuf->OriginalChannels);
    }
    else
    {
        byte_align  = albuf->OriginalAlign;
        byte_align *= FrameSizeFromUserFmt(albuf->OriginalChannels,
                                           albuf->OriginalType);
    }

    if(offset > albuf->OriginalSize || length > albuf->OriginalSize-offset ||
       (offset%byte_align) != 0 || (length%byte_align) != 0)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }

    channels = ChannelsFromFmt(albuf->FmtChannels);
    bytes = BytesFromFmt(albuf->FmtType);
    /* offset -> byte offset, length -> sample count */
    offset = offset/byte_align * channels*bytes;
    length = length/byte_align * albuf->OriginalAlign;

    ConvertData((char*)albuf->data+offset, (enum UserFmtType)albuf->FmtType,
                data, srctype, channels, length, align);
    WriteUnlock(&albuf->lock);

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferSamplesSOFT(ALuint buffer,
  ALuint samplerate, ALenum internalformat, ALsizei samples,
  ALenum channels, ALenum type, const ALvoid *data)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;
    ALsizei align;
    ALenum err;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    if(!(samples >= 0 && samplerate != 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if(IsValidType(type) == AL_FALSE || IsValidChannels(channels) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);

    align = ATOMIC_LOAD(&albuf->UnpackAlign);
    if(SanitizeAlignment(type, &align) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if((samples%align) != 0)
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    err = LoadData(albuf, samplerate, internalformat, samples,
                   channels, type, data, align, AL_FALSE);
    if(err != AL_NO_ERROR)
        SET_ERROR_AND_GOTO(context, err, done);

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alBufferSubSamplesSOFT(ALuint buffer,
  ALsizei offset, ALsizei samples,
  ALenum channels, ALenum type, const ALvoid *data)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;
    ALsizei align;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    if(!(samples >= 0 && offset >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if(IsValidType(type) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);

    WriteLock(&albuf->lock);
    align = ATOMIC_LOAD(&albuf->UnpackAlign);
    if(SanitizeAlignment(type, &align) == AL_FALSE)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }
    if(channels != (ALenum)albuf->FmtChannels)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(offset > albuf->SampleLen || samples > albuf->SampleLen-offset)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }
    if((samples%align) != 0)
    {
        WriteUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }

    /* offset -> byte offset */
    offset *= FrameSizeFromFmt(albuf->FmtChannels, albuf->FmtType);
    ConvertData((char*)albuf->data+offset, (enum UserFmtType)albuf->FmtType,
                data, type, ChannelsFromFmt(albuf->FmtChannels), samples, align);
    WriteUnlock(&albuf->lock);

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alGetBufferSamplesSOFT(ALuint buffer,
  ALsizei offset, ALsizei samples,
  ALenum channels, ALenum type, ALvoid *data)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;
    ALsizei align;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    if(!(samples >= 0 && offset >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if(IsValidType(type) == AL_FALSE)
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);

    ReadLock(&albuf->lock);
    align = ATOMIC_LOAD(&albuf->PackAlign);
    if(SanitizeAlignment(type, &align) == AL_FALSE)
    {
        ReadUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }
    if(channels != (ALenum)albuf->FmtChannels)
    {
        ReadUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }
    if(offset > albuf->SampleLen || samples > albuf->SampleLen-offset)
    {
        ReadUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }
    if((samples%align) != 0)
    {
        ReadUnlock(&albuf->lock);
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }

    /* offset -> byte offset */
    offset *= FrameSizeFromFmt(albuf->FmtChannels, albuf->FmtType);
    ConvertData(data, type, (char*)albuf->data+offset, (enum UserFmtType)albuf->FmtType,
                ChannelsFromFmt(albuf->FmtChannels), samples, align);
    ReadUnlock(&albuf->lock);

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsBufferFormatSupportedSOFT(ALenum format)
{
    enum FmtChannels dstchannels;
    enum FmtType dsttype;
    ALCcontext *context;
    ALboolean ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    ret = DecomposeFormat(format, &dstchannels, &dsttype);

    ALCcontext_DecRef(context);

    return ret;
}


AL_API void AL_APIENTRY alBufferf(ALuint buffer, ALenum param, ALfloat UNUSED(value))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBuffer3f(ALuint buffer, ALenum param, ALfloat UNUSED(value1), ALfloat UNUSED(value2), ALfloat UNUSED(value3))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferfv(ALuint buffer, ALenum param, const ALfloat *values)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferi(ALuint buffer, ALenum param, ALint value)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    switch(param)
    {
    case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
        if(!(value >= 0))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        ATOMIC_STORE(&albuf->UnpackAlign, value);
        break;

    case AL_PACK_BLOCK_ALIGNMENT_SOFT:
        if(!(value >= 0))
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        ATOMIC_STORE(&albuf->PackAlign, value);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBuffer3i(ALuint buffer, ALenum param, ALint UNUSED(value1), ALint UNUSED(value2), ALint UNUSED(value3))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferiv(ALuint buffer, ALenum param, const ALint *values)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    if(values)
    {
        switch(param)
        {
            case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
            case AL_PACK_BLOCK_ALIGNMENT_SOFT:
                alBufferi(buffer, param, values[0]);
                return;
        }
    }

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_LOOP_POINTS_SOFT:
        WriteLock(&albuf->lock);
        if(ReadRef(&albuf->ref) != 0)
        {
            WriteUnlock(&albuf->lock);
            SET_ERROR_AND_GOTO(context, AL_INVALID_OPERATION, done);
        }
        if(values[0] >= values[1] || values[0] < 0 ||
           values[1] > albuf->SampleLen)
        {
            WriteUnlock(&albuf->lock);
            SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
        }

        albuf->LoopStart = values[0];
        albuf->LoopEnd = values[1];
        WriteUnlock(&albuf->lock);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetBufferf(ALuint buffer, ALenum param, ALfloat *value)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(value))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_SEC_LENGTH_SOFT:
        ReadLock(&albuf->lock);
        if(albuf->SampleLen != 0)
            *value = albuf->SampleLen / (ALfloat)albuf->Frequency;
        else
            *value = 0.0f;
        ReadUnlock(&albuf->lock);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBuffer3f(ALuint buffer, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(value1 && value2 && value3))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBufferfv(ALuint buffer, ALenum param, ALfloat *values)
{
    ALCdevice *device;
    ALCcontext *context;

    switch(param)
    {
    case AL_SEC_LENGTH_SOFT:
        alGetBufferf(buffer, param, values);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alGetBufferi(ALuint buffer, ALenum param, ALint *value)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(value))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_FREQUENCY:
        *value = albuf->Frequency;
        break;

    case AL_BITS:
        *value = BytesFromFmt(albuf->FmtType) * 8;
        break;

    case AL_CHANNELS:
        *value = ChannelsFromFmt(albuf->FmtChannels);
        break;

    case AL_SIZE:
        ReadLock(&albuf->lock);
        *value = albuf->SampleLen * FrameSizeFromFmt(albuf->FmtChannels,
                                                     albuf->FmtType);
        ReadUnlock(&albuf->lock);
        break;

    case AL_INTERNAL_FORMAT_SOFT:
        *value = albuf->Format;
        break;

    case AL_BYTE_LENGTH_SOFT:
        *value = albuf->OriginalSize;
        break;

    case AL_SAMPLE_LENGTH_SOFT:
        *value = albuf->SampleLen;
        break;

    case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
        *value = ATOMIC_LOAD(&albuf->UnpackAlign);
        break;

    case AL_PACK_BLOCK_ALIGNMENT_SOFT:
        *value = ATOMIC_LOAD(&albuf->PackAlign);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBuffer3i(ALuint buffer, ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if(LookupBuffer(device, buffer) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(value1 && value2 && value3))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBufferiv(ALuint buffer, ALenum param, ALint *values)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer   *albuf;

    switch(param)
    {
    case AL_FREQUENCY:
    case AL_BITS:
    case AL_CHANNELS:
    case AL_SIZE:
    case AL_INTERNAL_FORMAT_SOFT:
    case AL_BYTE_LENGTH_SOFT:
    case AL_SAMPLE_LENGTH_SOFT:
    case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
    case AL_PACK_BLOCK_ALIGNMENT_SOFT:
        alGetBufferi(buffer, param, values);
        return;
    }

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBuffersRead(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    if(!(values))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    switch(param)
    {
    case AL_LOOP_POINTS_SOFT:
        ReadLock(&albuf->lock);
        values[0] = albuf->LoopStart;
        values[1] = albuf->LoopEnd;
        ReadUnlock(&albuf->lock);
        break;

    default:
        SET_ERROR_AND_GOTO(context, AL_INVALID_ENUM, done);
    }

done:
    UnlockBuffersRead(device);
    ALCcontext_DecRef(context);
}


/*
 * LoadData
 *
 * Loads the specified data into the buffer, using the specified formats.
 * Currently, the new format must have the same channel configuration as the
 * original format.
 */
ALenum LoadData(ALbuffer *ALBuf, ALuint freq, ALenum NewFormat, ALsizei frames, enum UserFmtChannels SrcChannels, enum UserFmtType SrcType, const ALvoid *data, ALsizei align, ALboolean storesrc)
{
    enum FmtChannels DstChannels = FmtMono;
    enum FmtType DstType = FmtByte;
    ALuint NewChannels, NewBytes;
    ALuint64 newsize;

    if(DecomposeFormat(NewFormat, &DstChannels, &DstType) == AL_FALSE)
        return AL_INVALID_ENUM;
    if((long)SrcChannels != (long)DstChannels)
        return AL_INVALID_ENUM;

    NewChannels = ChannelsFromFmt(DstChannels);
    NewBytes = BytesFromFmt(DstType);

    newsize = frames;
    newsize *= NewBytes;
    newsize *= NewChannels;
    if(newsize > INT_MAX)
        return AL_OUT_OF_MEMORY;

    WriteLock(&ALBuf->lock);
    if(ReadRef(&ALBuf->ref) != 0)
    {
        WriteUnlock(&ALBuf->lock);
        return AL_INVALID_OPERATION;
    }

    /* Round up to the next 16-byte multiple. This could reallocate only when
     * increasing or the new size is less than half the current, but then the
     * buffer's AL_SIZE would not be very reliable for accounting buffer memory
     * usage, and reporting the real size could cause problems for apps that
     * use AL_SIZE to try to get the buffer's play length.
     */
    newsize = (newsize+15) & ~0xf;
    if(newsize != ALBuf->BytesAlloc)
    {
        void *temp = al_calloc(16, (size_t)newsize);
        if(!temp && newsize)
        {
            WriteUnlock(&ALBuf->lock);
            return AL_OUT_OF_MEMORY;
        }
        al_free(ALBuf->data);
        ALBuf->data = temp;
        ALBuf->BytesAlloc = (ALuint)newsize;
    }

    if(data != NULL)
        ConvertData(ALBuf->data, (enum UserFmtType)DstType, data, SrcType, NewChannels, frames, align);

    if(storesrc)
    {
        ALBuf->OriginalChannels = SrcChannels;
        ALBuf->OriginalType     = SrcType;
        if(SrcType == UserFmtIMA4)
        {
            ALsizei byte_align = ((align-1)/2 + 4) * ChannelsFromUserFmt(SrcChannels);
            ALBuf->OriginalSize  = frames / align * byte_align;
            ALBuf->OriginalAlign = align;
        }
        else if(SrcType == UserFmtMSADPCM)
        {
            ALsizei byte_align = ((align-2)/2 + 7) * ChannelsFromUserFmt(SrcChannels);
            ALBuf->OriginalSize  = frames / align * byte_align;
            ALBuf->OriginalAlign = align;
        }
        else
        {
            ALBuf->OriginalSize  = frames * FrameSizeFromUserFmt(SrcChannels, SrcType);
            ALBuf->OriginalAlign = 1;
        }
    }
    else
    {
        ALBuf->OriginalChannels = (enum UserFmtChannels)DstChannels;
        ALBuf->OriginalType     = (enum UserFmtType)DstType;
        ALBuf->OriginalSize     = frames * NewBytes * NewChannels;
        ALBuf->OriginalAlign    = 1;
    }

    ALBuf->Frequency = freq;
    ALBuf->FmtChannels = DstChannels;
    ALBuf->FmtType = DstType;
    ALBuf->Format = NewFormat;

    ALBuf->SampleLen = frames;
    ALBuf->LoopStart = 0;
    ALBuf->LoopEnd = ALBuf->SampleLen;

    WriteUnlock(&ALBuf->lock);
    return AL_NO_ERROR;
}


ALuint BytesFromUserFmt(enum UserFmtType type)
{
    switch(type)
    {
    case UserFmtByte: return sizeof(ALbyte);
    case UserFmtUByte: return sizeof(ALubyte);
    case UserFmtShort: return sizeof(ALshort);
    case UserFmtUShort: return sizeof(ALushort);
    case UserFmtInt: return sizeof(ALint);
    case UserFmtUInt: return sizeof(ALuint);
    case UserFmtFloat: return sizeof(ALfloat);
    case UserFmtDouble: return sizeof(ALdouble);
    case UserFmtByte3: return sizeof(ALbyte[3]);
    case UserFmtUByte3: return sizeof(ALubyte[3]);
    case UserFmtMulaw: return sizeof(ALubyte);
    case UserFmtAlaw: return sizeof(ALubyte);
    case UserFmtIMA4: break; /* not handled here */
    case UserFmtMSADPCM: break; /* not handled here */
    }
    return 0;
}
ALuint ChannelsFromUserFmt(enum UserFmtChannels chans)
{
    switch(chans)
    {
    case UserFmtMono: return 1;
    case UserFmtStereo: return 2;
    case UserFmtRear: return 2;
    case UserFmtQuad: return 4;
    case UserFmtX51: return 6;
    case UserFmtX61: return 7;
    case UserFmtX71: return 8;
    case UserFmtBFormat2D: return 3;
    case UserFmtBFormat3D: return 4;
    }
    return 0;
}
static ALboolean DecomposeUserFormat(ALenum format, enum UserFmtChannels *chans,
                                     enum UserFmtType *type)
{
    static const struct {
        ALenum format;
        enum UserFmtChannels channels;
        enum UserFmtType type;
    } list[] = {
        { AL_FORMAT_MONO8,             UserFmtMono, UserFmtUByte   },
        { AL_FORMAT_MONO16,            UserFmtMono, UserFmtShort   },
        { AL_FORMAT_MONO_FLOAT32,      UserFmtMono, UserFmtFloat   },
        { AL_FORMAT_MONO_DOUBLE_EXT,   UserFmtMono, UserFmtDouble  },
        { AL_FORMAT_MONO_IMA4,         UserFmtMono, UserFmtIMA4    },
        { AL_FORMAT_MONO_MSADPCM_SOFT, UserFmtMono, UserFmtMSADPCM },
        { AL_FORMAT_MONO_MULAW,        UserFmtMono, UserFmtMulaw   },
        { AL_FORMAT_MONO_ALAW_EXT,     UserFmtMono, UserFmtAlaw    },

        { AL_FORMAT_STEREO8,             UserFmtStereo, UserFmtUByte   },
        { AL_FORMAT_STEREO16,            UserFmtStereo, UserFmtShort   },
        { AL_FORMAT_STEREO_FLOAT32,      UserFmtStereo, UserFmtFloat   },
        { AL_FORMAT_STEREO_DOUBLE_EXT,   UserFmtStereo, UserFmtDouble  },
        { AL_FORMAT_STEREO_IMA4,         UserFmtStereo, UserFmtIMA4    },
        { AL_FORMAT_STEREO_MSADPCM_SOFT, UserFmtStereo, UserFmtMSADPCM },
        { AL_FORMAT_STEREO_MULAW,        UserFmtStereo, UserFmtMulaw   },
        { AL_FORMAT_STEREO_ALAW_EXT,     UserFmtStereo, UserFmtAlaw    },

        { AL_FORMAT_REAR8,      UserFmtRear, UserFmtUByte },
        { AL_FORMAT_REAR16,     UserFmtRear, UserFmtShort },
        { AL_FORMAT_REAR32,     UserFmtRear, UserFmtFloat },
        { AL_FORMAT_REAR_MULAW, UserFmtRear, UserFmtMulaw },

        { AL_FORMAT_QUAD8_LOKI,  UserFmtQuad, UserFmtUByte },
        { AL_FORMAT_QUAD16_LOKI, UserFmtQuad, UserFmtShort },

        { AL_FORMAT_QUAD8,      UserFmtQuad, UserFmtUByte },
        { AL_FORMAT_QUAD16,     UserFmtQuad, UserFmtShort },
        { AL_FORMAT_QUAD32,     UserFmtQuad, UserFmtFloat },
        { AL_FORMAT_QUAD_MULAW, UserFmtQuad, UserFmtMulaw },

        { AL_FORMAT_51CHN8,      UserFmtX51, UserFmtUByte },
        { AL_FORMAT_51CHN16,     UserFmtX51, UserFmtShort },
        { AL_FORMAT_51CHN32,     UserFmtX51, UserFmtFloat },
        { AL_FORMAT_51CHN_MULAW, UserFmtX51, UserFmtMulaw },

        { AL_FORMAT_61CHN8,      UserFmtX61, UserFmtUByte },
        { AL_FORMAT_61CHN16,     UserFmtX61, UserFmtShort },
        { AL_FORMAT_61CHN32,     UserFmtX61, UserFmtFloat },
        { AL_FORMAT_61CHN_MULAW, UserFmtX61, UserFmtMulaw },

        { AL_FORMAT_71CHN8,      UserFmtX71, UserFmtUByte },
        { AL_FORMAT_71CHN16,     UserFmtX71, UserFmtShort },
        { AL_FORMAT_71CHN32,     UserFmtX71, UserFmtFloat },
        { AL_FORMAT_71CHN_MULAW, UserFmtX71, UserFmtMulaw },

        { AL_FORMAT_BFORMAT2D_8,       UserFmtBFormat2D, UserFmtUByte },
        { AL_FORMAT_BFORMAT2D_16,      UserFmtBFormat2D, UserFmtShort },
        { AL_FORMAT_BFORMAT2D_FLOAT32, UserFmtBFormat2D, UserFmtFloat },
        { AL_FORMAT_BFORMAT2D_MULAW,   UserFmtBFormat2D, UserFmtMulaw },

        { AL_FORMAT_BFORMAT3D_8,       UserFmtBFormat3D, UserFmtUByte },
        { AL_FORMAT_BFORMAT3D_16,      UserFmtBFormat3D, UserFmtShort },
        { AL_FORMAT_BFORMAT3D_FLOAT32, UserFmtBFormat3D, UserFmtFloat },
        { AL_FORMAT_BFORMAT3D_MULAW,   UserFmtBFormat3D, UserFmtMulaw },
    };
    ALuint i;

    for(i = 0;i < COUNTOF(list);i++)
    {
        if(list[i].format == format)
        {
            *chans = list[i].channels;
            *type  = list[i].type;
            return AL_TRUE;
        }
    }

    return AL_FALSE;
}

ALuint BytesFromFmt(enum FmtType type)
{
    switch(type)
    {
    case FmtByte: return sizeof(ALbyte);
    case FmtShort: return sizeof(ALshort);
    case FmtFloat: return sizeof(ALfloat);
    }
    return 0;
}
ALuint ChannelsFromFmt(enum FmtChannels chans)
{
    switch(chans)
    {
    case FmtMono: return 1;
    case FmtStereo: return 2;
    case FmtRear: return 2;
    case FmtQuad: return 4;
    case FmtX51: return 6;
    case FmtX61: return 7;
    case FmtX71: return 8;
    case FmtBFormat2D: return 3;
    case FmtBFormat3D: return 4;
    }
    return 0;
}
static ALboolean DecomposeFormat(ALenum format, enum FmtChannels *chans, enum FmtType *type)
{
    static const struct {
        ALenum format;
        enum FmtChannels channels;
        enum FmtType type;
    } list[] = {
        { AL_MONO8_SOFT,   FmtMono, FmtByte  },
        { AL_MONO16_SOFT,  FmtMono, FmtShort },
        { AL_MONO32F_SOFT, FmtMono, FmtFloat },

        { AL_STEREO8_SOFT,   FmtStereo, FmtByte  },
        { AL_STEREO16_SOFT,  FmtStereo, FmtShort },
        { AL_STEREO32F_SOFT, FmtStereo, FmtFloat },

        { AL_REAR8_SOFT,   FmtRear, FmtByte  },
        { AL_REAR16_SOFT,  FmtRear, FmtShort },
        { AL_REAR32F_SOFT, FmtRear, FmtFloat },

        { AL_FORMAT_QUAD8_LOKI,  FmtQuad, FmtByte  },
        { AL_FORMAT_QUAD16_LOKI, FmtQuad, FmtShort },

        { AL_QUAD8_SOFT,   FmtQuad, FmtByte  },
        { AL_QUAD16_SOFT,  FmtQuad, FmtShort },
        { AL_QUAD32F_SOFT, FmtQuad, FmtFloat },

        { AL_5POINT1_8_SOFT,   FmtX51, FmtByte  },
        { AL_5POINT1_16_SOFT,  FmtX51, FmtShort },
        { AL_5POINT1_32F_SOFT, FmtX51, FmtFloat },

        { AL_6POINT1_8_SOFT,   FmtX61, FmtByte  },
        { AL_6POINT1_16_SOFT,  FmtX61, FmtShort },
        { AL_6POINT1_32F_SOFT, FmtX61, FmtFloat },

        { AL_7POINT1_8_SOFT,   FmtX71, FmtByte  },
        { AL_7POINT1_16_SOFT,  FmtX71, FmtShort },
        { AL_7POINT1_32F_SOFT, FmtX71, FmtFloat },

        { AL_BFORMAT2D_8_SOFT,   FmtBFormat2D, FmtByte },
        { AL_BFORMAT2D_16_SOFT,  FmtBFormat2D, FmtShort },
        { AL_BFORMAT2D_32F_SOFT, FmtBFormat2D, FmtFloat },

        { AL_BFORMAT3D_8_SOFT,   FmtBFormat3D, FmtByte },
        { AL_BFORMAT3D_16_SOFT,  FmtBFormat3D, FmtShort },
        { AL_BFORMAT3D_32F_SOFT, FmtBFormat3D, FmtFloat },
    };
    ALuint i;

    for(i = 0;i < COUNTOF(list);i++)
    {
        if(list[i].format == format)
        {
            *chans = list[i].channels;
            *type  = list[i].type;
            return AL_TRUE;
        }
    }

    return AL_FALSE;
}

static ALboolean SanitizeAlignment(enum UserFmtType type, ALsizei *align)
{
    if(*align < 0)
        return AL_FALSE;

    if(*align == 0)
    {
        if(type == UserFmtIMA4)
        {
            /* Here is where things vary:
             * nVidia and Apple use 64+1 sample frames per block -> block_size=36 bytes per channel
             * Most PC sound software uses 2040+1 sample frames per block -> block_size=1024 bytes per channel
             */
            *align = 65;
        }
        else if(type == UserFmtMSADPCM)
            *align = 64;
        else
            *align = 1;
        return AL_TRUE;
    }

    if(type == UserFmtIMA4)
    {
        /* IMA4 block alignment must be a multiple of 8, plus 1. */
        return ((*align)&7) == 1;
    }
    if(type == UserFmtMSADPCM)
    {
        /* MSADPCM block alignment must be a multiple of 2. */
        /* FIXME: Too strict? Might only require align*channels to be a
         * multiple of 2. */
        return ((*align)&1) == 0;
    }

    return AL_TRUE;
}


static ALboolean IsValidType(ALenum type)
{
    switch(type)
    {
        case AL_BYTE_SOFT:
        case AL_UNSIGNED_BYTE_SOFT:
        case AL_SHORT_SOFT:
        case AL_UNSIGNED_SHORT_SOFT:
        case AL_INT_SOFT:
        case AL_UNSIGNED_INT_SOFT:
        case AL_FLOAT_SOFT:
        case AL_DOUBLE_SOFT:
        case AL_BYTE3_SOFT:
        case AL_UNSIGNED_BYTE3_SOFT:
        case AL_MULAW_SOFT:
            return AL_TRUE;
    }
    return AL_FALSE;
}

static ALboolean IsValidChannels(ALenum channels)
{
    switch(channels)
    {
        case AL_MONO_SOFT:
        case AL_STEREO_SOFT:
        case AL_REAR_SOFT:
        case AL_QUAD_SOFT:
        case AL_5POINT1_SOFT:
        case AL_6POINT1_SOFT:
        case AL_7POINT1_SOFT:
        case AL_BFORMAT2D_SOFT:
        case AL_BFORMAT3D_SOFT:
            return AL_TRUE;
    }
    return AL_FALSE;
}


ALbuffer *NewBuffer(ALCcontext *context)
{
    ALCdevice *device = context->Device;
    ALbuffer *buffer;
    ALenum err;

    buffer = al_calloc(16, sizeof(ALbuffer));
    if(!buffer)
        SET_ERROR_AND_RETURN_VALUE(context, AL_OUT_OF_MEMORY, NULL);
    RWLockInit(&buffer->lock);

    err = NewThunkEntry(&buffer->id);
    if(err == AL_NO_ERROR)
        err = InsertUIntMapEntry(&device->BufferMap, buffer->id, buffer);
    if(err != AL_NO_ERROR)
    {
        FreeThunkEntry(buffer->id);
        memset(buffer, 0, sizeof(ALbuffer));
        al_free(buffer);

        SET_ERROR_AND_RETURN_VALUE(context, err, NULL);
    }

    return buffer;
}

void DeleteBuffer(ALCdevice *device, ALbuffer *buffer)
{
    RemoveBuffer(device, buffer->id);
    FreeThunkEntry(buffer->id);

    al_free(buffer->data);

    memset(buffer, 0, sizeof(*buffer));
    al_free(buffer);
}


/*
 *    ReleaseALBuffers()
 *
 *    INTERNAL: Called to destroy any buffers that still exist on the device
 */
ALvoid ReleaseALBuffers(ALCdevice *device)
{
    ALsizei i;
    for(i = 0;i < device->BufferMap.size;i++)
    {
        ALbuffer *temp = device->BufferMap.values[i];
        device->BufferMap.values[i] = NULL;

        al_free(temp->data);

        FreeThunkEntry(temp->id);
        memset(temp, 0, sizeof(ALbuffer));
        al_free(temp);
    }
}
