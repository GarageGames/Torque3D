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

#include <dsound.h>
#include <cguid.h>
#include <mmreg.h>
#ifndef _WAVEFORMATEXTENSIBLE_
#include <ks.h>
#include <ksmedia.h>
#endif

#include "alMain.h"
#include "alu.h"
#include "ringbuffer.h"
#include "threads.h"
#include "compat.h"
#include "alstring.h"

#include "backends/base.h"

#ifndef DSSPEAKER_5POINT1
#   define DSSPEAKER_5POINT1          0x00000006
#endif
#ifndef DSSPEAKER_5POINT1_BACK
#   define DSSPEAKER_5POINT1_BACK     0x00000006
#endif
#ifndef DSSPEAKER_7POINT1
#   define DSSPEAKER_7POINT1          0x00000007
#endif
#ifndef DSSPEAKER_7POINT1_SURROUND
#   define DSSPEAKER_7POINT1_SURROUND 0x00000008
#endif
#ifndef DSSPEAKER_5POINT1_SURROUND
#   define DSSPEAKER_5POINT1_SURROUND 0x00000009
#endif


DEFINE_GUID(KSDATAFORMAT_SUBTYPE_PCM, 0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 0x00000003, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define DEVNAME_HEAD "OpenAL Soft on "


#ifdef HAVE_DYNLOAD
static void *ds_handle;
static HRESULT (WINAPI *pDirectSoundCreate)(const GUID *pcGuidDevice, IDirectSound **ppDS, IUnknown *pUnkOuter);
static HRESULT (WINAPI *pDirectSoundEnumerateW)(LPDSENUMCALLBACKW pDSEnumCallback, void *pContext);
static HRESULT (WINAPI *pDirectSoundCaptureCreate)(const GUID *pcGuidDevice, IDirectSoundCapture **ppDSC, IUnknown *pUnkOuter);
static HRESULT (WINAPI *pDirectSoundCaptureEnumerateW)(LPDSENUMCALLBACKW pDSEnumCallback, void *pContext);

#define DirectSoundCreate            pDirectSoundCreate
#define DirectSoundEnumerateW        pDirectSoundEnumerateW
#define DirectSoundCaptureCreate     pDirectSoundCaptureCreate
#define DirectSoundCaptureEnumerateW pDirectSoundCaptureEnumerateW
#endif


static ALCboolean DSoundLoad(void)
{
#ifdef HAVE_DYNLOAD
    if(!ds_handle)
    {
        ds_handle = LoadLib("dsound.dll");
        if(ds_handle == NULL)
        {
            ERR("Failed to load dsound.dll\n");
            return ALC_FALSE;
        }

#define LOAD_FUNC(f) do {                                                     \
    p##f = GetSymbol(ds_handle, #f);                                          \
    if(p##f == NULL) {                                                        \
        CloseLib(ds_handle);                                                  \
        ds_handle = NULL;                                                     \
        return ALC_FALSE;                                                     \
    }                                                                         \
} while(0)
        LOAD_FUNC(DirectSoundCreate);
        LOAD_FUNC(DirectSoundEnumerateW);
        LOAD_FUNC(DirectSoundCaptureCreate);
        LOAD_FUNC(DirectSoundCaptureEnumerateW);
#undef LOAD_FUNC
    }
#endif
    return ALC_TRUE;
}


#define MAX_UPDATES 128

typedef struct {
    al_string name;
    GUID guid;
} DevMap;
TYPEDEF_VECTOR(DevMap, vector_DevMap)

static vector_DevMap PlaybackDevices;
static vector_DevMap CaptureDevices;

static void clear_devlist(vector_DevMap *list)
{
#define DEINIT_STR(i) AL_STRING_DEINIT((i)->name)
    VECTOR_FOR_EACH(DevMap, *list, DEINIT_STR);
    VECTOR_RESIZE(*list, 0, 0);
#undef DEINIT_STR
}

static BOOL CALLBACK DSoundEnumDevices(GUID *guid, const WCHAR *desc, const WCHAR* UNUSED(drvname), void *data)
{
    vector_DevMap *devices = data;
    OLECHAR *guidstr = NULL;
    DevMap entry;
    HRESULT hr;
    int count;

    if(!guid)
        return TRUE;

    AL_STRING_INIT(entry.name);

    count = 0;
    while(1)
    {
        const DevMap *iter;

        alstr_copy_cstr(&entry.name, DEVNAME_HEAD);
        alstr_append_wcstr(&entry.name, desc);
        if(count != 0)
        {
            char str[64];
            snprintf(str, sizeof(str), " #%d", count+1);
            alstr_append_cstr(&entry.name, str);
        }

#define MATCH_ENTRY(i) (alstr_cmp(entry.name, (i)->name) == 0)
        VECTOR_FIND_IF(iter, const DevMap, *devices, MATCH_ENTRY);
        if(iter == VECTOR_END(*devices)) break;
#undef MATCH_ENTRY
        count++;
    }
    entry.guid = *guid;

    hr = StringFromCLSID(guid, &guidstr);
    if(SUCCEEDED(hr))
    {
        TRACE("Got device \"%s\", GUID \"%ls\"\n", alstr_get_cstr(entry.name), guidstr);
        CoTaskMemFree(guidstr);
    }

    VECTOR_PUSH_BACK(*devices, entry);

    return TRUE;
}


