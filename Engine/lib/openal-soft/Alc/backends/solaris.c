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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"

#include <sys/audioio.h>


typedef struct ALCsolarisBackend {
    DERIVE_FROM_TYPE(ALCbackend);

    int fd;

    ALubyte *mix_data;
    int data_size;

    volatile int killNow;
    althrd_t thread;
} ALCsolarisBackend;

static int ALCsolarisBackend_mixerProc(void *ptr);

static void ALCsolarisBackend_Construct(ALCsolarisBackend *self, ALCdevice *device);
static void ALCsolarisBackend_Destruct(ALCsolarisBackend *self);
static ALCenum ALCsolarisBackend_open(ALCsolarisBackend *self, const ALCchar *name);
static void ALCsolarisBackend_close(ALCsolarisBackend *self);
static ALCboolean ALCsolarisBackend_reset(ALCsolarisBackend *self);
static ALCboolean ALCsolarisBackend_start(ALCsolarisBackend *self);
static void ALCsolarisBackend_stop(ALCsolarisBackend *self);
static DECLARE_FORWARD2(ALCsolarisBackend, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCsolarisBackend, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCsolarisBackend, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCsolarisBackend, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCsolarisBackend, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCsolarisBackend)

DEFINE_ALCBACKEND_VTABLE(ALCsolarisBackend);


static const ALCchar solaris_device[] = "Solaris Default";

static const char *solaris_driver = "/dev/audio";


static void ALCsolarisBackend_Construct(ALCsolarisBackend *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCsolarisBackend, ALCbackend, self);

    self->fd = -1;
}

static void ALCsolarisBackend_Destruct(ALCsolarisBackend *self)
{
    if(self->fd != -1)
        close(self->fd);
    self->fd = -1;

    free(self->mix_data);
    self->mix_data = NULL;
    self->data_size = 0;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static int ALCsolarisBackend_mixerProc(void *ptr)
{
    ALCsolarisBackend *self = ptr;
    ALCdevice *Device = STATIC_CAST(ALCbackend,self)->mDevice;
    ALint frameSize;
    int wrote;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    frameSize = FrameSizeFromDevFmt(Device->FmtChans, Device->FmtType);

    while(!self->killNow && Device->Connected)
    {
        ALint len = self->data_size;
        ALubyte *WritePtr = self->mix_data;

        aluMixData(Device, WritePtr, len/frameSize);
        while(len > 0 && !self->killNow)
        {
            wrote = write(self->fd, WritePtr, len);
            if(wrote < 0)
            {
                if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    ERR("write failed: %s\n", strerror(errno));
                    ALCsolarisBackend_lock(self);
                    aluHandleDisconnect(Device);
                    ALCsolarisBackend_unlock(self);
                    break;
                }

                al_nssleep(1000000);
                continue;
            }

            len -= wrote;
            WritePtr += wrote;
        }
    }

    return 0;
}


