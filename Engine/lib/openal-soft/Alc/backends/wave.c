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
#include <memory.h>
#include <errno.h>

#include "alMain.h"
#include "alu.h"
#include "alconfig.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"


static const ALCchar waveDevice[] = "Wave File Writer";

static const ALubyte SUBTYPE_PCM[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa,
    0x00, 0x38, 0x9b, 0x71
};
static const ALubyte SUBTYPE_FLOAT[] = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa,
    0x00, 0x38, 0x9b, 0x71
};

static const ALubyte SUBTYPE_BFORMAT_PCM[] = {
    0x01, 0x00, 0x00, 0x00, 0x21, 0x07, 0xd3, 0x11, 0x86, 0x44, 0xc8, 0xc1,
    0xca, 0x00, 0x00, 0x00
};

static const ALubyte SUBTYPE_BFORMAT_FLOAT[] = {
    0x03, 0x00, 0x00, 0x00, 0x21, 0x07, 0xd3, 0x11, 0x86, 0x44, 0xc8, 0xc1,
    0xca, 0x00, 0x00, 0x00
};

static void fwrite16le(ALushort val, FILE *f)
{
    ALubyte data[2] = { val&0xff, (val>>8)&0xff };
    fwrite(data, 1, 2, f);
}

static void fwrite32le(ALuint val, FILE *f)
{
    ALubyte data[4] = { val&0xff, (val>>8)&0xff, (val>>16)&0xff, (val>>24)&0xff };
    fwrite(data, 1, 4, f);
}


typedef struct ALCwaveBackend {
    DERIVE_FROM_TYPE(ALCbackend);

    FILE *mFile;
    long mDataStart;

    ALvoid *mBuffer;
    ALuint mSize;

    ATOMIC(ALenum) killNow;
    althrd_t thread;
} ALCwaveBackend;

static int ALCwaveBackend_mixerProc(void *ptr);

static void ALCwaveBackend_Construct(ALCwaveBackend *self, ALCdevice *device);
static void ALCwaveBackend_Destruct(ALCwaveBackend *self);
static ALCenum ALCwaveBackend_open(ALCwaveBackend *self, const ALCchar *name);
static ALCboolean ALCwaveBackend_reset(ALCwaveBackend *self);
static ALCboolean ALCwaveBackend_start(ALCwaveBackend *self);
static void ALCwaveBackend_stop(ALCwaveBackend *self);
static DECLARE_FORWARD2(ALCwaveBackend, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCwaveBackend, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCwaveBackend, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCwaveBackend, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCwaveBackend, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCwaveBackend)

DEFINE_ALCBACKEND_VTABLE(ALCwaveBackend);


static void ALCwaveBackend_Construct(ALCwaveBackend *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCwaveBackend, ALCbackend, self);

    self->mFile = NULL;
    self->mDataStart = -1;

    self->mBuffer = NULL;
    self->mSize = 0;

    ATOMIC_INIT(&self->killNow, AL_TRUE);
}

static void ALCwaveBackend_Destruct(ALCwaveBackend *self)
{
    if(self->mFile)
        fclose(self->mFile);
    self->mFile = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}

