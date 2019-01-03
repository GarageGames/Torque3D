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
#include "alconfig.h"
#include "ringbuffer.h"
#include "compat.h"

#include "backends/base.h"

#include <portaudio.h>


static const ALCchar pa_device[] = "PortAudio Default";


#ifdef HAVE_DYNLOAD
static void *pa_handle;
#define MAKE_FUNC(x) static __typeof(x) * p##x
MAKE_FUNC(Pa_Initialize);
MAKE_FUNC(Pa_Terminate);
MAKE_FUNC(Pa_GetErrorText);
MAKE_FUNC(Pa_StartStream);
MAKE_FUNC(Pa_StopStream);
MAKE_FUNC(Pa_OpenStream);
MAKE_FUNC(Pa_CloseStream);
MAKE_FUNC(Pa_GetDefaultOutputDevice);
MAKE_FUNC(Pa_GetDefaultInputDevice);
MAKE_FUNC(Pa_GetStreamInfo);
#undef MAKE_FUNC

#define Pa_Initialize                  pPa_Initialize
#define Pa_Terminate                   pPa_Terminate
#define Pa_GetErrorText                pPa_GetErrorText
#define Pa_StartStream                 pPa_StartStream
#define Pa_StopStream                  pPa_StopStream
#define Pa_OpenStream                  pPa_OpenStream
#define Pa_CloseStream                 pPa_CloseStream
#define Pa_GetDefaultOutputDevice      pPa_GetDefaultOutputDevice
#define Pa_GetDefaultInputDevice       pPa_GetDefaultInputDevice
#define Pa_GetStreamInfo               pPa_GetStreamInfo
#endif

static ALCboolean pa_load(void)
{
    PaError err;

#ifdef HAVE_DYNLOAD
    if(!pa_handle)
    {
#ifdef _WIN32
# define PALIB "portaudio.dll"
#elif defined(__APPLE__) && defined(__MACH__)
# define PALIB "libportaudio.2.dylib"
#elif defined(__OpenBSD__)
# define PALIB "libportaudio.so"
#else
# define PALIB "libportaudio.so.2"
#endif

        pa_handle = LoadLib(PALIB);
        if(!pa_handle)
            return ALC_FALSE;

#define LOAD_FUNC(f) do {                                                     \
    p##f = GetSymbol(pa_handle, #f);                                          \
    if(p##f == NULL)                                                          \
    {                                                                         \
        CloseLib(pa_handle);                                                  \
        pa_handle = NULL;                                                     \
        return ALC_FALSE;                                                     \
    }                                                                         \
} while(0)
        LOAD_FUNC(Pa_Initialize);
        LOAD_FUNC(Pa_Terminate);
        LOAD_FUNC(Pa_GetErrorText);
        LOAD_FUNC(Pa_StartStream);
        LOAD_FUNC(Pa_StopStream);
        LOAD_FUNC(Pa_OpenStream);
        LOAD_FUNC(Pa_CloseStream);
        LOAD_FUNC(Pa_GetDefaultOutputDevice);
        LOAD_FUNC(Pa_GetDefaultInputDevice);
        LOAD_FUNC(Pa_GetStreamInfo);
#undef LOAD_FUNC

        if((err=Pa_Initialize()) != paNoError)
        {
            ERR("Pa_Initialize() returned an error: %s\n", Pa_GetErrorText(err));
            CloseLib(pa_handle);
            pa_handle = NULL;
            return ALC_FALSE;
        }
    }
#else
    if((err=Pa_Initialize()) != paNoError)
    {
        ERR("Pa_Initialize() returned an error: %s\n", Pa_GetErrorText(err));
        return ALC_FALSE;
    }
#endif
    return ALC_TRUE;
}


typedef struct ALCportPlayback {
    DERIVE_FROM_TYPE(ALCbackend);

    PaStream *stream;
    PaStreamParameters params;
    ALuint update_size;
} ALCportPlayback;

static int ALCportPlayback_WriteCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo,
    const PaStreamCallbackFlags statusFlags, void *userData);

