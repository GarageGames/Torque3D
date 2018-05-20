/**
 * OpenAL cross platform audio library
 * Copyright (C) 2018 by authors.
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
#include <SDL2/SDL.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"


#ifdef _WIN32
#define DEVNAME_PREFIX "OpenAL Soft on "
#else
#define DEVNAME_PREFIX ""
#endif

typedef struct ALCsdl2Backend {
    DERIVE_FROM_TYPE(ALCbackend);

    SDL_AudioDeviceID deviceID;
    ALsizei frameSize;

    ALuint Frequency;
    enum DevFmtChannels FmtChans;
    enum DevFmtType     FmtType;
    ALuint UpdateSize;
} ALCsdl2Backend;

static void ALCsdl2Backend_Construct(ALCsdl2Backend *self, ALCdevice *device);
static void ALCsdl2Backend_Destruct(ALCsdl2Backend *self);
static ALCenum ALCsdl2Backend_open(ALCsdl2Backend *self, const ALCchar *name);
static ALCboolean ALCsdl2Backend_reset(ALCsdl2Backend *self);
static ALCboolean ALCsdl2Backend_start(ALCsdl2Backend *self);
static void ALCsdl2Backend_stop(ALCsdl2Backend *self);
static DECLARE_FORWARD2(ALCsdl2Backend, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCsdl2Backend, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCsdl2Backend, ALCbackend, ClockLatency, getClockLatency)
static void ALCsdl2Backend_lock(ALCsdl2Backend *self);
static void ALCsdl2Backend_unlock(ALCsdl2Backend *self);
DECLARE_DEFAULT_ALLOCATORS(ALCsdl2Backend)

DEFINE_ALCBACKEND_VTABLE(ALCsdl2Backend);

static const ALCchar defaultDeviceName[] = DEVNAME_PREFIX "Default Device";

static void ALCsdl2Backend_Construct(ALCsdl2Backend *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCsdl2Backend, ALCbackend, self);

    self->deviceID = 0;
    self->frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);
    self->Frequency = device->Frequency;
    self->FmtChans = device->FmtChans;
    self->FmtType = device->FmtType;
    self->UpdateSize = device->UpdateSize;
}

static void ALCsdl2Backend_Destruct(ALCsdl2Backend *self)
{
    if(self->deviceID)
        SDL_CloseAudioDevice(self->deviceID);
    self->deviceID = 0;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static void ALCsdl2Backend_audioCallback(void *ptr, Uint8 *stream, int len)
{
    ALCsdl2Backend *self = (ALCsdl2Backend*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    assert((len % self->frameSize) == 0);
    aluMixData(device, stream, len / self->frameSize);
}

static ALCenum ALCsdl2Backend_open(ALCsdl2Backend *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    SDL_AudioSpec want, have;

    SDL_zero(want);
    SDL_zero(have);

    want.freq = device->Frequency;
    switch(device->FmtType)
    {
        case DevFmtUByte: want.format = AUDIO_U8; break;
        case DevFmtByte: want.format = AUDIO_S8; break;
        case DevFmtUShort: want.format = AUDIO_U16SYS; break;
        case DevFmtShort: want.format = AUDIO_S16SYS; break;
        case DevFmtUInt: /* fall-through */
        case DevFmtInt: want.format = AUDIO_S32SYS; break;
        case DevFmtFloat: want.format = AUDIO_F32; break;
    }
    want.channels = (device->FmtChans == DevFmtMono) ? 1 : 2;
    want.samples = device->UpdateSize;
    want.callback = ALCsdl2Backend_audioCallback;
    want.userdata = self;

    /* Passing NULL to SDL_OpenAudioDevice opens a default, which isn't
     * necessarily the first in the list.
     */
    if(!name || strcmp(name, defaultDeviceName) == 0)
        self->deviceID = SDL_OpenAudioDevice(NULL, SDL_FALSE, &want, &have,
                                             SDL_AUDIO_ALLOW_ANY_CHANGE);
    else
    {
        const size_t prefix_len = strlen(DEVNAME_PREFIX);
        if(strncmp(name, DEVNAME_PREFIX, prefix_len) == 0)
            self->deviceID = SDL_OpenAudioDevice(name+prefix_len, SDL_FALSE, &want, &have,
                                                 SDL_AUDIO_ALLOW_ANY_CHANGE);
        else
            self->deviceID = SDL_OpenAudioDevice(name, SDL_FALSE, &want, &have,
                                                 SDL_AUDIO_ALLOW_ANY_CHANGE);
    }
    if(self->deviceID == 0)
        return ALC_INVALID_VALUE;

    device->Frequency = have.freq;
    if(have.channels == 1)
        device->FmtChans = DevFmtMono;
    else if(have.channels == 2)
        device->FmtChans = DevFmtStereo;
    else
    {
        ERR("Got unhandled SDL channel count: %d\n", (int)have.channels);
        return ALC_INVALID_VALUE;
    }
    switch(have.format)
    {
        case AUDIO_U8:     device->FmtType = DevFmtUByte;  break;
        case AUDIO_S8:     device->FmtType = DevFmtByte;   break;
        case AUDIO_U16SYS: device->FmtType = DevFmtUShort; break;
        case AUDIO_S16SYS: device->FmtType = DevFmtShort;  break;
        case AUDIO_S32SYS: device->FmtType = DevFmtInt;    break;
        case AUDIO_F32SYS: device->FmtType = DevFmtFloat;  break;
        default:
            ERR("Got unsupported SDL format: 0x%04x\n", have.format);
            return ALC_INVALID_VALUE;
    }
    device->UpdateSize = have.samples;
    device->NumUpdates = 2; /* SDL always (tries to) use two periods. */

    self->frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);
    self->Frequency = device->Frequency;
    self->FmtChans = device->FmtChans;
    self->FmtType = device->FmtType;
    self->UpdateSize = device->UpdateSize;

    alstr_copy_cstr(&device->DeviceName, name ? name : defaultDeviceName);

    return ALC_NO_ERROR;
}

