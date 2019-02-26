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

#include <windows.h>
#include <mmsystem.h>

#include "alMain.h"
#include "alu.h"
#include "ringbuffer.h"
#include "threads.h"

#include "backends/base.h"

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT  0x0003
#endif

#define DEVNAME_HEAD "OpenAL Soft on "


static vector_al_string PlaybackDevices;
static vector_al_string CaptureDevices;

static void clear_devlist(vector_al_string *list)
{
    VECTOR_FOR_EACH(al_string, *list, alstr_reset);
    VECTOR_RESIZE(*list, 0, 0);
}


static void ProbePlaybackDevices(void)
{
    ALuint numdevs;
    ALuint i;

    clear_devlist(&PlaybackDevices);

    numdevs = waveOutGetNumDevs();
    VECTOR_RESIZE(PlaybackDevices, 0, numdevs);
    for(i = 0;i < numdevs;i++)
    {
        WAVEOUTCAPSW WaveCaps;
        const al_string *iter;
        al_string dname;

        AL_STRING_INIT(dname);
        if(waveOutGetDevCapsW(i, &WaveCaps, sizeof(WaveCaps)) == MMSYSERR_NOERROR)
        {
            ALuint count = 0;
            while(1)
            {
                alstr_copy_cstr(&dname, DEVNAME_HEAD);
                alstr_append_wcstr(&dname, WaveCaps.szPname);
                if(count != 0)
                {
                    char str[64];
                    snprintf(str, sizeof(str), " #%d", count+1);
                    alstr_append_cstr(&dname, str);
                }
                count++;

#define MATCH_ENTRY(i) (alstr_cmp(dname, *(i)) == 0)
                VECTOR_FIND_IF(iter, const al_string, PlaybackDevices, MATCH_ENTRY);
                if(iter == VECTOR_END(PlaybackDevices)) break;
#undef MATCH_ENTRY
            }

            TRACE("Got device \"%s\", ID %u\n", alstr_get_cstr(dname), i);
        }
        VECTOR_PUSH_BACK(PlaybackDevices, dname);
    }
}

static void ProbeCaptureDevices(void)
{
    ALuint numdevs;
    ALuint i;

    clear_devlist(&CaptureDevices);

    numdevs = waveInGetNumDevs();
    VECTOR_RESIZE(CaptureDevices, 0, numdevs);
    for(i = 0;i < numdevs;i++)
    {
        WAVEINCAPSW WaveCaps;
        const al_string *iter;
        al_string dname;

        AL_STRING_INIT(dname);
        if(waveInGetDevCapsW(i, &WaveCaps, sizeof(WaveCaps)) == MMSYSERR_NOERROR)
        {
            ALuint count = 0;
            while(1)
            {
                alstr_copy_cstr(&dname, DEVNAME_HEAD);
                alstr_append_wcstr(&dname, WaveCaps.szPname);
                if(count != 0)
                {
                    char str[64];
                    snprintf(str, sizeof(str), " #%d", count+1);
                    alstr_append_cstr(&dname, str);
                }
                count++;

#define MATCH_ENTRY(i) (alstr_cmp(dname, *(i)) == 0)
                VECTOR_FIND_IF(iter, const al_string, CaptureDevices, MATCH_ENTRY);
                if(iter == VECTOR_END(CaptureDevices)) break;
#undef MATCH_ENTRY
            }

            TRACE("Got device \"%s\", ID %u\n", alstr_get_cstr(dname), i);
        }
        VECTOR_PUSH_BACK(CaptureDevices, dname);
    }
}


typedef struct ALCwinmmPlayback {
    DERIVE_FROM_TYPE(ALCbackend);

    RefCount WaveBuffersCommitted;
    WAVEHDR WaveBuffer[4];

    HWAVEOUT OutHdl;

    WAVEFORMATEX Format;

    ATOMIC(ALenum) killNow;
    althrd_t thread;
} ALCwinmmPlayback;

static void ALCwinmmPlayback_Construct(ALCwinmmPlayback *self, ALCdevice *device);
static void ALCwinmmPlayback_Destruct(ALCwinmmPlayback *self);

static void CALLBACK ALCwinmmPlayback_waveOutProc(HWAVEOUT device, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);
static int ALCwinmmPlayback_mixerProc(void *arg);

