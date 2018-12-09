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
#include "sample_cvt.h"


extern inline void LockBufferList(ALCdevice *device);
extern inline void UnlockBufferList(ALCdevice *device);
extern inline ALsizei FrameSizeFromUserFmt(enum UserFmtChannels chans, enum UserFmtType type);
extern inline ALsizei FrameSizeFromFmt(enum FmtChannels chans, enum FmtType type);

static ALbuffer *AllocBuffer(ALCcontext *context);
static void FreeBuffer(ALCdevice *device, ALbuffer *buffer);
static const ALchar *NameFromUserFmtType(enum UserFmtType type);
static void LoadData(ALCcontext *context, ALbuffer *buffer, ALuint freq, ALsizei size,
                     enum UserFmtChannels SrcChannels, enum UserFmtType SrcType,
                     const ALvoid *data, ALbitfieldSOFT access);
static ALboolean DecomposeUserFormat(ALenum format, enum UserFmtChannels *chans, enum UserFmtType *type);
static ALsizei SanitizeAlignment(enum UserFmtType type, ALsizei align);

static inline ALbuffer *LookupBuffer(ALCdevice *device, ALuint id)
{
    BufferSubList *sublist;
    ALuint lidx = (id-1) >> 6;
    ALsizei slidx = (id-1) & 0x3f;

    if(UNLIKELY(lidx >= VECTOR_SIZE(device->BufferList)))
        return NULL;
    sublist = &VECTOR_ELEM(device->BufferList, lidx);
    if(UNLIKELY(sublist->FreeMask & (U64(1)<<slidx)))
        return NULL;
    return sublist->Buffers + slidx;
}


#define INVALID_STORAGE_MASK ~(AL_MAP_READ_BIT_SOFT | AL_MAP_WRITE_BIT_SOFT | AL_PRESERVE_DATA_BIT_SOFT | AL_MAP_PERSISTENT_BIT_SOFT)
#define MAP_READ_WRITE_FLAGS (AL_MAP_READ_BIT_SOFT | AL_MAP_WRITE_BIT_SOFT)
#define INVALID_MAP_FLAGS ~(AL_MAP_READ_BIT_SOFT | AL_MAP_WRITE_BIT_SOFT | AL_MAP_PERSISTENT_BIT_SOFT)


AL_API ALvoid AL_APIENTRY alGenBuffers(ALsizei n, ALuint *buffers)
{
    ALCcontext *context;
    ALsizei cur = 0;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        alSetError(context, AL_INVALID_VALUE, "Generating %d buffers", n);
    else for(cur = 0;cur < n;cur++)
    {
        ALbuffer *buffer = AllocBuffer(context);
        if(!buffer)
        {
            alDeleteBuffers(cur, buffers);
            break;
        }

        buffers[cur] = buffer->id;
    }

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

    LockBufferList(device);
    if(UNLIKELY(n < 0))
    {
        alSetError(context, AL_INVALID_VALUE, "Deleting %d buffers", n);
        goto done;
    }

    for(i = 0;i < n;i++)
    {
        if(!buffers[i])
            continue;

        /* Check for valid Buffer ID, and make sure it's not in use. */
        if((ALBuf=LookupBuffer(device, buffers[i])) == NULL)
        {
            alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffers[i]);
            goto done;
        }
        if(ReadRef(&ALBuf->ref) != 0)
        {
            alSetError(context, AL_INVALID_OPERATION, "Deleting in-use buffer %u", buffers[i]);
            goto done;
        }
    }
    for(i = 0;i < n;i++)
    {
        if((ALBuf=LookupBuffer(device, buffers[i])) != NULL)
            FreeBuffer(device, ALBuf);
    }

done:
    UnlockBufferList(device);
    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsBuffer(ALuint buffer)
{
    ALCcontext *context;
    ALboolean ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    LockBufferList(context->Device);
    ret = ((!buffer || LookupBuffer(context->Device, buffer)) ?
           AL_TRUE : AL_FALSE);
    UnlockBufferList(context->Device);

    ALCcontext_DecRef(context);

    return ret;
}