typedef struct ALCdsoundPlayback {
    DERIVE_FROM_TYPE(ALCbackend);

    IDirectSound       *DS;
    IDirectSoundBuffer *PrimaryBuffer;
    IDirectSoundBuffer *Buffer;
    IDirectSoundNotify *Notifies;
    HANDLE             NotifyEvent;

    ATOMIC(ALenum) killNow;
    althrd_t thread;
} ALCdsoundPlayback;

static int ALCdsoundPlayback_mixerProc(void *ptr);

static void ALCdsoundPlayback_Construct(ALCdsoundPlayback *self, ALCdevice *device);
static void ALCdsoundPlayback_Destruct(ALCdsoundPlayback *self);
static ALCenum ALCdsoundPlayback_open(ALCdsoundPlayback *self, const ALCchar *name);
static ALCboolean ALCdsoundPlayback_reset(ALCdsoundPlayback *self);
static ALCboolean ALCdsoundPlayback_start(ALCdsoundPlayback *self);
static void ALCdsoundPlayback_stop(ALCdsoundPlayback *self);
static DECLARE_FORWARD2(ALCdsoundPlayback, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCdsoundPlayback, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCdsoundPlayback, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCdsoundPlayback, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCdsoundPlayback, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCdsoundPlayback)

DEFINE_ALCBACKEND_VTABLE(ALCdsoundPlayback);


static void ALCdsoundPlayback_Construct(ALCdsoundPlayback *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCdsoundPlayback, ALCbackend, self);

    self->DS = NULL;
    self->PrimaryBuffer = NULL;
    self->Buffer = NULL;
    self->Notifies = NULL;
    self->NotifyEvent = NULL;
    ATOMIC_INIT(&self->killNow, AL_TRUE);
}

static void ALCdsoundPlayback_Destruct(ALCdsoundPlayback *self)
{
    if(self->Notifies)
        IDirectSoundNotify_Release(self->Notifies);
    self->Notifies = NULL;
    if(self->Buffer)
        IDirectSoundBuffer_Release(self->Buffer);
    self->Buffer = NULL;
    if(self->PrimaryBuffer != NULL)
        IDirectSoundBuffer_Release(self->PrimaryBuffer);
    self->PrimaryBuffer = NULL;

    if(self->DS)
        IDirectSound_Release(self->DS);
    self->DS = NULL;
    if(self->NotifyEvent)
        CloseHandle(self->NotifyEvent);
    self->NotifyEvent = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


FORCE_ALIGN static int ALCdsoundPlayback_mixerProc(void *ptr)
{
    ALCdsoundPlayback *self = ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    DSBCAPS DSBCaps;
    DWORD LastCursor = 0;
    DWORD PlayCursor;
    void *WritePtr1, *WritePtr2;
    DWORD WriteCnt1,  WriteCnt2;
    BOOL Playing = FALSE;
    DWORD FrameSize;
    DWORD FragSize;
    DWORD avail;
    HRESULT err;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    memset(&DSBCaps, 0, sizeof(DSBCaps));
    DSBCaps.dwSize = sizeof(DSBCaps);
    err = IDirectSoundBuffer_GetCaps(self->Buffer, &DSBCaps);
    if(FAILED(err))
    {
        ERR("Failed to get buffer caps: 0x%lx\n", err);
        ALCdevice_Lock(device);
        aluHandleDisconnect(device, "Failure retrieving playback buffer info: 0x%lx", err);
        ALCdevice_Unlock(device);
        return 1;
    }

    FrameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);
    FragSize = device->UpdateSize * FrameSize;

    IDirectSoundBuffer_GetCurrentPosition(self->Buffer, &LastCursor, NULL);
    while(!ATOMIC_LOAD(&self->killNow, almemory_order_acquire) &&
          ATOMIC_LOAD(&device->Connected, almemory_order_acquire))
    {
        // Get current play cursor
        IDirectSoundBuffer_GetCurrentPosition(self->Buffer, &PlayCursor, NULL);
        avail = (PlayCursor-LastCursor+DSBCaps.dwBufferBytes) % DSBCaps.dwBufferBytes;

        if(avail < FragSize)
        {
            if(!Playing)
            {
                err = IDirectSoundBuffer_Play(self->Buffer, 0, 0, DSBPLAY_LOOPING);
                if(FAILED(err))
                {
                    ERR("Failed to play buffer: 0x%lx\n", err);
                    ALCdevice_Lock(device);
                    aluHandleDisconnect(device, "Failure starting playback: 0x%lx", err);
                    ALCdevice_Unlock(device);
                    return 1;
                }
                Playing = TRUE;
            }

            avail = WaitForSingleObjectEx(self->NotifyEvent, 2000, FALSE);
            if(avail != WAIT_OBJECT_0)
                ERR("WaitForSingleObjectEx error: 0x%lx\n", avail);
            continue;
        }
        avail -= avail%FragSize;

        // Lock output buffer
        WriteCnt1 = 0;
        WriteCnt2 = 0;
        err = IDirectSoundBuffer_Lock(self->Buffer, LastCursor, avail, &WritePtr1, &WriteCnt1, &WritePtr2, &WriteCnt2, 0);

        // If the buffer is lost, restore it and lock
        if(err == DSERR_BUFFERLOST)
        {
            WARN("Buffer lost, restoring...\n");
            err = IDirectSoundBuffer_Restore(self->Buffer);
            if(SUCCEEDED(err))
            {
                Playing = FALSE;
                LastCursor = 0;
                err = IDirectSoundBuffer_Lock(self->Buffer, 0, DSBCaps.dwBufferBytes, &WritePtr1, &WriteCnt1, &WritePtr2, &WriteCnt2, 0);
            }
        }

        // Successfully locked the output buffer
        if(SUCCEEDED(err))
        {
            // If we have an active context, mix data directly into output buffer otherwise fill with silence
            ALCdevice_Lock(device);
            aluMixData(device, WritePtr1, WriteCnt1/FrameSize);
            aluMixData(device, WritePtr2, WriteCnt2/FrameSize);
            ALCdevice_Unlock(device);

            // Unlock output buffer only when successfully locked
            IDirectSoundBuffer_Unlock(self->Buffer, WritePtr1, WriteCnt1, WritePtr2, WriteCnt2);
        }
        else
        {
            ERR("Buffer lock error: %#lx\n", err);
            ALCdevice_Lock(device);
            aluHandleDisconnect(device, "Failed to lock output buffer: 0x%lx", err);
            ALCdevice_Unlock(device);
            return 1;
        }

        // Update old write cursor location
        LastCursor += WriteCnt1+WriteCnt2;
        LastCursor %= DSBCaps.dwBufferBytes;
    }

    return 0;
}