static ALCenum ALCwinmmPlayback_open(ALCwinmmPlayback *self, const ALCchar *name);
static ALCboolean ALCwinmmPlayback_reset(ALCwinmmPlayback *self);
static ALCboolean ALCwinmmPlayback_start(ALCwinmmPlayback *self);
static void ALCwinmmPlayback_stop(ALCwinmmPlayback *self);
static DECLARE_FORWARD2(ALCwinmmPlayback, ALCbackend, ALCenum, captureSamples, ALCvoid*, ALCuint)
static DECLARE_FORWARD(ALCwinmmPlayback, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCwinmmPlayback, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCwinmmPlayback, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCwinmmPlayback, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCwinmmPlayback)

DEFINE_ALCBACKEND_VTABLE(ALCwinmmPlayback);


static void ALCwinmmPlayback_Construct(ALCwinmmPlayback *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCwinmmPlayback, ALCbackend, self);

    InitRef(&self->WaveBuffersCommitted, 0);
    self->OutHdl = NULL;

    ATOMIC_INIT(&self->killNow, AL_TRUE);
}

static void ALCwinmmPlayback_Destruct(ALCwinmmPlayback *self)
{
    if(self->OutHdl)
        waveOutClose(self->OutHdl);
    self->OutHdl = 0;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


/* ALCwinmmPlayback_waveOutProc
 *
 * Posts a message to 'ALCwinmmPlayback_mixerProc' everytime a WaveOut Buffer
 * is completed and returns to the application (for more data)
 */
static void CALLBACK ALCwinmmPlayback_waveOutProc(HWAVEOUT UNUSED(device), UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR UNUSED(param2))
{
    ALCwinmmPlayback *self = (ALCwinmmPlayback*)instance;

    if(msg != WOM_DONE)
        return;

    DecrementRef(&self->WaveBuffersCommitted);
    PostThreadMessage(self->thread, msg, 0, param1);
}

FORCE_ALIGN static int ALCwinmmPlayback_mixerProc(void *arg)
{
    ALCwinmmPlayback *self = arg;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    WAVEHDR *WaveHdr;
    MSG msg;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(msg.message != WOM_DONE)
            continue;

        if(ATOMIC_LOAD(&self->killNow, almemory_order_acquire))
        {
            if(ReadRef(&self->WaveBuffersCommitted) == 0)
                break;
            continue;
        }

        WaveHdr = ((WAVEHDR*)msg.lParam);
        ALCwinmmPlayback_lock(self);
        aluMixData(device, WaveHdr->lpData, WaveHdr->dwBufferLength /
                                            self->Format.nBlockAlign);
        ALCwinmmPlayback_unlock(self);

        // Send buffer back to play more data
        waveOutWrite(self->OutHdl, WaveHdr, sizeof(WAVEHDR));
        IncrementRef(&self->WaveBuffersCommitted);
    }

    return 0;
}


static ALCenum ALCwinmmPlayback_open(ALCwinmmPlayback *self, const ALCchar *deviceName)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const al_string *iter;
    UINT DeviceID;
    MMRESULT res;

    if(VECTOR_SIZE(PlaybackDevices) == 0)
        ProbePlaybackDevices();

    // Find the Device ID matching the deviceName if valid
#define MATCH_DEVNAME(iter) (!alstr_empty(*(iter)) && \
                             (!deviceName || alstr_cmp_cstr(*(iter), deviceName) == 0))
    VECTOR_FIND_IF(iter, const al_string, PlaybackDevices, MATCH_DEVNAME);
    if(iter == VECTOR_END(PlaybackDevices))
        return ALC_INVALID_VALUE;
#undef MATCH_DEVNAME

    DeviceID = (UINT)(iter - VECTOR_BEGIN(PlaybackDevices));