AL_API ALvoid AL_APIENTRY alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{ alBufferStorageSOFT(buffer, format, data, size, freq, 0); }

AL_API void AL_APIENTRY alBufferStorageSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq, ALbitfieldSOFT flags)
{
    enum UserFmtChannels srcchannels = UserFmtMono;
    enum UserFmtType srctype = UserFmtUByte;
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(size < 0))
        alSetError(context, AL_INVALID_VALUE, "Negative storage size %d", size);
    else if(UNLIKELY(freq < 1))
        alSetError(context, AL_INVALID_VALUE, "Invalid sample rate %d", freq);
    else if(UNLIKELY((flags&INVALID_STORAGE_MASK) != 0))
        alSetError(context, AL_INVALID_VALUE, "Invalid storage flags 0x%x",
                   flags&INVALID_STORAGE_MASK);
    else if(UNLIKELY((flags&AL_MAP_PERSISTENT_BIT_SOFT) && !(flags&MAP_READ_WRITE_FLAGS)))
        alSetError(context, AL_INVALID_VALUE,
                   "Declaring persistently mapped storage without read or write access");
    else if(UNLIKELY(DecomposeUserFormat(format, &srcchannels, &srctype) == AL_FALSE))
        alSetError(context, AL_INVALID_ENUM, "Invalid format 0x%04x", format);
    else
        LoadData(context, albuf, freq, size, srcchannels, srctype, data, flags);

    UnlockBufferList(device);
    ALCcontext_DecRef(context);
}

AL_API void* AL_APIENTRY alMapBufferSOFT(ALuint buffer, ALsizei offset, ALsizei length, ALbitfieldSOFT access)
{
    void *retval = NULL;
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return retval;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY((access&INVALID_MAP_FLAGS) != 0))
        alSetError(context, AL_INVALID_VALUE, "Invalid map flags 0x%x", access&INVALID_MAP_FLAGS);
    else if(UNLIKELY(!(access&MAP_READ_WRITE_FLAGS)))
        alSetError(context, AL_INVALID_VALUE, "Mapping buffer %u without read or write access",
                   buffer);
    else
    {
        ALbitfieldSOFT unavailable = (albuf->Access^access) & access;
        if(UNLIKELY(ReadRef(&albuf->ref) != 0 && !(access&AL_MAP_PERSISTENT_BIT_SOFT)))
            alSetError(context, AL_INVALID_OPERATION,
                       "Mapping in-use buffer %u without persistent mapping", buffer);
        else if(UNLIKELY(albuf->MappedAccess != 0))
            alSetError(context, AL_INVALID_OPERATION, "Mapping already-mapped buffer %u", buffer);
        else if(UNLIKELY((unavailable&AL_MAP_READ_BIT_SOFT)))
            alSetError(context, AL_INVALID_VALUE,
                       "Mapping buffer %u for reading without read access", buffer);
        else if(UNLIKELY((unavailable&AL_MAP_WRITE_BIT_SOFT)))
            alSetError(context, AL_INVALID_VALUE,
                       "Mapping buffer %u for writing without write access", buffer);
        else if(UNLIKELY((unavailable&AL_MAP_PERSISTENT_BIT_SOFT)))
            alSetError(context, AL_INVALID_VALUE,
                       "Mapping buffer %u persistently without persistent access", buffer);
        else if(UNLIKELY(offset < 0 || offset >= albuf->OriginalSize ||
                         length <= 0 || length > albuf->OriginalSize - offset))
            alSetError(context, AL_INVALID_VALUE, "Mapping invalid range %d+%d for buffer %u",
                       offset, length, buffer);
        else
        {
            retval = (ALbyte*)albuf->data + offset;
            albuf->MappedAccess = access;
            albuf->MappedOffset = offset;
            albuf->MappedSize = length;
        }
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
    return retval;
}

