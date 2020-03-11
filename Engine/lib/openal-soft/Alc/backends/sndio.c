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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"

#include "backends/base.h"

#include <sndio.h>




typedef struct ALCsndioBackend {
    DERIVE_FROM_TYPE(ALCbackend);

    struct sio_hdl *sndHandle;

    ALvoid *mix_data;
    ALsizei data_size;

    ATOMIC(int) killNow;
    althrd_t thread;
} ALCsndioBackend;

static int ALCsndioBackend_mixerProc(void *ptr);

static void ALCsndioBackend_Construct(ALCsndioBackend *self, ALCdevice *device);
static void ALCsndioBackend_Destruct(ALCsndioBackend *self);
static ALCenum ALCsndioBackend_open(ALCsndioBackend *self, const ALCchar *name);
static ALCboolean ALCsndioBackend_reset(ALCsndioBackend *self);
static ALCboolean ALCsndioBackend_start(ALCsndioBackend *self);
static void ALCsndioBackend_stop(ALCsndioBackend *self);
static DECLARE_FORWARD2(ALCsndioBackend, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCsndioBackend, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCsndioBackend, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCsndioBackend, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCsndioBackend, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCsndioBackend)

DEFINE_ALCBACKEND_VTABLE(ALCsndioBackend);


static const ALCchar sndio_device[] = "SndIO Default";


static void ALCsndioBackend_Construct(ALCsndioBackend *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCsndioBackend, ALCbackend, self);

    self->sndHandle = NULL;
    self->mix_data = NULL;
    ATOMIC_INIT(&self->killNow, AL_TRUE);
}