static int ALCwaveBackend_mixerProc(void *ptr)
{
    ALCwaveBackend *self = (ALCwaveBackend*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    struct timespec now, start;
    ALint64 avail, done;
    ALuint frameSize;
    size_t fs;
    const long restTime = (long)((ALuint64)device->UpdateSize * 1000000000 /
                                 device->Frequency / 2);

    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);

    done = 0;
    if(altimespec_get(&start, AL_TIME_UTC) != AL_TIME_UTC)
    {
        ERR("Failed to get starting time\n");
        return 1;
    }
    while(!ATOMIC_LOAD(&self->killNow, almemory_order_acquire) &&
          ATOMIC_LOAD(&device->Connected, almemory_order_acquire))
    {
        if(altimespec_get(&now, AL_TIME_UTC) != AL_TIME_UTC)
        {
            ERR("Failed to get current time\n");
            return 1;
        }

        avail  = (now.tv_sec - start.tv_sec) * device->Frequency;
        avail += (ALint64)(now.tv_nsec - start.tv_nsec) * device->Frequency / 1000000000;
        if(avail < done)
        {
            /* Oops, time skipped backwards. Reset the number of samples done
             * with one update available since we (likely) just came back from
             * sleeping. */
            done = avail - device->UpdateSize;
        }

        if(avail-done < device->UpdateSize)
            al_nssleep(restTime);
        else while(avail-done >= device->UpdateSize)
        {
            ALCwaveBackend_lock(self);
            aluMixData(device, self->mBuffer, device->UpdateSize);
            ALCwaveBackend_unlock(self);
            done += device->UpdateSize;

            if(!IS_LITTLE_ENDIAN)
            {
                ALuint bytesize = BytesFromDevFmt(device->FmtType);
                ALuint i;

                if(bytesize == 2)
                {
                    ALushort *samples = self->mBuffer;
                    ALuint len = self->mSize / 2;
                    for(i = 0;i < len;i++)
                    {
                        ALushort samp = samples[i];
                        samples[i] = (samp>>8) | (samp<<8);
                    }
                }
                else if(bytesize == 4)
                {
                    ALuint *samples = self->mBuffer;
                    ALuint len = self->mSize / 4;
                    for(i = 0;i < len;i++)
                    {
                        ALuint samp = samples[i];
                        samples[i] = (samp>>24) | ((samp>>8)&0x0000ff00) |
                                     ((samp<<8)&0x00ff0000) | (samp<<24);
                    }
                }
            }

            fs = fwrite(self->mBuffer, frameSize, device->UpdateSize, self->mFile);
            (void)fs;
            if(ferror(self->mFile))
            {
                ERR("Error writing to file\n");
                ALCdevice_Lock(device);
                aluHandleDisconnect(device, "Failed to write playback samples");
                ALCdevice_Unlock(device);
                break;
            }
        }
    }

    return 0;
}