retry_open:
    memset(&self->Format, 0, sizeof(WAVEFORMATEX));
    if(device->FmtType == DevFmtFloat)
    {
        self->Format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        self->Format.wBitsPerSample = 32;
    }
    else
    {
        self->Format.wFormatTag = WAVE_FORMAT_PCM;
        if(device->FmtType == DevFmtUByte || device->FmtType == DevFmtByte)
            self->Format.wBitsPerSample = 8;
        else
            self->Format.wBitsPerSample = 16;
    }
    self->Format.nChannels = ((device->FmtChans == DevFmtMono) ? 1 : 2);
    self->Format.nBlockAlign = self->Format.wBitsPerSample *
                               self->Format.nChannels / 8;
    self->Format.nSamplesPerSec = device->Frequency;
    self->Format.nAvgBytesPerSec = self->Format.nSamplesPerSec *
                                   self->Format.nBlockAlign;
    self->Format.cbSize = 0;

    if((res=waveOutOpen(&self->OutHdl, DeviceID, &self->Format, (DWORD_PTR)&ALCwinmmPlayback_waveOutProc, (DWORD_PTR)self, CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
    {
        if(device->FmtType == DevFmtFloat)
        {
            device->FmtType = DevFmtShort;
            goto retry_open;
        }
        ERR("waveOutOpen failed: %u\n", res);
        goto failure;
    }

    alstr_copy(&device->DeviceName, VECTOR_ELEM(PlaybackDevices, DeviceID));
    return ALC_NO_ERROR;

failure:
    if(self->OutHdl)
        waveOutClose(self->OutHdl);
    self->OutHdl = NULL;

    return ALC_INVALID_VALUE;
}

static ALCboolean ALCwinmmPlayback_reset(ALCwinmmPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    device->UpdateSize = (ALuint)((ALuint64)device->UpdateSize *
                                  self->Format.nSamplesPerSec /
                                  device->Frequency);
    device->UpdateSize = (device->UpdateSize*device->NumUpdates + 3) / 4;
    device->NumUpdates = 4;
    device->Frequency = self->Format.nSamplesPerSec;

    if(self->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        if(self->Format.wBitsPerSample == 32)
            device->FmtType = DevFmtFloat;
        else
        {
            ERR("Unhandled IEEE float sample depth: %d\n", self->Format.wBitsPerSample);
            return ALC_FALSE;
        }
    }
    else if(self->Format.wFormatTag == WAVE_FORMAT_PCM)
    {
        if(self->Format.wBitsPerSample == 16)
            device->FmtType = DevFmtShort;
        else if(self->Format.wBitsPerSample == 8)
            device->FmtType = DevFmtUByte;
        else
        {
            ERR("Unhandled PCM sample depth: %d\n", self->Format.wBitsPerSample);
            return ALC_FALSE;
        }
    }
    else
    {
        ERR("Unhandled format tag: 0x%04x\n", self->Format.wFormatTag);
        return ALC_FALSE;
    }

    if(self->Format.nChannels == 2)
        device->FmtChans = DevFmtStereo;
    else if(self->Format.nChannels == 1)
        device->FmtChans = DevFmtMono;
    else
    {
        ERR("Unhandled channel count: %d\n", self->Format.nChannels);
        return ALC_FALSE;
    }
    SetDefaultWFXChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCwinmmPlayback_start(ALCwinmmPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALbyte *BufferData;
    ALint BufferSize;
    ALuint i;

    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCwinmmPlayback_mixerProc, self) != althrd_success)
        return ALC_FALSE;

    InitRef(&self->WaveBuffersCommitted, 0);

    // Create 4 Buffers
    BufferSize  = device->UpdateSize*device->NumUpdates / 4;
    BufferSize *= FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);

    BufferData = calloc(4, BufferSize);
    for(i = 0;i < 4;i++)
    {
        memset(&self->WaveBuffer[i], 0, sizeof(WAVEHDR));
        self->WaveBuffer[i].dwBufferLength = BufferSize;
        self->WaveBuffer[i].lpData = ((i==0) ? (CHAR*)BufferData :
                                      (self->WaveBuffer[i-1].lpData +
                                       self->WaveBuffer[i-1].dwBufferLength));
        waveOutPrepareHeader(self->OutHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        waveOutWrite(self->OutHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        IncrementRef(&self->WaveBuffersCommitted);
    }

    return ALC_TRUE;
}

static void ALCwinmmPlayback_stop(ALCwinmmPlayback *self)
{
    void *buffer = NULL;
    int i;

    if(ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
        return;
    althrd_join(self->thread, &i);

    // Release the wave buffers
    for(i = 0;i < 4;i++)
    {
        waveOutUnprepareHeader(self->OutHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        if(i == 0) buffer = self->WaveBuffer[i].lpData;
        self->WaveBuffer[i].lpData = NULL;
    }
    free(buffer);
}



typedef struct ALCwinmmCapture {
    DERIVE_FROM_TYPE(ALCbackend);

    RefCount WaveBuffersCommitted;
    WAVEHDR WaveBuffer[4];

    HWAVEIN InHdl;

    ll_ringbuffer_t *Ring;

    WAVEFORMATEX Format;

    ATOMIC(ALenum) killNow;
    althrd_t thread;
} ALCwinmmCapture;

static void ALCwinmmCapture_Construct(ALCwinmmCapture *self, ALCdevice *device);
static void ALCwinmmCapture_Destruct(ALCwinmmCapture *self);

static void CALLBACK ALCwinmmCapture_waveInProc(HWAVEIN device, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);
static int ALCwinmmCapture_captureProc(void *arg);

static ALCenum ALCwinmmCapture_open(ALCwinmmCapture *self, const ALCchar *name);
static DECLARE_FORWARD(ALCwinmmCapture, ALCbackend, ALCboolean, reset)
static ALCboolean ALCwinmmCapture_start(ALCwinmmCapture *self);
static void ALCwinmmCapture_stop(ALCwinmmCapture *self);
static ALCenum ALCwinmmCapture_captureSamples(ALCwinmmCapture *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCwinmmCapture_availableSamples(ALCwinmmCapture *self);
static DECLARE_FORWARD(ALCwinmmCapture, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCwinmmCapture, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCwinmmCapture, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCwinmmCapture)

DEFINE_ALCBACKEND_VTABLE(ALCwinmmCapture);


static void ALCwinmmCapture_Construct(ALCwinmmCapture *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCwinmmCapture, ALCbackend, self);

    InitRef(&self->WaveBuffersCommitted, 0);
    self->InHdl = NULL;

    ATOMIC_INIT(&self->killNow, AL_TRUE);
}

static void ALCwinmmCapture_Destruct(ALCwinmmCapture *self)
{
    void *buffer = NULL;
    int i;

    /* Tell the processing thread to quit and wait for it to do so. */
    if(!ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
    {
        PostThreadMessage(self->thread, WM_QUIT, 0, 0);

        althrd_join(self->thread, &i);

        /* Make sure capture is stopped and all pending buffers are flushed. */
        waveInReset(self->InHdl);

        // Release the wave buffers
        for(i = 0;i < 4;i++)
        {
            waveInUnprepareHeader(self->InHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
            if(i == 0) buffer = self->WaveBuffer[i].lpData;
            self->WaveBuffer[i].lpData = NULL;
        }
        free(buffer);
    }

    ll_ringbuffer_free(self->Ring);
    self->Ring = NULL;

    // Close the Wave device
    if(self->InHdl)
        waveInClose(self->InHdl);
    self->InHdl = 0;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


/* ALCwinmmCapture_waveInProc
 *
 * Posts a message to 'ALCwinmmCapture_captureProc' everytime a WaveIn Buffer
 * is completed and returns to the application (with more data).
 */
static void CALLBACK ALCwinmmCapture_waveInProc(HWAVEIN UNUSED(device), UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR UNUSED(param2))
{
    ALCwinmmCapture *self = (ALCwinmmCapture*)instance;

    if(msg != WIM_DATA)
        return;

    DecrementRef(&self->WaveBuffersCommitted);
    PostThreadMessage(self->thread, msg, 0, param1);
}

static int ALCwinmmCapture_captureProc(void *arg)
{
    ALCwinmmCapture *self = arg;
    WAVEHDR *WaveHdr;
    MSG msg;

    althrd_setname(althrd_current(), RECORD_THREAD_NAME);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(msg.message != WIM_DATA)
            continue;
        /* Don't wait for other buffers to finish before quitting. We're
         * closing so we don't need them. */
        if(ATOMIC_LOAD(&self->killNow, almemory_order_acquire))
            break;

        WaveHdr = ((WAVEHDR*)msg.lParam);
        ll_ringbuffer_write(self->Ring, WaveHdr->lpData,
            WaveHdr->dwBytesRecorded / self->Format.nBlockAlign
        );

        // Send buffer back to capture more data
        waveInAddBuffer(self->InHdl, WaveHdr, sizeof(WAVEHDR));
        IncrementRef(&self->WaveBuffersCommitted);
    }

    return 0;
}


static ALCenum ALCwinmmCapture_open(ALCwinmmCapture *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const al_string *iter;
    ALbyte *BufferData = NULL;
    DWORD CapturedDataSize;
    ALint BufferSize;
    UINT DeviceID;
    MMRESULT res;
    ALuint i;

    if(VECTOR_SIZE(CaptureDevices) == 0)
        ProbeCaptureDevices();

    // Find the Device ID matching the deviceName if valid
#define MATCH_DEVNAME(iter) (!alstr_empty(*(iter)) && (!name || alstr_cmp_cstr(*iter, name) == 0))
    VECTOR_FIND_IF(iter, const al_string, CaptureDevices, MATCH_DEVNAME);
    if(iter == VECTOR_END(CaptureDevices))
        return ALC_INVALID_VALUE;
#undef MATCH_DEVNAME

    DeviceID = (UINT)(iter - VECTOR_BEGIN(CaptureDevices));

    switch(device->FmtChans)
    {
        case DevFmtMono:
        case DevFmtStereo:
            break;

        case DevFmtQuad:
        case DevFmtX51:
        case DevFmtX51Rear:
        case DevFmtX61:
        case DevFmtX71:
        case DevFmtAmbi3D:
            return ALC_INVALID_ENUM;
    }

    switch(device->FmtType)
    {
        case DevFmtUByte:
        case DevFmtShort:
        case DevFmtInt:
        case DevFmtFloat:
            break;

        case DevFmtByte:
        case DevFmtUShort:
        case DevFmtUInt:
            return ALC_INVALID_ENUM;
    }

    memset(&self->Format, 0, sizeof(WAVEFORMATEX));
    self->Format.wFormatTag = ((device->FmtType == DevFmtFloat) ?
                               WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
    self->Format.nChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);
    self->Format.wBitsPerSample = BytesFromDevFmt(device->FmtType) * 8;
    self->Format.nBlockAlign = self->Format.wBitsPerSample *
                               self->Format.nChannels / 8;
    self->Format.nSamplesPerSec = device->Frequency;
    self->Format.nAvgBytesPerSec = self->Format.nSamplesPerSec *
                                   self->Format.nBlockAlign;
    self->Format.cbSize = 0;

    if((res=waveInOpen(&self->InHdl, DeviceID, &self->Format, (DWORD_PTR)&ALCwinmmCapture_waveInProc, (DWORD_PTR)self, CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
    {
        ERR("waveInOpen failed: %u\n", res);
        goto failure;
    }

    // Allocate circular memory buffer for the captured audio
    CapturedDataSize = device->UpdateSize*device->NumUpdates;

    // Make sure circular buffer is at least 100ms in size
    if(CapturedDataSize < (self->Format.nSamplesPerSec / 10))
        CapturedDataSize = self->Format.nSamplesPerSec / 10;

    self->Ring = ll_ringbuffer_create(CapturedDataSize, self->Format.nBlockAlign, false);
    if(!self->Ring) goto failure;

    InitRef(&self->WaveBuffersCommitted, 0);

    // Create 4 Buffers of 50ms each
    BufferSize = self->Format.nAvgBytesPerSec / 20;
    BufferSize -= (BufferSize % self->Format.nBlockAlign);

    BufferData = calloc(4, BufferSize);
    if(!BufferData) goto failure;

    for(i = 0;i < 4;i++)
    {
        memset(&self->WaveBuffer[i], 0, sizeof(WAVEHDR));
        self->WaveBuffer[i].dwBufferLength = BufferSize;
        self->WaveBuffer[i].lpData = ((i==0) ? (CHAR*)BufferData :
                                      (self->WaveBuffer[i-1].lpData +
                                       self->WaveBuffer[i-1].dwBufferLength));
        self->WaveBuffer[i].dwFlags = 0;
        self->WaveBuffer[i].dwLoops = 0;
        waveInPrepareHeader(self->InHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        waveInAddBuffer(self->InHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        IncrementRef(&self->WaveBuffersCommitted);
    }

    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCwinmmCapture_captureProc, self) != althrd_success)
        goto failure;

    alstr_copy(&device->DeviceName, VECTOR_ELEM(CaptureDevices, DeviceID));
    return ALC_NO_ERROR;

failure:
    if(BufferData)
    {
        for(i = 0;i < 4;i++)
            waveInUnprepareHeader(self->InHdl, &self->WaveBuffer[i], sizeof(WAVEHDR));
        free(BufferData);
    }

    ll_ringbuffer_free(self->Ring);
    self->Ring = NULL;

    if(self->InHdl)
        waveInClose(self->InHdl);
    self->InHdl = NULL;

    return ALC_INVALID_VALUE;
}

static ALCboolean ALCwinmmCapture_start(ALCwinmmCapture *self)
{
    waveInStart(self->InHdl);
    return ALC_TRUE;
}

static void ALCwinmmCapture_stop(ALCwinmmCapture *self)
{
    waveInStop(self->InHdl);
}

static ALCenum ALCwinmmCapture_captureSamples(ALCwinmmCapture *self, ALCvoid *buffer, ALCuint samples)
{
    ll_ringbuffer_read(self->Ring, buffer, samples);
    return ALC_NO_ERROR;
}

static ALCuint ALCwinmmCapture_availableSamples(ALCwinmmCapture *self)
{
    return (ALCuint)ll_ringbuffer_read_space(self->Ring);
}


static inline void AppendAllDevicesList2(const al_string *name)
{
    if(!alstr_empty(*name))
        AppendAllDevicesList(alstr_get_cstr(*name));
}
static inline void AppendCaptureDeviceList2(const al_string *name)
{
    if(!alstr_empty(*name))
        AppendCaptureDeviceList(alstr_get_cstr(*name));
}

typedef struct ALCwinmmBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCwinmmBackendFactory;
#define ALCWINMMBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCwinmmBackendFactory, ALCbackendFactory) } }

static ALCboolean ALCwinmmBackendFactory_init(ALCwinmmBackendFactory *self);
static void ALCwinmmBackendFactory_deinit(ALCwinmmBackendFactory *self);
static ALCboolean ALCwinmmBackendFactory_querySupport(ALCwinmmBackendFactory *self, ALCbackend_Type type);
static void ALCwinmmBackendFactory_probe(ALCwinmmBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCwinmmBackendFactory_createBackend(ALCwinmmBackendFactory *self, ALCdevice *device, ALCbackend_Type type);

DEFINE_ALCBACKENDFACTORY_VTABLE(ALCwinmmBackendFactory);


static ALCboolean ALCwinmmBackendFactory_init(ALCwinmmBackendFactory* UNUSED(self))
{
    VECTOR_INIT(PlaybackDevices);
    VECTOR_INIT(CaptureDevices);

    return ALC_TRUE;
}

static void ALCwinmmBackendFactory_deinit(ALCwinmmBackendFactory* UNUSED(self))
{
    clear_devlist(&PlaybackDevices);
    VECTOR_DEINIT(PlaybackDevices);

    clear_devlist(&CaptureDevices);
    VECTOR_DEINIT(CaptureDevices);
}

static ALCboolean ALCwinmmBackendFactory_querySupport(ALCwinmmBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || type == ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCwinmmBackendFactory_probe(ALCwinmmBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            ProbePlaybackDevices();
            VECTOR_FOR_EACH(const al_string, PlaybackDevices, AppendAllDevicesList2);
            break;

        case CAPTURE_DEVICE_PROBE:
            ProbeCaptureDevices();
            VECTOR_FOR_EACH(const al_string, CaptureDevices, AppendCaptureDeviceList2);
            break;
    }
}

static ALCbackend* ALCwinmmBackendFactory_createBackend(ALCwinmmBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCwinmmPlayback *backend;
        NEW_OBJ(backend, ALCwinmmPlayback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }
    if(type == ALCbackend_Capture)
    {
        ALCwinmmCapture *backend;
        NEW_OBJ(backend, ALCwinmmCapture)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}

ALCbackendFactory *ALCwinmmBackendFactory_getFactory(void)
{
    static ALCwinmmBackendFactory factory = ALCWINMMBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}