AL_API void AL_APIENTRY alUnmapBufferSOFT(ALuint buffer)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if((albuf=LookupBuffer(device, buffer)) == NULL)
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(albuf->MappedAccess == 0)
        alSetError(context, AL_INVALID_OPERATION, "Unmapping unmapped buffer %u", buffer);
    else
    {
        albuf->MappedAccess = 0;
        albuf->MappedOffset = 0;
        albuf->MappedSize = 0;
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alFlushMappedBufferSOFT(ALuint buffer, ALsizei offset, ALsizei length)
{
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!(albuf->MappedAccess&AL_MAP_WRITE_BIT_SOFT)))
        alSetError(context, AL_INVALID_OPERATION,
                   "Flushing buffer %u while not mapped for writing", buffer);
    else if(UNLIKELY(offset < albuf->MappedOffset ||
                     offset >= albuf->MappedOffset+albuf->MappedSize ||
                     length <= 0 || length > albuf->MappedOffset+albuf->MappedSize-offset))
        alSetError(context, AL_INVALID_VALUE, "Flushing invalid range %d+%d on buffer %u",
                   offset, length, buffer);
    else
    {
        /* FIXME: Need to use some method of double-buffering for the mixer and
         * app to hold separate memory, which can be safely transfered
         * asynchronously. Currently we just say the app shouldn't write where
         * OpenAL's reading, and hope for the best...
         */
        ATOMIC_THREAD_FENCE(almemory_order_seq_cst);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alBufferSubDataSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei offset, ALsizei length)
{
    enum UserFmtChannels srcchannels = UserFmtMono;
    enum UserFmtType srctype = UserFmtUByte;
    ALCdevice *device;
    ALCcontext *context;
    ALbuffer *albuf;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(DecomposeUserFormat(format, &srcchannels, &srctype) == AL_FALSE))
        alSetError(context, AL_INVALID_ENUM, "Invalid format 0x%04x", format);
    else
    {
        ALsizei unpack_align, align;
        ALsizei byte_align;
        ALsizei frame_size;
        ALsizei num_chans;
        void *dst;

        unpack_align = ATOMIC_LOAD_SEQ(&albuf->UnpackAlign);
        align = SanitizeAlignment(srctype, unpack_align);
        if(UNLIKELY(align < 1))
            alSetError(context, AL_INVALID_VALUE, "Invalid unpack alignment %d", unpack_align);
        else if(UNLIKELY((long)srcchannels != (long)albuf->FmtChannels ||
                         srctype != albuf->OriginalType))
            alSetError(context, AL_INVALID_ENUM, "Unpacking data with mismatched format");
        else if(UNLIKELY(align != albuf->OriginalAlign))
            alSetError(context, AL_INVALID_VALUE,
                       "Unpacking data with alignment %u does not match original alignment %u",
                       align, albuf->OriginalAlign);
        else if(UNLIKELY(albuf->MappedAccess != 0))
            alSetError(context, AL_INVALID_OPERATION, "Unpacking data into mapped buffer %u",
                       buffer);
        else
        {
            num_chans = ChannelsFromFmt(albuf->FmtChannels);
            frame_size = num_chans * BytesFromFmt(albuf->FmtType);
            if(albuf->OriginalType == UserFmtIMA4)
                byte_align = ((align-1)/2 + 4) * num_chans;
            else if(albuf->OriginalType == UserFmtMSADPCM)
                byte_align = ((align-2)/2 + 7) * num_chans;
            else
                byte_align = align * frame_size;

            if(UNLIKELY(offset < 0 || length < 0 || offset > albuf->OriginalSize ||
                        length > albuf->OriginalSize-offset))
                alSetError(context, AL_INVALID_VALUE, "Invalid data sub-range %d+%d on buffer %u",
                            offset, length, buffer);
            else if(UNLIKELY((offset%byte_align) != 0))
                alSetError(context, AL_INVALID_VALUE,
                    "Sub-range offset %d is not a multiple of frame size %d (%d unpack alignment)",
                    offset, byte_align, align);
            else if(UNLIKELY((length%byte_align) != 0))
                alSetError(context, AL_INVALID_VALUE,
                    "Sub-range length %d is not a multiple of frame size %d (%d unpack alignment)",
                    length, byte_align, align);
            else
            {
                /* offset -> byte offset, length -> sample count */
                offset = offset/byte_align * align * frame_size;
                length = length/byte_align * align;

                dst = (ALbyte*)albuf->data + offset;
                if(srctype == UserFmtIMA4 && albuf->FmtType == FmtShort)
                    Convert_ALshort_ALima4(dst, data, num_chans, length, align);
                else if(srctype == UserFmtMSADPCM && albuf->FmtType == FmtShort)
                    Convert_ALshort_ALmsadpcm(dst, data, num_chans, length, align);
                else
                {
                    assert((long)srctype == (long)albuf->FmtType);
                    memcpy(dst, data, length*frame_size);
                }
            }
        }
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferSamplesSOFT(ALuint UNUSED(buffer),
  ALuint UNUSED(samplerate), ALenum UNUSED(internalformat), ALsizei UNUSED(samples),
  ALenum UNUSED(channels), ALenum UNUSED(type), const ALvoid *UNUSED(data))
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    alSetError(context, AL_INVALID_OPERATION, "alBufferSamplesSOFT not supported");

    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alBufferSubSamplesSOFT(ALuint UNUSED(buffer),
  ALsizei UNUSED(offset), ALsizei UNUSED(samples),
  ALenum UNUSED(channels), ALenum UNUSED(type), const ALvoid *UNUSED(data))
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    alSetError(context, AL_INVALID_OPERATION, "alBufferSubSamplesSOFT not supported");

    ALCcontext_DecRef(context);
}

