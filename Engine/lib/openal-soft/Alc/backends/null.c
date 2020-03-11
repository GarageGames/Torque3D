/**
 * OpenAL cross platform audio library
 * Copyright (C) 2010 by Chris Robinson
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
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"


typedef struct ALCnullBackend {
    DERIVE_FROM_TYPE(ALCbackend);

    ATOMIC(int) killNow;
    althrd_t thread;
} ALCnullBackend;

static int ALCnullBackend_mixerProc(void *ptr);

static void ALCnullBackend_Construct(ALCnullBackend *self, ALCdevice *device);
static DECLARE_FORWARD(ALCnullBackend, ALCbackend, void, Destruct)
static ALCenum ALCnullBackend_open(ALCnullBackend *self, const ALCchar *name);
static ALCboolean ALCnullBackend_reset(ALCnullBackend *self);
static ALCboolean ALCnullBackend_start(ALCnullBackend *self);
static void ALCnullBackend_stop(ALCnullBackend *self);
static DECLARE_FORWARD2(ALCnullBackend, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCnullBackend, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCnullBackend, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCnullBackend, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCnullBackend, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCnullBackend)

DEFINE_ALCBACKEND_VTABLE(ALCnullBackend);


static const ALCchar nullDevice[] = "No Output";


static void ALCnullBackend_Construct(ALCnullBackend *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCnullBackend, ALCbackend, self);

    ATOMIC_INIT(&self->killNow, AL_TRUE);
}


static int ALCnullBackend_mixerProc(void *ptr)
{
    ALCnullBackend *self = (ALCnullBackend*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    struct timespec now, start;
    ALuint64 avail, done;
    const long restTime = (long)((ALuint64)device->UpdateSize * 1000000000 /
                                 device->Frequency / 2);

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

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
            ALCnullBackend_lock(self);
            aluMixData(device, NULL, device->UpdateSize);
            ALCnullBackend_unlock(self);
            done += device->UpdateSize;
        }
    }

    return 0;
}


static ALCenum ALCnullBackend_open(ALCnullBackend *self, const ALCchar *name)
{
    ALCdevice *device;

    if(!name)
        name = nullDevice;
    else if(strcmp(name, nullDevice) != 0)
        return ALC_INVALID_VALUE;

    device = STATIC_CAST(ALCbackend, self)->mDevice;
    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static ALCboolean ALCnullBackend_reset(ALCnullBackend *self)
{
    SetDefaultWFXChannelOrder(STATIC_CAST(ALCbackend, self)->mDevice);
    return ALC_TRUE;
}

static ALCboolean ALCnullBackend_start(ALCnullBackend *self)
{
    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCnullBackend_mixerProc, self) != althrd_success)
        return ALC_FALSE;
    return ALC_TRUE;
}

static void ALCnullBackend_stop(ALCnullBackend *self)
{
    int res;

    if(ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
        return;
    althrd_join(self->thread, &res);
}


typedef struct ALCnullBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCnullBackendFactory;
#define ALCNULLBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCnullBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCnullBackendFactory_getFactory(void);

static ALCboolean ALCnullBackendFactory_init(ALCnullBackendFactory *self);
static DECLARE_FORWARD(ALCnullBackendFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCnullBackendFactory_querySupport(ALCnullBackendFactory *self, ALCbackend_Type type);
static void ALCnullBackendFactory_probe(ALCnullBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCnullBackendFactory_createBackend(ALCnullBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCnullBackendFactory);


ALCbackendFactory *ALCnullBackendFactory_getFactory(void)
{
    static ALCnullBackendFactory factory = ALCNULLBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCnullBackendFactory_init(ALCnullBackendFactory* UNUSED(self))
{
    return ALC_TRUE;
}

static ALCboolean ALCnullBackendFactory_querySupport(ALCnullBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCnullBackendFactory_probe(ALCnullBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(nullDevice);
            break;
        case CAPTURE_DEVICE_PROBE:
            break;
    }
}

static ALCbackend* ALCnullBackendFactory_createBackend(ALCnullBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCnullBackend *backend;
        NEW_OBJ(backend, ALCnullBackend)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