static ALCenum ALCdsoundPlayback_open(ALCdsoundPlayback *self, const ALCchar *deviceName)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const GUID *guid = NULL;
    HRESULT hr, hrcom;

    if(VECTOR_SIZE(PlaybackDevices) == 0)
    {
        /* Initialize COM to prevent name truncation */
        hrcom = CoInitialize(NULL);
        hr = DirectSoundEnumerateW(DSoundEnumDevices, &PlaybackDevices);
        if(FAILED(hr))
            ERR("Error enumerating DirectSound devices (0x%lx)!\n", hr);
        if(SUCCEEDED(hrcom))
            CoUninitialize();
    }

    if(!deviceName && VECTOR_SIZE(PlaybackDevices) > 0)
    {
        deviceName = alstr_get_cstr(VECTOR_FRONT(PlaybackDevices).name);
        guid = &VECTOR_FRONT(PlaybackDevices).guid;
    }
    else
    {
        const DevMap *iter;

#define MATCH_NAME(i)  (alstr_cmp_cstr((i)->name, deviceName) == 0)
        VECTOR_FIND_IF(iter, const DevMap, PlaybackDevices, MATCH_NAME);
#undef MATCH_NAME
        if(iter == VECTOR_END(PlaybackDevices))
            return ALC_INVALID_VALUE;
        guid = &iter->guid;
    }

    hr = DS_OK;
    self->NotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if(self->NotifyEvent == NULL)
        hr = E_FAIL;

    //DirectSound Init code
    if(SUCCEEDED(hr))
        hr = DirectSoundCreate(guid, &self->DS, NULL);
    if(SUCCEEDED(hr))
        hr = IDirectSound_SetCooperativeLevel(self->DS, GetForegroundWindow(), DSSCL_PRIORITY);
    if(FAILED(hr))
    {
        if(self->DS)
            IDirectSound_Release(self->DS);
        self->DS = NULL;
        if(self->NotifyEvent)
            CloseHandle(self->NotifyEvent);
        self->NotifyEvent = NULL;

        ERR("Device init failed: 0x%08lx\n", hr);
        return ALC_INVALID_VALUE;
    }

    alstr_copy_cstr(&device->DeviceName, deviceName);

    return ALC_NO_ERROR;
}

