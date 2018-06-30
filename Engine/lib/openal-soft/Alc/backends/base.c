
#include "config.h"

#include <stdlib.h>

#include "alMain.h"
#include "alu.h"

#include "backends/base.h"


extern inline ALuint64 GetDeviceClockTime(ALCdevice *device);
extern inline void ALCdevice_Lock(ALCdevice *device);
extern inline void ALCdevice_Unlock(ALCdevice *device);

/* Base ALCbackend method implementations. */
void ALCbackend_Construct(ALCbackend *self, ALCdevice *device)
{
    int ret = almtx_init(&self->mMutex, almtx_recursive);
    assert(ret == althrd_success);
    self->mDevice = device;
}

void ALCbackend_Destruct(ALCbackend *self)
{
    almtx_destroy(&self->mMutex);
}

ALCboolean ALCbackend_reset(ALCbackend* UNUSED(self))
{
    return ALC_FALSE;
}

ALCenum ALCbackend_captureSamples(ALCbackend* UNUSED(self), void* UNUSED(buffer), ALCuint UNUSED(samples))
{
    return ALC_INVALID_DEVICE;
}

ALCuint ALCbackend_availableSamples(ALCbackend* UNUSED(self))
{
    return 0;
}

ClockLatency ALCbackend_getClockLatency(ALCbackend *self)
{
    ALCdevice *device = self->mDevice;
    ALuint refcount;
    ClockLatency ret;

    do {
        while(((refcount=ATOMIC_LOAD(&device->MixCount, almemory_order_acquire))&1))
            althrd_yield();
        ret.ClockTime = GetDeviceClockTime(device);
        ATOMIC_THREAD_FENCE(almemory_order_acquire);
    } while(refcount != ATOMIC_LOAD(&device->MixCount, almemory_order_relaxed));

    /* NOTE: The device will generally have about all but one periods filled at
     * any given time during playback. Without a more accurate measurement from
     * the output, this is an okay approximation.
     */
    ret.Latency = device->UpdateSize * DEVICE_CLOCK_RES / device->Frequency *
                  maxu(device->NumUpdates-1, 1);

    return ret;
}

void ALCbackend_lock(ALCbackend *self)
{
    int ret = almtx_lock(&self->mMutex);
    assert(ret == althrd_success);
}

void ALCbackend_unlock(ALCbackend *self)
{
    int ret = almtx_unlock(&self->mMutex);
    assert(ret == althrd_success);
}


/* Base ALCbackendFactory method implementations. */
void ALCbackendFactory_deinit(ALCbackendFactory* UNUSED(self))
{
}