AL_API void AL_APIENTRY alGetBufferSamplesSOFT(ALuint UNUSED(buffer),
  ALsizei UNUSED(offset), ALsizei UNUSED(samples),
  ALenum UNUSED(channels), ALenum UNUSED(type), ALvoid *UNUSED(data))
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    alSetError(context, AL_INVALID_OPERATION, "alGetBufferSamplesSOFT not supported");

    ALCcontext_DecRef(context);
}

AL_API ALboolean AL_APIENTRY alIsBufferFormatSupportedSOFT(ALenum UNUSED(format))
{
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    alSetError(context, AL_INVALID_OPERATION, "alIsBufferFormatSupportedSOFT not supported");

    ALCcontext_DecRef(context);
    return AL_FALSE;
}


AL_API void AL_APIENTRY alBufferf(ALuint buffer, ALenum param, ALfloat UNUSED(value))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer float property 0x%04x", param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBuffer3f(ALuint buffer, ALenum param, ALfloat UNUSED(value1), ALfloat UNUSED(value2), ALfloat UNUSED(value3))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer 3-float property 0x%04x", param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBufferfv(ALuint buffer, ALenum param, const ALfloat *values)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!values))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer float-vector property 0x%04x", param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else switch(param)
    {
    case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
        if(UNLIKELY(value < 0))
            alSetError(context, AL_INVALID_VALUE, "Invalid unpack block alignment %d", value);
        else
            ATOMIC_STORE_SEQ(&albuf->UnpackAlign, value);
        break;

    case AL_PACK_BLOCK_ALIGNMENT_SOFT:
        if(UNLIKELY(value < 0))
            alSetError(context, AL_INVALID_VALUE, "Invalid pack block alignment %d", value);
        else
            ATOMIC_STORE_SEQ(&albuf->PackAlign, value);
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer integer property 0x%04x", param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alBuffer3i(ALuint buffer, ALenum param, ALint UNUSED(value1), ALint UNUSED(value2), ALint UNUSED(value3))
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer 3-integer property 0x%04x", param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!values))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    case AL_LOOP_POINTS_SOFT:
        if(UNLIKELY(ReadRef(&albuf->ref) != 0))
            alSetError(context, AL_INVALID_OPERATION, "Modifying in-use buffer %u's loop points",
                       buffer);
        else if(UNLIKELY(values[0] >= values[1] || values[0] < 0 || values[1] > albuf->SampleLen))
            alSetError(context, AL_INVALID_VALUE, "Invalid loop point range %d -> %d o buffer %u",
                       values[0], values[1], buffer);
        else
        {
            albuf->LoopStart = values[0];
            albuf->LoopEnd = values[1];
        }
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer integer-vector property 0x%04x",
                   param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!value))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer float property 0x%04x", param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBuffer3f(ALuint buffer, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!value1 || !value2 || !value3))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer 3-float property 0x%04x", param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!values))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer float-vector property 0x%04x", param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!value))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
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
        *value = albuf->SampleLen * FrameSizeFromFmt(albuf->FmtChannels,
                                                     albuf->FmtType);
        break;

    case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
        *value = ATOMIC_LOAD_SEQ(&albuf->UnpackAlign);
        break;

    case AL_PACK_BLOCK_ALIGNMENT_SOFT:
        *value = ATOMIC_LOAD_SEQ(&albuf->PackAlign);
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer integer property 0x%04x", param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


