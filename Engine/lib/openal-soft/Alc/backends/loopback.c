/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by Chris Robinson
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

#include "backends/base.h"


typedef struct ALCloopback {
    DERIVE_FROM_TYPE(ALCbackend);
} ALCloopback;

static void ALCloopback_Construct(ALCloopback *self, ALCdevice *device);
static DECLARE_FORWARD(ALCloopback, ALCbackend, void, Destruct)
static ALCenum ALCloopback_open(ALCloopback *self, const ALCchar *name);
static ALCboolean ALCloopback_reset(ALCloopback *self);
static ALCboolean ALCloopback_start(ALCloopback *self);
static void ALCloopback_stop(ALCloopback *self);
static DECLARE_FORWARD2(ALCloopback, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCloopback, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCloopback, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCloopback, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCloopback, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCloopback)
DEFINE_ALCBACKEND_VTABLE(ALCloopback);


static void ALCloopback_Construct(ALCloopback *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCloopback, ALCbackend, self);
}


static ALCenum ALCloopback_open(ALCloopback *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    alstr_copy_cstr(&device->DeviceName, name);
    return ALC_NO_ERROR;
}

static ALCboolean ALCloopback_reset(ALCloopback *self)
{
    SetDefaultWFXChannelOrder(STATIC_CAST(ALCbackend, self)->mDevice);
    return ALC_TRUE;
}

static ALCboolean ALCloopback_start(ALCloopback* UNUSED(self))
{
    return ALC_TRUE;
}

static void ALCloopback_stop(ALCloopback* UNUSED(self))
{
}


typedef struct ALCloopbackFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCloopbackFactory;
#define ALCNULLBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCloopbackFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCloopbackFactory_getFactory(void);
static ALCboolean ALCloopbackFactory_init(ALCloopbackFactory *self);
static DECLARE_FORWARD(ALCloopbackFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCloopbackFactory_querySupport(ALCloopbackFactory *self, ALCbackend_Type type);
static void ALCloopbackFactory_probe(ALCloopbackFactory *self, enum DevProbe type);
static ALCbackend* ALCloopbackFactory_createBackend(ALCloopbackFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCloopbackFactory);


ALCbackendFactory *ALCloopbackFactory_getFactory(void)
{
    static ALCloopbackFactory factory = ALCNULLBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}

static ALCboolean ALCloopbackFactory_init(ALCloopbackFactory* UNUSED(self))
{
    return ALC_TRUE;
}

static ALCboolean ALCloopbackFactory_querySupport(ALCloopbackFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Loopback)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCloopbackFactory_probe(ALCloopbackFactory* UNUSED(self), enum DevProbe UNUSED(type))
{
}

static ALCbackend* ALCloopbackFactory_createBackend(ALCloopbackFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Loopback)
    {
        ALCloopback *backend;
        NEW_OBJ(backend, ALCloopback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