static ALCboolean ALCdsoundPlayback_reset(ALCdsoundPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    DSBUFFERDESC DSBDescription;
    WAVEFORMATEXTENSIBLE OutputType;
    DWORD speakers;
    HRESULT hr;

    memset(&OutputType, 0, sizeof(OutputType));

    if(self->Notifies)
        IDirectSoundNotify_Release(self->Notifies);
    self->Notifies = NULL;
    if(self->Buffer)
        IDirectSoundBuffer_Release(self->Buffer);
    self->Buffer = NULL;
    if(self->PrimaryBuffer != NULL)
        IDirectSoundBuffer_Release(self->PrimaryBuffer);
    self->PrimaryBuffer = NULL;

    switch(device->FmtType)
    {
        case DevFmtByte:
            device->FmtType = DevFmtUByte;
            break;
        case DevFmtFloat:
            if((device->Flags&DEVICE_SAMPLE_TYPE_REQUEST))
                break;
            /* fall-through */
        case DevFmtUShort:
            device->FmtType = DevFmtShort;
            break;
        case DevFmtUInt:
            device->FmtType = DevFmtInt;
            break;
        case DevFmtUByte:
        case DevFmtShort:
        case DevFmtInt:
            break;
    }

    hr = IDirectSound_GetSpeakerConfig(self->DS, &speakers);
    if(SUCCEEDED(hr))
    {
        speakers = DSSPEAKER_CONFIG(speakers);
        if(!(device->Flags&DEVICE_CHANNELS_REQUEST))
        {
            if(speakers == DSSPEAKER_MONO)
                device->FmtChans = DevFmtMono;
            else if(speakers == DSSPEAKER_STEREO || speakers == DSSPEAKER_HEADPHONE)
                device->FmtChans = DevFmtStereo;
            else if(speakers == DSSPEAKER_QUAD)
                device->FmtChans = DevFmtQuad;
            else if(speakers == DSSPEAKER_5POINT1_SURROUND)
                device->FmtChans = DevFmtX51;
            else if(speakers == DSSPEAKER_5POINT1_BACK)
                device->FmtChans = DevFmtX51Rear;
            else if(speakers == DSSPEAKER_7POINT1 || speakers == DSSPEAKER_7POINT1_SURROUND)
                device->FmtChans = DevFmtX71;
            else
                ERR("Unknown system speaker config: 0x%lx\n", speakers);
        }
        device->IsHeadphones = (device->FmtChans == DevFmtStereo &&
                                speakers == DSSPEAKER_HEADPHONE);

        switch(device->FmtChans)
        {
            case DevFmtMono:
                OutputType.dwChannelMask = SPEAKER_FRONT_CENTER;
                break;
            case DevFmtAmbi3D:
                device->FmtChans = DevFmtStereo;
                /*fall-through*/
            case DevFmtStereo:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT;
                break;
            case DevFmtQuad:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT |
                                           SPEAKER_BACK_LEFT |
                                           SPEAKER_BACK_RIGHT;
                break;
            case DevFmtX51:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT |
                                           SPEAKER_FRONT_CENTER |
                                           SPEAKER_LOW_FREQUENCY |
                                           SPEAKER_SIDE_LEFT |
                                           SPEAKER_SIDE_RIGHT;
                break;
            case DevFmtX51Rear:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT |
                                           SPEAKER_FRONT_CENTER |
                                           SPEAKER_LOW_FREQUENCY |
                                           SPEAKER_BACK_LEFT |
                                           SPEAKER_BACK_RIGHT;
                break;
            case DevFmtX61:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT |
                                           SPEAKER_FRONT_CENTER |
                                           SPEAKER_LOW_FREQUENCY |
                                           SPEAKER_BACK_CENTER |
                                           SPEAKER_SIDE_LEFT |
                                           SPEAKER_SIDE_RIGHT;
                break;
            case DevFmtX71:
                OutputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                           SPEAKER_FRONT_RIGHT |
                                           SPEAKER_FRONT_CENTER |
                                           SPEAKER_LOW_FREQUENCY |
                                           SPEAKER_BACK_LEFT |
                                           SPEAKER_BACK_RIGHT |
                                           SPEAKER_SIDE_LEFT |
                                           SPEAKER_SIDE_RIGHT;
                break;
        }

retry_open:
        hr = S_OK;
        OutputType.Format.wFormatTag = WAVE_FORMAT_PCM;
        OutputType.Format.nChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);
        OutputType.Format.wBitsPerSample = BytesFromDevFmt(device->FmtType) * 8;
        OutputType.Format.nBlockAlign = OutputType.Format.nChannels*OutputType.Format.wBitsPerSample/8;
        OutputType.Format.nSamplesPerSec = device->Frequency;
        OutputType.Format.nAvgBytesPerSec = OutputType.Format.nSamplesPerSec*OutputType.Format.nBlockAlign;
        OutputType.Format.cbSize = 0;
    }

    if(OutputType.Format.nChannels > 2 || device->FmtType == DevFmtFloat)
    {
        OutputType.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        OutputType.Samples.wValidBitsPerSample = OutputType.Format.wBitsPerSample;
        OutputType.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        if(device->FmtType == DevFmtFloat)
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        else
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

        if(self->PrimaryBuffer)
            IDirectSoundBuffer_Release(self->PrimaryBuffer);
        self->PrimaryBuffer = NULL;
    }
    else
    {
        if(SUCCEEDED(hr) && !self->PrimaryBuffer)
        {
            memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
            DSBDescription.dwSize=sizeof(DSBUFFERDESC);
            DSBDescription.dwFlags=DSBCAPS_PRIMARYBUFFER;
            hr = IDirectSound_CreateSoundBuffer(self->DS, &DSBDescription, &self->PrimaryBuffer, NULL);
        }
        if(SUCCEEDED(hr))
            hr = IDirectSoundBuffer_SetFormat(self->PrimaryBuffer,&OutputType.Format);
    }

    if(SUCCEEDED(hr))
    {
        if(device->NumUpdates > MAX_UPDATES)
        {
            device->UpdateSize = (device->UpdateSize*device->NumUpdates +
                                  MAX_UPDATES-1) / MAX_UPDATES;
            device->NumUpdates = MAX_UPDATES;
        }

        memset(&DSBDescription,0,sizeof(DSBUFFERDESC));
        DSBDescription.dwSize=sizeof(DSBUFFERDESC);
        DSBDescription.dwFlags=DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
        DSBDescription.dwBufferBytes=device->UpdateSize * device->NumUpdates *
                                     OutputType.Format.nBlockAlign;
        DSBDescription.lpwfxFormat=&OutputType.Format;
        hr = IDirectSound_CreateSoundBuffer(self->DS, &DSBDescription, &self->Buffer, NULL);
        if(FAILED(hr) && device->FmtType == DevFmtFloat)
        {
            device->FmtType = DevFmtShort;
            goto retry_open;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = IDirectSoundBuffer_QueryInterface(self->Buffer, &IID_IDirectSoundNotify, (void**)&self->Notifies);
        if(SUCCEEDED(hr))
        {
            DSBPOSITIONNOTIFY notifies[MAX_UPDATES];
            ALuint i;

            for(i = 0;i < device->NumUpdates;++i)
            {
                notifies[i].dwOffset = i * device->UpdateSize *
                                       OutputType.Format.nBlockAlign;
                notifies[i].hEventNotify = self->NotifyEvent;
            }
            if(IDirectSoundNotify_SetNotificationPositions(self->Notifies, device->NumUpdates, notifies) != DS_OK)
                hr = E_FAIL;
        }
    }

    if(FAILED(hr))
    {
        if(self->Notifies != NULL)
            IDirectSoundNotify_Release(self->Notifies);
        self->Notifies = NULL;
        if(self->Buffer != NULL)
            IDirectSoundBuffer_Release(self->Buffer);
        self->Buffer = NULL;
        if(self->PrimaryBuffer != NULL)
            IDirectSoundBuffer_Release(self->PrimaryBuffer);
        self->PrimaryBuffer = NULL;
        return ALC_FALSE;
    }

    ResetEvent(self->NotifyEvent);
    SetDefaultWFXChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCdsoundPlayback_start(ALCdsoundPlayback *self)
{
    ATOMIC_STORE(&self->killNow, AL_FALSE, almemory_order_release);
    if(althrd_create(&self->thread, ALCdsoundPlayback_mixerProc, self) != althrd_success)
        return ALC_FALSE;

    return ALC_TRUE;
}

static void ALCdsoundPlayback_stop(ALCdsoundPlayback *self)
{
    int res;

    if(ATOMIC_EXCHANGE(&self->killNow, AL_TRUE, almemory_order_acq_rel))
        return;
    althrd_join(self->thread, &res);

    IDirectSoundBuffer_Stop(self->Buffer);
}



typedef struct ALCdsoundCapture {
    DERIVE_FROM_TYPE(ALCbackend);

    IDirectSoundCapture *DSC;
    IDirectSoundCaptureBuffer *DSCbuffer;
    DWORD BufferBytes;
    DWORD Cursor;

    ll_ringbuffer_t *Ring;
} ALCdsoundCapture;

static void ALCdsoundCapture_Construct(ALCdsoundCapture *self, ALCdevice *device);
static void ALCdsoundCapture_Destruct(ALCdsoundCapture *self);
static ALCenum ALCdsoundCapture_open(ALCdsoundCapture *self, const ALCchar *name);
static DECLARE_FORWARD(ALCdsoundCapture, ALCbackend, ALCboolean, reset)
static ALCboolean ALCdsoundCapture_start(ALCdsoundCapture *self);
static void ALCdsoundCapture_stop(ALCdsoundCapture *self);
static ALCenum ALCdsoundCapture_captureSamples(ALCdsoundCapture *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCdsoundCapture_availableSamples(ALCdsoundCapture *self);
static DECLARE_FORWARD(ALCdsoundCapture, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCdsoundCapture, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCdsoundCapture, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCdsoundCapture)

DEFINE_ALCBACKEND_VTABLE(ALCdsoundCapture);

static void ALCdsoundCapture_Construct(ALCdsoundCapture *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCdsoundCapture, ALCbackend, self);

    self->DSC = NULL;
    self->DSCbuffer = NULL;
    self->Ring = NULL;
}

static void ALCdsoundCapture_Destruct(ALCdsoundCapture *self)
{
    ll_ringbuffer_free(self->Ring);
    self->Ring = NULL;

    if(self->DSCbuffer != NULL)
    {
        IDirectSoundCaptureBuffer_Stop(self->DSCbuffer);
        IDirectSoundCaptureBuffer_Release(self->DSCbuffer);
        self->DSCbuffer = NULL;
    }

    if(self->DSC)
        IDirectSoundCapture_Release(self->DSC);
    self->DSC = NULL;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static ALCenum ALCdsoundCapture_open(ALCdsoundCapture *self, const ALCchar *deviceName)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    WAVEFORMATEXTENSIBLE InputType;
    DSCBUFFERDESC DSCBDescription;
    const GUID *guid = NULL;
    HRESULT hr, hrcom;
    ALuint samples;

    if(VECTOR_SIZE(CaptureDevices) == 0)
    {
        /* Initialize COM to prevent name truncation */
        hrcom = CoInitialize(NULL);
        hr = DirectSoundCaptureEnumerateW(DSoundEnumDevices, &CaptureDevices);
        if(FAILED(hr))
            ERR("Error enumerating DirectSound devices (0x%lx)!\n", hr);
        if(SUCCEEDED(hrcom))
            CoUninitialize();
    }

    if(!deviceName && VECTOR_SIZE(CaptureDevices) > 0)
    {
        deviceName = alstr_get_cstr(VECTOR_FRONT(CaptureDevices).name);
        guid = &VECTOR_FRONT(CaptureDevices).guid;
    }
    else
    {
        const DevMap *iter;

#define MATCH_NAME(i)  (alstr_cmp_cstr((i)->name, deviceName) == 0)
        VECTOR_FIND_IF(iter, const DevMap, CaptureDevices, MATCH_NAME);
#undef MATCH_NAME
        if(iter == VECTOR_END(CaptureDevices))
            return ALC_INVALID_VALUE;
        guid = &iter->guid;
    }

    switch(device->FmtType)
    {
        case DevFmtByte:
        case DevFmtUShort:
        case DevFmtUInt:
            WARN("%s capture samples not supported\n", DevFmtTypeString(device->FmtType));
            return ALC_INVALID_ENUM;

        case DevFmtUByte:
        case DevFmtShort:
        case DevFmtInt:
        case DevFmtFloat:
            break;
    }

    memset(&InputType, 0, sizeof(InputType));
    switch(device->FmtChans)
    {
        case DevFmtMono:
            InputType.dwChannelMask = SPEAKER_FRONT_CENTER;
            break;
        case DevFmtStereo:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT;
            break;
        case DevFmtQuad:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT |
                                      SPEAKER_BACK_LEFT |
                                      SPEAKER_BACK_RIGHT;
            break;
        case DevFmtX51:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT |
                                      SPEAKER_FRONT_CENTER |
                                      SPEAKER_LOW_FREQUENCY |
                                      SPEAKER_SIDE_LEFT |
                                      SPEAKER_SIDE_RIGHT;
            break;
        case DevFmtX51Rear:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT |
                                      SPEAKER_FRONT_CENTER |
                                      SPEAKER_LOW_FREQUENCY |
                                      SPEAKER_BACK_LEFT |
                                      SPEAKER_BACK_RIGHT;
            break;
        case DevFmtX61:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT |
                                      SPEAKER_FRONT_CENTER |
                                      SPEAKER_LOW_FREQUENCY |
                                      SPEAKER_BACK_CENTER |
                                      SPEAKER_SIDE_LEFT |
                                      SPEAKER_SIDE_RIGHT;
            break;
        case DevFmtX71:
            InputType.dwChannelMask = SPEAKER_FRONT_LEFT |
                                      SPEAKER_FRONT_RIGHT |
                                      SPEAKER_FRONT_CENTER |
                                      SPEAKER_LOW_FREQUENCY |
                                      SPEAKER_BACK_LEFT |
                                      SPEAKER_BACK_RIGHT |
                                      SPEAKER_SIDE_LEFT |
                                      SPEAKER_SIDE_RIGHT;
            break;
        case DevFmtAmbi3D:
            WARN("%s capture not supported\n", DevFmtChannelsString(device->FmtChans));
            return ALC_INVALID_ENUM;
    }

    InputType.Format.wFormatTag = WAVE_FORMAT_PCM;
    InputType.Format.nChannels = ChannelsFromDevFmt(device->FmtChans, device->AmbiOrder);
    InputType.Format.wBitsPerSample = BytesFromDevFmt(device->FmtType) * 8;
    InputType.Format.nBlockAlign = InputType.Format.nChannels*InputType.Format.wBitsPerSample/8;
    InputType.Format.nSamplesPerSec = device->Frequency;
    InputType.Format.nAvgBytesPerSec = InputType.Format.nSamplesPerSec*InputType.Format.nBlockAlign;
    InputType.Format.cbSize = 0;
    InputType.Samples.wValidBitsPerSample = InputType.Format.wBitsPerSample;
    if(device->FmtType == DevFmtFloat)
        InputType.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    else
        InputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    if(InputType.Format.nChannels > 2 || device->FmtType == DevFmtFloat)
    {
        InputType.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        InputType.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
    }

    samples = device->UpdateSize * device->NumUpdates;
    samples = maxu(samples, 100 * device->Frequency / 1000);

    memset(&DSCBDescription, 0, sizeof(DSCBUFFERDESC));
    DSCBDescription.dwSize = sizeof(DSCBUFFERDESC);
    DSCBDescription.dwFlags = 0;
    DSCBDescription.dwBufferBytes = samples * InputType.Format.nBlockAlign;
    DSCBDescription.lpwfxFormat = &InputType.Format;

    //DirectSoundCapture Init code
    hr = DirectSoundCaptureCreate(guid, &self->DSC, NULL);
    if(SUCCEEDED(hr))
        hr = IDirectSoundCapture_CreateCaptureBuffer(self->DSC, &DSCBDescription, &self->DSCbuffer, NULL);
    if(SUCCEEDED(hr))
    {
         self->Ring = ll_ringbuffer_create(device->UpdateSize*device->NumUpdates,
                                           InputType.Format.nBlockAlign, false);
         if(self->Ring == NULL)
             hr = DSERR_OUTOFMEMORY;
    }

    if(FAILED(hr))
    {
        ERR("Device init failed: 0x%08lx\n", hr);

        ll_ringbuffer_free(self->Ring);
        self->Ring = NULL;
        if(self->DSCbuffer != NULL)
            IDirectSoundCaptureBuffer_Release(self->DSCbuffer);
        self->DSCbuffer = NULL;
        if(self->DSC)
            IDirectSoundCapture_Release(self->DSC);
        self->DSC = NULL;

        return ALC_INVALID_VALUE;
    }

    self->BufferBytes = DSCBDescription.dwBufferBytes;
    SetDefaultWFXChannelOrder(device);

    alstr_copy_cstr(&device->DeviceName, deviceName);

    return ALC_NO_ERROR;
}

static ALCboolean ALCdsoundCapture_start(ALCdsoundCapture *self)
{
    HRESULT hr;

    hr = IDirectSoundCaptureBuffer_Start(self->DSCbuffer, DSCBSTART_LOOPING);
    if(FAILED(hr))
    {
        ERR("start failed: 0x%08lx\n", hr);
        aluHandleDisconnect(STATIC_CAST(ALCbackend, self)->mDevice,
                            "Failure starting capture: 0x%lx", hr);
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCdsoundCapture_stop(ALCdsoundCapture *self)
{
    HRESULT hr;

    hr = IDirectSoundCaptureBuffer_Stop(self->DSCbuffer);
    if(FAILED(hr))
    {
        ERR("stop failed: 0x%08lx\n", hr);
        aluHandleDisconnect(STATIC_CAST(ALCbackend, self)->mDevice,
                            "Failure stopping capture: 0x%lx", hr);
    }
}

static ALCenum ALCdsoundCapture_captureSamples(ALCdsoundCapture *self, ALCvoid *buffer, ALCuint samples)
{
    ll_ringbuffer_read(self->Ring, buffer, samples);
    return ALC_NO_ERROR;
}

static ALCuint ALCdsoundCapture_availableSamples(ALCdsoundCapture *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    DWORD ReadCursor, LastCursor, BufferBytes, NumBytes;
    void *ReadPtr1, *ReadPtr2;
    DWORD ReadCnt1,  ReadCnt2;
    DWORD FrameSize;
    HRESULT hr;

    if(!ATOMIC_LOAD(&device->Connected, almemory_order_acquire))
        goto done;

    FrameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);
    BufferBytes = self->BufferBytes;
    LastCursor = self->Cursor;

    hr = IDirectSoundCaptureBuffer_GetCurrentPosition(self->DSCbuffer, NULL, &ReadCursor);
    if(SUCCEEDED(hr))
    {
        NumBytes = (ReadCursor-LastCursor + BufferBytes) % BufferBytes;
        if(NumBytes == 0)
            goto done;
        hr = IDirectSoundCaptureBuffer_Lock(self->DSCbuffer, LastCursor, NumBytes,
                                            &ReadPtr1, &ReadCnt1,
                                            &ReadPtr2, &ReadCnt2, 0);
    }
    if(SUCCEEDED(hr))
    {
        ll_ringbuffer_write(self->Ring, ReadPtr1, ReadCnt1/FrameSize);
        if(ReadPtr2 != NULL)
            ll_ringbuffer_write(self->Ring, ReadPtr2, ReadCnt2/FrameSize);
        hr = IDirectSoundCaptureBuffer_Unlock(self->DSCbuffer,
                                              ReadPtr1, ReadCnt1,
                                              ReadPtr2, ReadCnt2);
        self->Cursor = (LastCursor+ReadCnt1+ReadCnt2) % BufferBytes;
    }

    if(FAILED(hr))
    {
        ERR("update failed: 0x%08lx\n", hr);
        aluHandleDisconnect(device, "Failure retrieving capture data: 0x%lx", hr);
    }

done:
    return (ALCuint)ll_ringbuffer_read_space(self->Ring);
}


static inline void AppendAllDevicesList2(const DevMap *entry)
{ AppendAllDevicesList(alstr_get_cstr(entry->name)); }
static inline void AppendCaptureDeviceList2(const DevMap *entry)
{ AppendCaptureDeviceList(alstr_get_cstr(entry->name)); }

typedef struct ALCdsoundBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCdsoundBackendFactory;
#define ALCDSOUNDBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCdsoundBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCdsoundBackendFactory_getFactory(void);

static ALCboolean ALCdsoundBackendFactory_init(ALCdsoundBackendFactory *self);
static void ALCdsoundBackendFactory_deinit(ALCdsoundBackendFactory *self);
static ALCboolean ALCdsoundBackendFactory_querySupport(ALCdsoundBackendFactory *self, ALCbackend_Type type);
static void ALCdsoundBackendFactory_probe(ALCdsoundBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCdsoundBackendFactory_createBackend(ALCdsoundBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCdsoundBackendFactory);


ALCbackendFactory *ALCdsoundBackendFactory_getFactory(void)
{
    static ALCdsoundBackendFactory factory = ALCDSOUNDBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCdsoundBackendFactory_init(ALCdsoundBackendFactory* UNUSED(self))
{
    VECTOR_INIT(PlaybackDevices);
    VECTOR_INIT(CaptureDevices);

    if(!DSoundLoad())
        return ALC_FALSE;
    return ALC_TRUE;
}

static void ALCdsoundBackendFactory_deinit(ALCdsoundBackendFactory* UNUSED(self))
{
    clear_devlist(&PlaybackDevices);
    VECTOR_DEINIT(PlaybackDevices);

    clear_devlist(&CaptureDevices);
    VECTOR_DEINIT(CaptureDevices);

#ifdef HAVE_DYNLOAD
    if(ds_handle)
        CloseLib(ds_handle);
    ds_handle = NULL;
#endif
}

static ALCboolean ALCdsoundBackendFactory_querySupport(ALCdsoundBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || type == ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCdsoundBackendFactory_probe(ALCdsoundBackendFactory* UNUSED(self), enum DevProbe type)
{
    HRESULT hr, hrcom;

    /* Initialize COM to prevent name truncation */
    hrcom = CoInitialize(NULL);
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            clear_devlist(&PlaybackDevices);
            hr = DirectSoundEnumerateW(DSoundEnumDevices, &PlaybackDevices);
            if(FAILED(hr))
                ERR("Error enumerating DirectSound playback devices (0x%lx)!\n", hr);
            VECTOR_FOR_EACH(const DevMap, PlaybackDevices, AppendAllDevicesList2);
            break;

        case CAPTURE_DEVICE_PROBE:
            clear_devlist(&CaptureDevices);
            hr = DirectSoundCaptureEnumerateW(DSoundEnumDevices, &CaptureDevices);
            if(FAILED(hr))
                ERR("Error enumerating DirectSound capture devices (0x%lx)!\n", hr);
            VECTOR_FOR_EACH(const DevMap, CaptureDevices, AppendCaptureDeviceList2);
            break;
    }
    if(SUCCEEDED(hrcom))
        CoUninitialize();
}

static ALCbackend* ALCdsoundBackendFactory_createBackend(ALCdsoundBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCdsoundPlayback *backend;
        NEW_OBJ(backend, ALCdsoundPlayback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    if(type == ALCbackend_Capture)
    {
        ALCdsoundCapture *backend;
        NEW_OBJ(backend, ALCdsoundCapture)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