static void ALCsndioBackend_Destruct(ALCsndioBackend *self)
{
    if(self->sndHandle)
        sio_close(self->sndHandle);
    self->sndHandle = NULL;

    al_free(self->mix_data);
    self->mix_data = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static int ALCsndioBackend_mixerProc(void *ptr)
{
    ALCsndioBackend *self = (ALCsndioBackend*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALsizei frameSize;
    size_t wrote;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);

    while(!ATOMIC_LOAD(&self->killNow, almemory_order_acquire) &&
          ATOMIC_LOAD(&device->Connected, almemory_order_acquire))
    {
        ALsizei len = self->data_size;
        ALubyte *WritePtr = self->mix_data;

        ALCsndioBackend_lock(self);
        aluMixData(device, WritePtr, len/frameSize);
        ALCsndioBackend_unlock(self);
        while(len > 0 && !ATOMIC_LOAD(&self->killNow, almemory_order_acquire))
        {
            wrote = sio_write(self->sndHandle, WritePtr, len);
            if(wrote == 0)
            {
                ERR("sio_write failed\n");
                ALCdevice_Lock(device);
                aluHandleDisconnect(device, "Failed to write playback samples");
                ALCdevice_Unlock(device);
                break;
            }

            len -= wrote;
            WritePtr += wrote;
        }
    }

    return 0;
}


static ALCenum ALCsndioBackend_open(ALCsndioBackend *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;

    if(!name)
        name = sndio_device;
    else if(strcmp(name, sndio_device) != 0)
        return ALC_INVALID_VALUE;

    self->sndHandle = sio_open(NULL, SIO_PLAY, 0);
    if(self->sndHandle == NULL)
    {
        ERR("Could not open device\n");
        return ALC_INVALID_VALUE;
    }

    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static ALCboolean ALCsndioBackend_reset(ALCsndioBackend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    struct sio_par par;

    sio_initpar(&par);

    par.rate = device->Frequency;
    par.pchan = ((device->FmtChans != DevFmtMono) ? 2 : 1);

    switch(device->FmtType)
    {
        case DevFmtByte:
            par.bits = 8;
            par.sig = 1;
            break;
        case DevFmtUByte:
            par.bits = 8;
            par.sig = 0;
            break;
        case DevFmtFloat:
        case DevFmtShort:
            par.bits = 16;
            par.sig = 1;
            break;
        case DevFmtUShort:
            par.bits = 16;
            par.sig = 0;
            break;
        case DevFmtInt:
            par.bits = 32;
            par.sig = 1;
            break;
        case DevFmtUInt:
            par.bits = 32;
            par.sig = 0;
            break;
    }
    par.le = SIO_LE_NATIVE;

    par.round = device->UpdateSize;
    par.appbufsz = device->UpdateSize * (device->NumUpdates-1);
    if(!par.appbufsz) par.appbufsz = device->UpdateSize;

    if(!sio_setpar(self->sndHandle, &par) || !sio_getpar(self->sndHandle, &par))
    {
        ERR("Failed to set device parameters\n");
        return ALC_FALSE;
    }

    if(par.bits != par.bps*8)
    {
        ERR("Padded samples not supported (%u of %u bits)\n", par.bits, par.bps*8);
        return ALC_FALSE;
    }

    device->Frequency = par.rate;
    device->FmtChans = ((par.pchan==1) ? DevFmtMono : DevFmtStereo);

    if(par.bits == 8 && par.sig == 1)
        device->FmtType = DevFmtByte;
    else if(par.bits == 8 && par.sig == 0)
        device->FmtType = DevFmtUByte;
    else if(par.bits == 16 && par.sig == 1)
        device->FmtType = DevFmtShort;
    else if(par.bits == 16 && par.sig == 0)
        device->FmtType = DevFmtUShort;
    else if(par.bits == 32 && par.sig == 1)
        device->FmtType = DevFmtInt;
    else if(par.bits == 32 && par.sig == 0)
        device->FmtType = DevFmtUInt;
    else
    {
        ERR("Unhandled sample format: %s %u-bit\n", (par.sig?"signed":"unsigned"), par.bits);
        return ALC_FALSE;
    }

    device->UpdateSize = par.round;
    device->NumUpdates = (par.bufsz/par.round) + 1;

    SetDefaultChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCsndioBackend_start(ALCsndioBackend *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;

    self->data_size = device->UpdateSize * FrameSizeFromDevFmt(
        device->FmtChans, device->FmtType, device->AmbiOrder
    );
    al_free(self->mix_data);
    self->mix_data = al_calloc(16, self->data_size);

    if(!sio_start(self->sndHandle))
    {
        ERR("Error starting playback\n");
        return ALC_FALSE;
    }

    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCsndioBackend_mixerProc, self) != althrd_success)
    {
        sio_stop(self->sndHandle);
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCsndioBackend_stop(ALCsndioBackend *self)
{
    int res;

    if(ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
        return;
    althrd_join(self->thread, &res);

    if(!sio_stop(self->sndHandle))
        ERR("Error stopping device\n");

    al_free(self->mix_data);
    self->mix_data = NULL;
}


typedef struct ALCsndioBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCsndioBackendFactory;
#define ALCSNDIOBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCsndioBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCsndioBackendFactory_getFactory(void);

static ALCboolean ALCsndioBackendFactory_init(ALCsndioBackendFactory *self);
static DECLARE_FORWARD(ALCsndioBackendFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCsndioBackendFactory_querySupport(ALCsndioBackendFactory *self, ALCbackend_Type type);
static void ALCsndioBackendFactory_probe(ALCsndioBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCsndioBackendFactory_createBackend(ALCsndioBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCsndioBackendFactory);


ALCbackendFactory *ALCsndioBackendFactory_getFactory(void)
{
    static ALCsndioBackendFactory factory = ALCSNDIOBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCsndioBackendFactory_init(ALCsndioBackendFactory* UNUSED(self))
{
    /* No dynamic loading */
    return ALC_TRUE;
}

static ALCboolean ALCsndioBackendFactory_querySupport(ALCsndioBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCsndioBackendFactory_probe(ALCsndioBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(sndio_device);
            break;
        case CAPTURE_DEVICE_PROBE:
            break;
    }
}

static ALCbackend* ALCsndioBackendFactory_createBackend(ALCsndioBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCsndioBackend *backend;
        NEW_OBJ(backend, ALCsndioBackend)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