AL_API void AL_APIENTRY alGetBuffer3i(ALuint buffer, ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALCdevice *device;
    ALCcontext *context;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;
    LockBufferList(device);
    if(UNLIKELY(LookupBuffer(device, buffer) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!value1 || !value2 || !value3))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer 3-integer property 0x%04x", param);
    }
    UnlockBufferList(device);

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
    LockBufferList(device);
    if(UNLIKELY((albuf=LookupBuffer(device, buffer)) == NULL))
        alSetError(context, AL_INVALID_NAME, "Invalid buffer ID %u", buffer);
    else if(UNLIKELY(!values))
        alSetError(context, AL_INVALID_VALUE, "NULL pointer");
    else switch(param)
    {
    case AL_LOOP_POINTS_SOFT:
        values[0] = albuf->LoopStart;
        values[1] = albuf->LoopEnd;
        break;

    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid buffer integer-vector property 0x%04x",
                   param);
    }
    UnlockBufferList(device);

    ALCcontext_DecRef(context);
}


static const ALchar *NameFromUserFmtType(enum UserFmtType type)
{
    switch(type)
    {
        case UserFmtUByte: return "Unsigned Byte";
        case UserFmtShort: return "Signed Short";
        case UserFmtFloat: return "Float32";
        case UserFmtDouble: return "Float64";
        case UserFmtMulaw: return "muLaw";
        case UserFmtAlaw: return "aLaw";
        case UserFmtIMA4: return "IMA4 ADPCM";
        case UserFmtMSADPCM: return "MSADPCM";
    }
    return "<internal type error>";
}

/*
 * LoadData
 *
 * Loads the specified data into the buffer, using the specified format.
 */