static void ALCportPlayback_Construct(ALCportPlayback *self, ALCdevice *device);
static void ALCportPlayback_Destruct(ALCportPlayback *self);
static ALCenum ALCportPlayback_open(ALCportPlayback *self, const ALCchar *name);
static ALCboolean ALCportPlayback_reset(ALCportPlayback *self);
static ALCboolean ALCportPlayback_start(ALCportPlayback *self);
static void ALCportPlayback_stop(ALCportPlayback *self);
static DECLARE_FORWARD2(ALCportPlayback, ALCbackend, ALCenum, captureSamples, ALCvoid*, ALCuint)
static DECLARE_FORWARD(ALCportPlayback, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCportPlayback, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCportPlayback, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCportPlayback, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCportPlayback)

DEFINE_ALCBACKEND_VTABLE(ALCportPlayback);


static void ALCportPlayback_Construct(ALCportPlayback *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCportPlayback, ALCbackend, self);

    self->stream = NULL;
}

static void ALCportPlayback_Destruct(ALCportPlayback *self)
{
    PaError err = self->stream ? Pa_CloseStream(self->stream) : paNoError;
    if(err != paNoError)
        ERR("Error closing stream: %s\n", Pa_GetErrorText(err));
    self->stream = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static int ALCportPlayback_WriteCallback(const void *UNUSED(inputBuffer), void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *UNUSED(timeInfo),
    const PaStreamCallbackFlags UNUSED(statusFlags), void *userData)
{
    ALCportPlayback *self = userData;

    ALCportPlayback_lock(self);
    aluMixData(STATIC_CAST(ALCbackend, self)->mDevice, outputBuffer, framesPerBuffer);
    ALCportPlayback_unlock(self);
    return 0;
}


static ALCenum ALCportPlayback_open(ALCportPlayback *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    PaError err;

    if(!name)
        name = pa_device;
    else if(strcmp(name, pa_device) != 0)
        return ALC_INVALID_VALUE;

    self->update_size = device->UpdateSize;

    self->params.device = -1;
    if(!ConfigValueInt(NULL, "port", "device", &self->params.device) ||
       self->params.device < 0)
        self->params.device = Pa_GetDefaultOutputDevice();
    self->params.suggestedLatency = (device->UpdateSize*device->NumUpdates) /
                                    (float)device->Frequency;
    self->params.hostApiSpecificStreamInfo = NULL;

    self->params.channelCount = ((device->FmtChans == DevFmtMono) ? 1 : 2);

    switch(device->FmtType)
    {
        case DevFmtByte:
            self->params.sampleFormat = paInt8;
            break;
        case DevFmtUByte:
            self->params.sampleFormat = paUInt8;
            break;
        case DevFmtUShort:
            /* fall-through */
        case DevFmtShort:
            self->params.sampleFormat = paInt16;
            break;
        case DevFmtUInt:
            /* fall-through */
        case DevFmtInt:
            self->params.sampleFormat = paInt32;
            break;
        case DevFmtFloat:
            self->params.sampleFormat = paFloat32;
            break;
    }

retry_open:
    err = Pa_OpenStream(&self->stream, NULL, &self->params,
        device->Frequency, device->UpdateSize, paNoFlag,
        ALCportPlayback_WriteCallback, self
    );
    if(err != paNoError)
    {
        if(self->params.sampleFormat == paFloat32)
        {
            self->params.sampleFormat = paInt16;
            goto retry_open;
        }
        ERR("Pa_OpenStream() returned an error: %s\n", Pa_GetErrorText(err));
        return ALC_INVALID_VALUE;
    }

    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;

}

static ALCboolean ALCportPlayback_reset(ALCportPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const PaStreamInfo *streamInfo;

    streamInfo = Pa_GetStreamInfo(self->stream);
    device->Frequency = streamInfo->sampleRate;
    device->UpdateSize = self->update_size;

    if(self->params.sampleFormat == paInt8)
        device->FmtType = DevFmtByte;
    else if(self->params.sampleFormat == paUInt8)
        device->FmtType = DevFmtUByte;
    else if(self->params.sampleFormat == paInt16)
        device->FmtType = DevFmtShort;
    else if(self->params.sampleFormat == paInt32)
        device->FmtType = DevFmtInt;
    else if(self->params.sampleFormat == paFloat32)
        device->FmtType = DevFmtFloat;
    else
    {
        ERR("Unexpected sample format: 0x%lx\n", self->params.sampleFormat);
        return ALC_FALSE;
    }

    if(self->params.channelCount == 2)
        device->FmtChans = DevFmtStereo;
    else if(self->params.channelCount == 1)
        device->FmtChans = DevFmtMono;
    else
    {
        ERR("Unexpected channel count: %u\n", self->params.channelCount);
        return ALC_FALSE;
    }
    SetDefaultChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCportPlayback_start(ALCportPlayback *self)
{
    PaError err;

    err = Pa_StartStream(self->stream);
    if(err != paNoError)
    {
        ERR("Pa_StartStream() returned an error: %s\n", Pa_GetErrorText(err));
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCportPlayback_stop(ALCportPlayback *self)
{
    PaError err = Pa_StopStream(self->stream);
    if(err != paNoError)
        ERR("Error stopping stream: %s\n", Pa_GetErrorText(err));
}


typedef struct ALCportCapture {
    DERIVE_FROM_TYPE(ALCbackend);

    PaStream *stream;
    PaStreamParameters params;

    ll_ringbuffer_t *ring;
} ALCportCapture;

static int ALCportCapture_ReadCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo,
    const PaStreamCallbackFlags statusFlags, void *userData);

static void ALCportCapture_Construct(ALCportCapture *self, ALCdevice *device);
static void ALCportCapture_Destruct(ALCportCapture *self);
static ALCenum ALCportCapture_open(ALCportCapture *self, const ALCchar *name);
static DECLARE_FORWARD(ALCportCapture, ALCbackend, ALCboolean, reset)
static ALCboolean ALCportCapture_start(ALCportCapture *self);
static void ALCportCapture_stop(ALCportCapture *self);
static ALCenum ALCportCapture_captureSamples(ALCportCapture *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCportCapture_availableSamples(ALCportCapture *self);
static DECLARE_FORWARD(ALCportCapture, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCportCapture, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCportCapture, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCportCapture)

DEFINE_ALCBACKEND_VTABLE(ALCportCapture);


static void ALCportCapture_Construct(ALCportCapture *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCportCapture, ALCbackend, self);

    self->stream = NULL;
    self->ring = NULL;
}

static void ALCportCapture_Destruct(ALCportCapture *self)
{
    PaError err = self->stream ? Pa_CloseStream(self->stream) : paNoError;
    if(err != paNoError)
        ERR("Error closing stream: %s\n", Pa_GetErrorText(err));
    self->stream = NULL;

    ll_ringbuffer_free(self->ring);
    self->ring = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static int ALCportCapture_ReadCallback(const void *inputBuffer, void *UNUSED(outputBuffer),
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *UNUSED(timeInfo),
    const PaStreamCallbackFlags UNUSED(statusFlags), void *userData)
{
    ALCportCapture *self = userData;
    size_t writable = ll_ringbuffer_write_space(self->ring);

    if(framesPerBuffer > writable)
        framesPerBuffer = writable;
    ll_ringbuffer_write(self->ring, inputBuffer, framesPerBuffer);
    return 0;
}


static ALCenum ALCportCapture_open(ALCportCapture *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALuint samples, frame_size;
    PaError err;

    if(!name)
        name = pa_device;
    else if(strcmp(name, pa_device) != 0)
        return ALC_INVALID_VALUE;

    samples = device->UpdateSize * device->NumUpdates;
    samples = maxu(samples, 100 * device->Frequency / 1000);
    frame_size = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);

    self->ring = ll_ringbuffer_create(samples, frame_size, false);
    if(self->ring == NULL) return ALC_INVALID_VALUE;

    self->params.device = -1;
    if(!ConfigValueInt(NULL, "port", "capture", &self->params.device) ||
       self->params.device < 0)
        self->params.device = Pa_GetDefaultInputDevice();
    self->params.suggestedLatency = 0.0f;
    self->params.hostApiSpecificStreamInfo = NULL;

    switch(device->FmtType)
    {
        case DevFmtByte:
            self->params.sampleFormat = paInt8;
            break;
        case DevFmtUByte:
            self->params.sampleFormat = paUInt8;
            break;
        case DevFmtShort:
            self->params.sampleFormat = paInt16;
            break;
        case DevFmtInt:
            self->params.sampleFormat = paInt32;
            break;
        case DevFmtFloat:
            self->params.sampleFormat = paFloat32;
            break;
        case DevFmtUInt:
        case DevFmtUShort:
            ERR("%s samples not supported\n", DevFmtTypeString(device->FmtType));
            return ALC_INVALID_VALUE;
    }
    self->params.channelCount = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);

    err = Pa_OpenStream(&self->stream, &self->params, NULL,
        device->Frequency, paFramesPerBufferUnspecified, paNoFlag,
        ALCportCapture_ReadCallback, self
    );
    if(err != paNoError)
    {
        ERR("Pa_OpenStream() returned an error: %s\n", Pa_GetErrorText(err));
        return ALC_INVALID_VALUE;
    }

    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}


static ALCboolean ALCportCapture_start(ALCportCapture *self)
{
    PaError err = Pa_StartStream(self->stream);
    if(err != paNoError)
    {
        ERR("Error starting stream: %s\n", Pa_GetErrorText(err));
        return ALC_FALSE;
    }
    return ALC_TRUE;
}

static void ALCportCapture_stop(ALCportCapture *self)
{
    PaError err = Pa_StopStream(self->stream);
    if(err != paNoError)
        ERR("Error stopping stream: %s\n", Pa_GetErrorText(err));
}


static ALCuint ALCportCapture_availableSamples(ALCportCapture *self)
{
    return ll_ringbuffer_read_space(self->ring);
}

static ALCenum ALCportCapture_captureSamples(ALCportCapture *self, ALCvoid *buffer, ALCuint samples)
{
    ll_ringbuffer_read(self->ring, buffer, samples);
    return ALC_NO_ERROR;
}


typedef struct ALCportBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCportBackendFactory;
#define ALCPORTBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCportBackendFactory, ALCbackendFactory) } }

static ALCboolean ALCportBackendFactory_init(ALCportBackendFactory *self);
static void ALCportBackendFactory_deinit(ALCportBackendFactory *self);
static ALCboolean ALCportBackendFactory_querySupport(ALCportBackendFactory *self, ALCbackend_Type type);
static void ALCportBackendFactory_probe(ALCportBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCportBackendFactory_createBackend(ALCportBackendFactory *self, ALCdevice *device, ALCbackend_Type type);

DEFINE_ALCBACKENDFACTORY_VTABLE(ALCportBackendFactory);


static ALCboolean ALCportBackendFactory_init(ALCportBackendFactory* UNUSED(self))
{
    if(!pa_load())
        return ALC_FALSE;
    return ALC_TRUE;
}

static void ALCportBackendFactory_deinit(ALCportBackendFactory* UNUSED(self))
{
#ifdef HAVE_DYNLOAD
    if(pa_handle)
    {
        Pa_Terminate();
        CloseLib(pa_handle);
        pa_handle = NULL;
    }
#else
    Pa_Terminate();
#endif
}

static ALCboolean ALCportBackendFactory_querySupport(ALCportBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || type == ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCportBackendFactory_probe(ALCportBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(pa_device);
            break;
        case CAPTURE_DEVICE_PROBE:
            AppendCaptureDeviceList(pa_device);
            break;
    }
}

static ALCbackend* ALCportBackendFactory_createBackend(ALCportBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCportPlayback *backend;
        NEW_OBJ(backend, ALCportPlayback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }
    if(type == ALCbackend_Capture)
    {
        ALCportCapture *backend;
        NEW_OBJ(backend, ALCportCapture)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}

ALCbackendFactory *ALCportBackendFactory_getFactory(void)
{
    static ALCportBackendFactory factory = ALCPORTBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}