static ALCenum ALCsolarisBackend_open(ALCsolarisBackend *self, const ALCchar *name)
{
    ALCdevice *device;

    if(!name)
        name = solaris_device;
    else if(strcmp(name, solaris_device) != 0)
        return ALC_INVALID_VALUE;

    self->fd = open(solaris_driver, O_WRONLY);
    if(self->fd == -1)
    {
        ERR("Could not open %s: %s\n", solaris_driver, strerror(errno));
        return ALC_INVALID_VALUE;
    }

    device = STATIC_CAST(ALCbackend,self)->mDevice;
    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static void ALCsolarisBackend_close(ALCsolarisBackend *self)
{
    close(self->fd);
    self->fd = -1;
}

static ALCboolean ALCsolarisBackend_reset(ALCsolarisBackend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    audio_info_t info;
    ALuint frameSize;
    int numChannels;

    AUDIO_INITINFO(&info);

    info.play.sample_rate = device->Frequency;

    if(device->FmtChans != DevFmtMono)
        device->FmtChans = DevFmtStereo;
    numChannels = ChannelsFromDevFmt(device->FmtChans);
    info.play.channels = numChannels;

    switch(device->FmtType)
    {
        case DevFmtByte:
            info.play.precision = 8;
            info.play.encoding = AUDIO_ENCODING_LINEAR;
            break;
        case DevFmtUByte:
            info.play.precision = 8;
            info.play.encoding = AUDIO_ENCODING_LINEAR8;
            break;
        case DevFmtUShort:
        case DevFmtInt:
        case DevFmtUInt:
        case DevFmtFloat:
            device->FmtType = DevFmtShort;
            /* fall-through */
        case DevFmtShort:
            info.play.precision = 16;
            info.play.encoding = AUDIO_ENCODING_LINEAR;
            break;
    }

    frameSize = numChannels * BytesFromDevFmt(device->FmtType);
    info.play.buffer_size = device->UpdateSize*device->NumUpdates * frameSize;

    if(ioctl(self->fd, AUDIO_SETINFO, &info) < 0)
    {
        ERR("ioctl failed: %s\n", strerror(errno));
        return ALC_FALSE;
    }

    if(ChannelsFromDevFmt(device->FmtChans) != info.play.channels)
    {
        ERR("Could not set %d channels, got %d instead\n", ChannelsFromDevFmt(device->FmtChans), info.play.channels);
        return ALC_FALSE;
    }

    if(!((info.play.precision == 8 && info.play.encoding == AUDIO_ENCODING_LINEAR8 && device->FmtType == DevFmtUByte) ||
         (info.play.precision == 8 && info.play.encoding == AUDIO_ENCODING_LINEAR && device->FmtType == DevFmtByte) ||
         (info.play.precision == 16 && info.play.encoding == AUDIO_ENCODING_LINEAR && device->FmtType == DevFmtShort) ||
         (info.play.precision == 32 && info.play.encoding == AUDIO_ENCODING_LINEAR && device->FmtType == DevFmtInt)))
    {
        ERR("Could not set %s samples, got %d (0x%x)\n", DevFmtTypeString(device->FmtType),
            info.play.precision, info.play.encoding);
        return ALC_FALSE;
    }

    device->Frequency = info.play.sample_rate;
    device->UpdateSize = (info.play.buffer_size/device->NumUpdates) + 1;

    SetDefaultChannelOrder(device);

    free(self->mix_data);
    self->data_size = device->UpdateSize * FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    self->mix_data = calloc(1, self->data_size);

    return ALC_TRUE;
}

static ALCboolean ALCsolarisBackend_start(ALCsolarisBackend *self)
{
    self->killNow = 0;
    if(althrd_create(&self->thread, ALCsolarisBackend_mixerProc, self) != althrd_success)
        return ALC_FALSE;
    return ALC_TRUE;
}

static void ALCsolarisBackend_stop(ALCsolarisBackend *self)
{
    int res;

    if(self->killNow)
        return;

    self->killNow = 1;
    althrd_join(self->thread, &res);

    if(ioctl(self->fd, AUDIO_DRAIN) < 0)
        ERR("Error draining device: %s\n", strerror(errno));
}


typedef struct ALCsolarisBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCsolarisBackendFactory;
#define ALCSOLARISBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCsolarisBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCsolarisBackendFactory_getFactory(void);

static ALCboolean ALCsolarisBackendFactory_init(ALCsolarisBackendFactory *self);
static DECLARE_FORWARD(ALCsolarisBackendFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCsolarisBackendFactory_querySupport(ALCsolarisBackendFactory *self, ALCbackend_Type type);
static void ALCsolarisBackendFactory_probe(ALCsolarisBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCsolarisBackendFactory_createBackend(ALCsolarisBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCsolarisBackendFactory);


ALCbackendFactory *ALCsolarisBackendFactory_getFactory(void)
{
    static ALCsolarisBackendFactory factory = ALCSOLARISBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCsolarisBackendFactory_init(ALCsolarisBackendFactory* UNUSED(self))
{
    ConfigValueStr(NULL, "solaris", "device", &solaris_driver);
    return ALC_TRUE;
}

static ALCboolean ALCsolarisBackendFactory_querySupport(ALCsolarisBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCsolarisBackendFactory_probe(ALCsolarisBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
        {
#ifdef HAVE_STAT
            struct stat buf;
            if(stat(solaris_driver, &buf) == 0)
#endif
                AppendAllDevicesList(solaris_device);
        }
        break;

        case CAPTURE_DEVICE_PROBE:
            break;
    }
}

ALCbackend* ALCsolarisBackendFactory_createBackend(ALCsolarisBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCsolarisBackend *backend;
        NEW_OBJ(backend, ALCsolarisBackend)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