static void LoadData(ALCcontext *context, ALbuffer *ALBuf, ALuint freq, ALsizei size, enum UserFmtChannels SrcChannels, enum UserFmtType SrcType, const ALvoid *data, ALbitfieldSOFT access)
{
    enum FmtChannels DstChannels = FmtMono;
    enum FmtType DstType = FmtUByte;
    ALsizei NumChannels, FrameSize;
    ALsizei SrcByteAlign;
    ALsizei unpackalign;
    ALsizei newsize;
    ALsizei frames;
    ALsizei align;

    if(UNLIKELY(ReadRef(&ALBuf->ref) != 0 || ALBuf->MappedAccess != 0))
        SETERR_RETURN(context, AL_INVALID_OPERATION,, "Modifying storage for in-use buffer %u",
                      ALBuf->id);

    /* Currently no channel configurations need to be converted. */
    switch(SrcChannels)
    {
        case UserFmtMono: DstChannels = FmtMono; break;
        case UserFmtStereo: DstChannels = FmtStereo; break;
        case UserFmtRear: DstChannels = FmtRear; break;
        case UserFmtQuad: DstChannels = FmtQuad; break;
        case UserFmtX51: DstChannels = FmtX51; break;
        case UserFmtX61: DstChannels = FmtX61; break;
        case UserFmtX71: DstChannels = FmtX71; break;
        case UserFmtBFormat2D: DstChannels = FmtBFormat2D; break;
        case UserFmtBFormat3D: DstChannels = FmtBFormat3D; break;
    }
    if(UNLIKELY((long)SrcChannels != (long)DstChannels))
        SETERR_RETURN(context, AL_INVALID_ENUM,, "Invalid format");

    /* IMA4 and MSADPCM convert to 16-bit short. */
    switch(SrcType)
    {
        case UserFmtUByte: DstType = FmtUByte; break;
        case UserFmtShort: DstType = FmtShort; break;
        case UserFmtFloat: DstType = FmtFloat; break;
        case UserFmtDouble: DstType = FmtDouble; break;
        case UserFmtAlaw: DstType = FmtAlaw; break;
        case UserFmtMulaw: DstType = FmtMulaw; break;
        case UserFmtIMA4: DstType = FmtShort; break;
        case UserFmtMSADPCM: DstType = FmtShort; break;
    }

    /* TODO: Currently we can only map samples when they're not converted. To
     * allow it would need some kind of double-buffering to hold onto a copy of
     * the original data.
     */
    if((access&MAP_READ_WRITE_FLAGS))
    {
        if(UNLIKELY((long)SrcType != (long)DstType))
            SETERR_RETURN(context, AL_INVALID_VALUE,, "%s samples cannot be mapped",
                          NameFromUserFmtType(SrcType));
    }

    unpackalign = ATOMIC_LOAD_SEQ(&ALBuf->UnpackAlign);
    if(UNLIKELY((align=SanitizeAlignment(SrcType, unpackalign)) < 1))
        SETERR_RETURN(context, AL_INVALID_VALUE,, "Invalid unpack alignment %d for %s samples",
                      unpackalign, NameFromUserFmtType(SrcType));

    if((access&AL_PRESERVE_DATA_BIT_SOFT))
    {
        /* Can only preserve data with the same format and alignment. */
        if(UNLIKELY(ALBuf->FmtChannels != DstChannels || ALBuf->OriginalType != SrcType))
            SETERR_RETURN(context, AL_INVALID_VALUE,, "Preserving data of mismatched format");
        if(UNLIKELY(ALBuf->OriginalAlign != align))
            SETERR_RETURN(context, AL_INVALID_VALUE,, "Preserving data of mismatched alignment");
    }

    /* Convert the input/source size in bytes to sample frames using the unpack
     * block alignment.
     */
    if(SrcType == UserFmtIMA4)
        SrcByteAlign = ((align-1)/2 + 4) * ChannelsFromUserFmt(SrcChannels);
    else if(SrcType == UserFmtMSADPCM)
        SrcByteAlign = ((align-2)/2 + 7) * ChannelsFromUserFmt(SrcChannels);
    else
        SrcByteAlign = align * FrameSizeFromUserFmt(SrcChannels, SrcType);
    if(UNLIKELY((size%SrcByteAlign) != 0))
        SETERR_RETURN(context, AL_INVALID_VALUE,,
            "Data size %d is not a multiple of frame size %d (%d unpack alignment)",
            size, SrcByteAlign, align);

    if(UNLIKELY(size / SrcByteAlign > INT_MAX / align))
        SETERR_RETURN(context, AL_OUT_OF_MEMORY,,
            "Buffer size overflow, %d blocks x %d samples per block", size/SrcByteAlign, align);
    frames = size / SrcByteAlign * align;

    /* Convert the sample frames to the number of bytes needed for internal
     * storage.
     */
    NumChannels = ChannelsFromFmt(DstChannels);
    FrameSize = NumChannels * BytesFromFmt(DstType);
    if(UNLIKELY(frames > INT_MAX/FrameSize))
        SETERR_RETURN(context, AL_OUT_OF_MEMORY,,
            "Buffer size overflow, %d frames x %d bytes per frame", frames, FrameSize);
    newsize = frames*FrameSize;

    /* Round up to the next 16-byte multiple. This could reallocate only when
     * increasing or the new size is less than half the current, but then the
     * buffer's AL_SIZE would not be very reliable for accounting buffer memory
     * usage, and reporting the real size could cause problems for apps that
     * use AL_SIZE to try to get the buffer's play length.
     */
    if(LIKELY(newsize <= INT_MAX-15))
        newsize = (newsize+15) & ~0xf;
    if(newsize != ALBuf->BytesAlloc)
    {
        void *temp = al_malloc(16, (size_t)newsize);
        if(UNLIKELY(!temp && newsize))
            SETERR_RETURN(context, AL_OUT_OF_MEMORY,, "Failed to allocate %d bytes of storage",
                          newsize);
        if((access&AL_PRESERVE_DATA_BIT_SOFT))
        {
            ALsizei tocopy = mini(newsize, ALBuf->BytesAlloc);
            if(tocopy > 0) memcpy(temp, ALBuf->data, tocopy);
        }
        al_free(ALBuf->data);
        ALBuf->data = temp;
        ALBuf->BytesAlloc = newsize;
    }

    if(SrcType == UserFmtIMA4)
    {
        assert(DstType == FmtShort);
        if(data != NULL && ALBuf->data != NULL)
            Convert_ALshort_ALima4(ALBuf->data, data, NumChannels, frames, align);
        ALBuf->OriginalAlign = align;
    }
    else if(SrcType == UserFmtMSADPCM)
    {
        assert(DstType == FmtShort);
        if(data != NULL && ALBuf->data != NULL)
            Convert_ALshort_ALmsadpcm(ALBuf->data, data, NumChannels, frames, align);
        ALBuf->OriginalAlign = align;
    }
    else
    {
        assert((long)SrcType == (long)DstType);
        if(data != NULL && ALBuf->data != NULL)
            memcpy(ALBuf->data, data, frames*FrameSize);
        ALBuf->OriginalAlign = 1;
    }
    ALBuf->OriginalSize = size;
    ALBuf->OriginalType = SrcType;

    ALBuf->Frequency = freq;
    ALBuf->FmtChannels = DstChannels;
    ALBuf->FmtType = DstType;
    ALBuf->Access = access;

    ALBuf->SampleLen = frames;
    ALBuf->LoopStart = 0;
    ALBuf->LoopEnd = ALBuf->SampleLen;
}