static ALCenum ALCwaveBackend_open(ALCwaveBackend *self, const ALCchar *name)
{
    ALCdevice *device;
    const char *fname;

    fname = GetConfigValue(NULL, "wave", "file", "");
    if(!fname[0]) return ALC_INVALID_VALUE;

    if(!name)
        name = waveDevice;
    else if(strcmp(name, waveDevice) != 0)
        return ALC_INVALID_VALUE;

    self->mFile = al_fopen(fname, "wb");
    if(!self->mFile)
    {
        ERR("Could not open file '%s': %s\n", fname, strerror(errno));
        return ALC_INVALID_VALUE;
    }

    device = STATIC_CAST(ALCbackend, self)->mDevice;
    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static ALCboolean ALCwaveBackend_reset(ALCwaveBackend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALuint channels=0, bits=0, chanmask=0;
    int isbformat = 0;
    size_t val;

    fseek(self->mFile, 0, SEEK_SET);
    clearerr(self->mFile);

    if(GetConfigValueBool(NULL, "wave", "bformat", 0))
    {
        device->FmtChans = DevFmtAmbi3D;
        device->AmbiOrder = 1;
    }

    switch(device->FmtType)
    {
        case DevFmtByte:
            device->FmtType = DevFmtUByte;
            break;
        case DevFmtUShort:
            device->FmtType = DevFmtShort;
            break;
        case DevFmtUInt:
            device->FmtType = DevFmtInt;
            break;
        case DevFmtUByte:
        case DevFmtShort:
        case DevFmtInt:
        case DevFmtFloat:
            break;
    }
    switch(device->FmtChans)
    {
        case DevFmtMono:   chanmask = 0x04; break;
        case DevFmtStereo: chanmask = 0x01 | 0x02; break;
        case DevFmtQuad:   chanmask = 0x01 | 0x02 | 0x10 | 0x20; break;
        case DevFmtX51: chanmask = 0x01 | 0x02 | 0x04 | 0x08 | 0x200 | 0x400; break;
        case DevFmtX51Rear: chanmask = 0x01 | 0x02 | 0x04 | 0x08 | 0x010 | 0x020; break;
        case DevFmtX61: chanmask = 0x01 | 0x02 | 0x04 | 0x08 | 0x100 | 0x200 | 0x400; break;
        case DevFmtX71: chanmask = 0x01 | 0x02 | 0x04 | 0x08 | 0x010 | 0x020 | 0x200 | 0x400; break;
        case DevFmtAmbi3D:
            /* .amb output requires FuMa */
            device->AmbiLayout = AmbiLayout_FuMa;
            device->AmbiScale = AmbiNorm_FuMa;
            isbformat = 1;
            chanmask = 0;
            break;
    }
    bits = BytesFromDevFmt(device->FmtType) * 8;
    channels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);

    fputs("RIFF", self->mFile);
    fwrite32le(0xFFFFFFFF, self->mFile); // 'RIFF' header len; filled in at close

    fputs("WAVE", self->mFile);

    fputs("fmt ", self->mFile);
    fwrite32le(40, self->mFile); // 'fmt ' header len; 40 bytes for EXTENSIBLE

    // 16-bit val, format type id (extensible: 0xFFFE)
    fwrite16le(0xFFFE, self->mFile);
    // 16-bit val, channel count
    fwrite16le(channels, self->mFile);
    // 32-bit val, frequency
    fwrite32le(device->Frequency, self->mFile);
    // 32-bit val, bytes per second
    fwrite32le(device->Frequency * channels * bits / 8, self->mFile);
    // 16-bit val, frame size
    fwrite16le(channels * bits / 8, self->mFile);
    // 16-bit val, bits per sample
    fwrite16le(bits, self->mFile);
    // 16-bit val, extra byte count
    fwrite16le(22, self->mFile);
    // 16-bit val, valid bits per sample
    fwrite16le(bits, self->mFile);
    // 32-bit val, channel mask
    fwrite32le(chanmask, self->mFile);
    // 16 byte GUID, sub-type format
    val = fwrite((device->FmtType == DevFmtFloat) ?
                 (isbformat ? SUBTYPE_BFORMAT_FLOAT : SUBTYPE_FLOAT) :
                 (isbformat ? SUBTYPE_BFORMAT_PCM : SUBTYPE_PCM), 1, 16, self->mFile);
    (void)val;

    fputs("data", self->mFile);
    fwrite32le(0xFFFFFFFF, self->mFile); // 'data' header len; filled in at close

    if(ferror(self->mFile))
    {
        ERR("Error writing header: %s\n", strerror(errno));
        return ALC_FALSE;
    }
    self->mDataStart = ftell(self->mFile);

    SetDefaultWFXChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCwaveBackend_start(ALCwaveBackend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    self->mSize = device->UpdateSize * FrameSizeFromDevFmt(
        device->FmtChans, device->FmtType, device->AmbiOrder
    );
    self->mBuffer = malloc(self->mSize);
    if(!self->mBuffer)
    {
        ERR("Buffer malloc failed\n");
        return ALC_FALSE;
    }

    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCwaveBackend_mixerProc, self) != althrd_success)
    {
        free(self->mBuffer);
        self->mBuffer = NULL;
        self->mSize = 0;
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCwaveBackend_stop(ALCwaveBackend *self)
{
    ALuint dataLen;
    long size;
    int res;

    if(ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
        return;
    althrd_join(self->thread, &res);

    free(self->mBuffer);
    self->mBuffer = NULL;

    size = ftell(self->mFile);
    if(size > 0)
    {
        dataLen = size - self->mDataStart;
        if(fseek(self->mFile, self->mDataStart-4, SEEK_SET) == 0)
            fwrite32le(dataLen, self->mFile); // 'data' header len
        if(fseek(self->mFile, 4, SEEK_SET) == 0)
            fwrite32le(size-8, self->mFile); // 'WAVE' header len
    }
}


typedef struct ALCwaveBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCwaveBackendFactory;
#define ALCWAVEBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCwaveBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCwaveBackendFactory_getFactory(void);

static ALCboolean ALCwaveBackendFactory_init(ALCwaveBackendFactory *self);
static DECLARE_FORWARD(ALCwaveBackendFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCwaveBackendFactory_querySupport(ALCwaveBackendFactory *self, ALCbackend_Type type);
static void ALCwaveBackendFactory_probe(ALCwaveBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCwaveBackendFactory_createBackend(ALCwaveBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCwaveBackendFactory);


ALCbackendFactory *ALCwaveBackendFactory_getFactory(void)
{
    static ALCwaveBackendFactory factory = ALCWAVEBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCwaveBackendFactory_init(ALCwaveBackendFactory* UNUSED(self))
{
    return ALC_TRUE;
}

static ALCboolean ALCwaveBackendFactory_querySupport(ALCwaveBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return !!ConfigValueExists(NULL, "wave", "file");
    return ALC_FALSE;
}

static void ALCwaveBackendFactory_probe(ALCwaveBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(waveDevice);
            break;
        case CAPTURE_DEVICE_PROBE:
            break;
    }
}

static ALCbackend* ALCwaveBackendFactory_createBackend(ALCwaveBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCwaveBackend *backend;
        NEW_OBJ(backend, ALCwaveBackend)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