static ALCboolean ALCsdl2Backend_reset(ALCsdl2Backend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    device->Frequency = self->Frequency;
    device->FmtChans = self->FmtChans;
    device->FmtType = self->FmtType;
    device->UpdateSize = self->UpdateSize;
    device->NumUpdates = 2;
    SetDefaultWFXChannelOrder(device);
    return ALC_TRUE;
}

static ALCboolean ALCsdl2Backend_start(ALCsdl2Backend *self)
{
    SDL_PauseAudioDevice(self->deviceID, 0);
    return ALC_TRUE;
}

static void ALCsdl2Backend_stop(ALCsdl2Backend *self)
{
    SDL_PauseAudioDevice(self->deviceID, 1);
}

static void ALCsdl2Backend_lock(ALCsdl2Backend *self)
{
    SDL_LockAudioDevice(self->deviceID);
}

static void ALCsdl2Backend_unlock(ALCsdl2Backend *self)
{
    SDL_UnlockAudioDevice(self->deviceID);
}


typedef struct ALCsdl2BackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCsdl2BackendFactory;
#define ALCsdl2BACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCsdl2BackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCsdl2BackendFactory_getFactory(void);

static ALCboolean ALCsdl2BackendFactory_init(ALCsdl2BackendFactory *self);
static void ALCsdl2BackendFactory_deinit(ALCsdl2BackendFactory *self);
static ALCboolean ALCsdl2BackendFactory_querySupport(ALCsdl2BackendFactory *self, ALCbackend_Type type);
static void ALCsdl2BackendFactory_probe(ALCsdl2BackendFactory *self, enum DevProbe type);
static ALCbackend* ALCsdl2BackendFactory_createBackend(ALCsdl2BackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCsdl2BackendFactory);


ALCbackendFactory *ALCsdl2BackendFactory_getFactory(void)
{
    static ALCsdl2BackendFactory factory = ALCsdl2BACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCsdl2BackendFactory_init(ALCsdl2BackendFactory* UNUSED(self))
{
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) == 0)
        return AL_TRUE;
    return ALC_FALSE;
}

static void ALCsdl2BackendFactory_deinit(ALCsdl2BackendFactory* UNUSED(self))
{
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static ALCboolean ALCsdl2BackendFactory_querySupport(ALCsdl2BackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCsdl2BackendFactory_probe(ALCsdl2BackendFactory* UNUSED(self), enum DevProbe type)
{
    int num_devices, i;
    al_string name;

    if(type != ALL_DEVICE_PROBE)
        return;

    AL_STRING_INIT(name);
    num_devices = SDL_GetNumAudioDevices(SDL_FALSE);

    AppendAllDevicesList(defaultDeviceName);
    for(i = 0;i < num_devices;++i)
    {
        alstr_copy_cstr(&name, DEVNAME_PREFIX);
        alstr_append_cstr(&name, SDL_GetAudioDeviceName(i, SDL_FALSE));
        AppendAllDevicesList(alstr_get_cstr(name));
    }
    alstr_reset(&name);
}

static ALCbackend* ALCsdl2BackendFactory_createBackend(ALCsdl2BackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCsdl2Backend *backend;
        NEW_OBJ(backend, ALCsdl2Backend)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