ALsizei BytesFromUserFmt(enum UserFmtType type)
{
    switch(type)
    {
    case UserFmtUByte: return sizeof(ALubyte);
    case UserFmtShort: return sizeof(ALshort);
    case UserFmtFloat: return sizeof(ALfloat);
    case UserFmtDouble: return sizeof(ALdouble);
    case UserFmtMulaw: return sizeof(ALubyte);
    case UserFmtAlaw: return sizeof(ALubyte);
    case UserFmtIMA4: break; /* not handled here */
    case UserFmtMSADPCM: break; /* not handled here */
    }
    return 0;
}
ALsizei ChannelsFromUserFmt(enum UserFmtChannels chans)
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

ALsizei BytesFromFmt(enum FmtType type)
{
    switch(type)
    {
    case FmtUByte: return sizeof(ALubyte);
    case FmtShort: return sizeof(ALshort);
    case FmtFloat: return sizeof(ALfloat);
    case FmtDouble: return sizeof(ALdouble);
    case FmtMulaw: return sizeof(ALubyte);
    case FmtAlaw: return sizeof(ALubyte);
    }
    return 0;
}
ALsizei ChannelsFromFmt(enum FmtChannels chans)
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

static ALsizei SanitizeAlignment(enum UserFmtType type, ALsizei align)
{
    if(align < 0)
        return 0;

    if(align == 0)
    {
        if(type == UserFmtIMA4)
        {
            /* Here is where things vary:
             * nVidia and Apple use 64+1 sample frames per block -> block_size=36 bytes per channel
             * Most PC sound software uses 2040+1 sample frames per block -> block_size=1024 bytes per channel
             */
            return 65;
        }
        if(type == UserFmtMSADPCM)
            return 64;
        return 1;
    }

    if(type == UserFmtIMA4)
    {
        /* IMA4 block alignment must be a multiple of 8, plus 1. */
        if((align&7) == 1) return align;
        return 0;
    }
    if(type == UserFmtMSADPCM)
    {
        /* MSADPCM block alignment must be a multiple of 2. */
        if((align&1) == 0) return align;
        return 0;
    }

    return align;
}


static ALbuffer *AllocBuffer(ALCcontext *context)
{
    ALCdevice *device = context->Device;
    BufferSubList *sublist, *subend;
    ALbuffer *buffer = NULL;
    ALsizei lidx = 0;
    ALsizei slidx;

    almtx_lock(&device->BufferLock);
    sublist = VECTOR_BEGIN(device->BufferList);
    subend = VECTOR_END(device->BufferList);
    for(;sublist != subend;++sublist)
    {
        if(sublist->FreeMask)
        {
            slidx = CTZ64(sublist->FreeMask);
            buffer = sublist->Buffers + slidx;
            break;
        }
        ++lidx;
    }
    if(UNLIKELY(!buffer))
    {
        const BufferSubList empty_sublist = { 0, NULL };
        /* Don't allocate so many list entries that the 32-bit ID could
         * overflow...
         */
        if(UNLIKELY(VECTOR_SIZE(device->BufferList) >= 1<<25))
        {
            almtx_unlock(&device->BufferLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Too many buffers allocated");
            return NULL;
        }
        lidx = (ALsizei)VECTOR_SIZE(device->BufferList);
        VECTOR_PUSH_BACK(device->BufferList, empty_sublist);
        sublist = &VECTOR_BACK(device->BufferList);
        sublist->FreeMask = ~U64(0);
        sublist->Buffers = al_calloc(16, sizeof(ALbuffer)*64);
        if(UNLIKELY(!sublist->Buffers))
        {
            VECTOR_POP_BACK(device->BufferList);
            almtx_unlock(&device->BufferLock);
            alSetError(context, AL_OUT_OF_MEMORY, "Failed to allocate buffer batch");
            return NULL;
        }

        slidx = 0;
        buffer = sublist->Buffers + slidx;
    }

    memset(buffer, 0, sizeof(*buffer));

    /* Add 1 to avoid buffer ID 0. */
    buffer->id = ((lidx<<6) | slidx) + 1;

    sublist->FreeMask &= ~(U64(1)<<slidx);
    almtx_unlock(&device->BufferLock);

    return buffer;
}

static void FreeBuffer(ALCdevice *device, ALbuffer *buffer)
{
    ALuint id = buffer->id - 1;
    ALsizei lidx = id >> 6;
    ALsizei slidx = id & 0x3f;

    al_free(buffer->data);
    memset(buffer, 0, sizeof(*buffer));

    VECTOR_ELEM(device->BufferList, lidx).FreeMask |= U64(1) << slidx;
}


/*
 *    ReleaseALBuffers()
 *
 *    INTERNAL: Called to destroy any buffers that still exist on the device
 */
ALvoid ReleaseALBuffers(ALCdevice *device)
{
    BufferSubList *sublist = VECTOR_BEGIN(device->BufferList);
    BufferSubList *subend = VECTOR_END(device->BufferList);
    size_t leftover = 0;
    for(;sublist != subend;++sublist)
    {
        ALuint64 usemask = ~sublist->FreeMask;
        while(usemask)
        {
            ALsizei idx = CTZ64(usemask);
            ALbuffer *buffer = sublist->Buffers + idx;

            al_free(buffer->data);
            memset(buffer, 0, sizeof(*buffer));
            ++leftover;

            usemask &= ~(U64(1) << idx);
        }
        sublist->FreeMask = ~usemask;
    }
    if(leftover > 0)
        WARN("(%p) Deleted "SZFMT" Buffer%s\n", device, leftover, (leftover==1)?"":"s");
}
