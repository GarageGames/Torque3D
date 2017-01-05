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
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"

#include <sys/soundcard.h>

/*
 * The OSS documentation talks about SOUND_MIXER_READ, but the header
 * only contains MIXER_READ. Play safe. Same for WRITE.
 */
#ifndef SOUND_MIXER_READ
#define SOUND_MIXER_READ MIXER_READ
#endif
#ifndef SOUND_MIXER_WRITE
#define SOUND_MIXER_WRITE MIXER_WRITE
#endif

#if defined(SOUND_VERSION) && (SOUND_VERSION < 0x040000)
#define ALC_OSS_COMPAT
#endif
#ifndef SNDCTL_AUDIOINFO
#define ALC_OSS_COMPAT
#endif

/*
 * FreeBSD strongly discourages the use of specific devices,
 * such as those returned in oss_audioinfo.devnode
 */
#ifdef __FreeBSD__
#define ALC_OSS_DEVNODE_TRUC
#endif

struct oss_device {
    const ALCchar *handle;
    const char *path;
    struct oss_device *next;
};

static struct oss_device oss_playback = {
    "OSS Default",
    "/dev/dsp",
    NULL
};

static struct oss_device oss_capture = {
    "OSS Default",
    "/dev/dsp",
    NULL
};

#ifdef ALC_OSS_COMPAT

static void ALCossListPopulate(struct oss_device *UNUSED(playback), struct oss_device *UNUSED(capture))
{
}

#else

#ifndef HAVE_STRNLEN
static size_t strnlen(const char *str, size_t maxlen)
{
    const char *end = memchr(str, 0, maxlen);
    if(!end) return maxlen;
    return end - str;
}
#endif

static void ALCossListAppend(struct oss_device *list, const char *handle, size_t hlen, const char *path, size_t plen)
{
    struct oss_device *next;
    struct oss_device *last;
    size_t i;

    /* skip the first item "OSS Default" */
    last = list;
    next = list->next;
#ifdef ALC_OSS_DEVNODE_TRUC
    for(i = 0;i < plen;i++)
    {
        if(path[i] == '.')
        {
            if(strncmp(path + i, handle + hlen + i - plen, plen - i) == 0)
                hlen = hlen + i - plen;
            plen = i;
        }
    }
#else
    (void)i;
#endif
    if(handle[0] == '\0')
    {
        handle = path;
        hlen = plen;
    }

    while(next != NULL)
    {
        if(strncmp(next->path, path, plen) == 0)
            return;
        last = next;
        next = next->next;
    }

    next = (struct oss_device*)malloc(sizeof(struct oss_device) + hlen + plen + 2);
    next->handle = (char*)(next + 1);
    next->path = next->handle + hlen + 1;
    next->next = NULL;
    last->next = next;

    strncpy((char*)next->handle, handle, hlen);
    ((char*)next->handle)[hlen] = '\0';
    strncpy((char*)next->path, path, plen);
    ((char*)next->path)[plen] = '\0';

    TRACE("Got device \"%s\", \"%s\"\n", next->handle, next->path);
}

static void ALCossListPopulate(struct oss_device *playback, struct oss_device *capture)
{
    struct oss_sysinfo si;
    struct oss_audioinfo ai;
    int fd, i;

    if((fd=open("/dev/mixer", O_RDONLY)) < 0)
    {
        ERR("Could not open /dev/mixer\n");
        return;
    }
    if(ioctl(fd, SNDCTL_SYSINFO, &si) == -1)
    {
        ERR("SNDCTL_SYSINFO failed: %s\n", strerror(errno));
        goto done;
    }
    for(i = 0;i < si.numaudios;i++)
    {
        const char *handle;
        size_t len;

        ai.dev = i;
        if(ioctl(fd, SNDCTL_AUDIOINFO, &ai) == -1)
        {
            ERR("SNDCTL_AUDIOINFO (%d) failed: %s\n", i, strerror(errno));
            continue;
        }
        if(ai.devnode[0] == '\0')
            continue;

        if(ai.handle[0] != '\0')
        {
            len = strnlen(ai.handle, sizeof(ai.handle));
            handle = ai.handle;
        }
        else
        {
            len = strnlen(ai.name, sizeof(ai.name));
            handle = ai.name;
        }
        if((ai.caps&DSP_CAP_INPUT) && capture != NULL)
            ALCossListAppend(capture, handle, len, ai.devnode, strnlen(ai.devnode, sizeof(ai.devnode)));
        if((ai.caps&DSP_CAP_OUTPUT) && playback != NULL)
            ALCossListAppend(playback, handle, len, ai.devnode, strnlen(ai.devnode, sizeof(ai.devnode)));
    }

done:
    close(fd);
}

#endif

static void ALCossListFree(struct oss_device *list)
{
    struct oss_device *cur;
    if(list == NULL)
        return;

    /* skip the first item "OSS Default" */
    cur = list->next;
    list->next = NULL;

    while(cur != NULL)
    {
        struct oss_device *next = cur->next;
        free(cur);
        cur = next;
    }
}

static int log2i(ALCuint x)
{
    int y = 0;
    while (x > 1)
    {
        x >>= 1;
        y++;
    }
    return y;
}

typedef struct ALCplaybackOSS {
    DERIVE_FROM_TYPE(ALCbackend);

    int fd;

    ALubyte *mix_data;
    int data_size;

    volatile int killNow;
    althrd_t thread;
} ALCplaybackOSS;

static int ALCplaybackOSS_mixerProc(void *ptr);

static void ALCplaybackOSS_Construct(ALCplaybackOSS *self, ALCdevice *device);
static DECLARE_FORWARD(ALCplaybackOSS, ALCbackend, void, Destruct)
static ALCenum ALCplaybackOSS_open(ALCplaybackOSS *self, const ALCchar *name);
static void ALCplaybackOSS_close(ALCplaybackOSS *self);
static ALCboolean ALCplaybackOSS_reset(ALCplaybackOSS *self);
static ALCboolean ALCplaybackOSS_start(ALCplaybackOSS *self);
static void ALCplaybackOSS_stop(ALCplaybackOSS *self);
static DECLARE_FORWARD2(ALCplaybackOSS, ALCbackend, ALCenum, captureSamples, ALCvoid*, ALCuint)
static DECLARE_FORWARD(ALCplaybackOSS, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCplaybackOSS, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCplaybackOSS, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCplaybackOSS, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCplaybackOSS)
DEFINE_ALCBACKEND_VTABLE(ALCplaybackOSS);


static int ALCplaybackOSS_mixerProc(void *ptr)
{
    ALCplaybackOSS *self = (ALCplaybackOSS*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    ALint frameSize;
    ssize_t wrote;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType);

    while(!self->killNow && device->Connected)
    {
        ALint len = self->data_size;
        ALubyte *WritePtr = self->mix_data;

        aluMixData(device, WritePtr, len/frameSize);
        while(len > 0 && !self->killNow)
        {
            wrote = write(self->fd, WritePtr, len);
            if(wrote < 0)
            {
                if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    ERR("write failed: %s\n", strerror(errno));
                    ALCplaybackOSS_lock(self);
                    aluHandleDisconnect(device);
                    ALCplaybackOSS_unlock(self);
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


static void ALCplaybackOSS_Construct(ALCplaybackOSS *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCplaybackOSS, ALCbackend, self);
}

static ALCenum ALCplaybackOSS_open(ALCplaybackOSS *self, const ALCchar *name)
{
    struct oss_device *dev = &oss_playback;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    if(!name)
        name = dev->handle;
    else
    {
        while (dev != NULL)
        {
            if (strcmp(dev->handle, name) == 0)
                break;
            dev = dev->next;
        }
        if (dev == NULL)
            return ALC_INVALID_VALUE;
    }

    self->killNow = 0;

    self->fd = open(dev->path, O_WRONLY);
    if(self->fd == -1)
    {
        ERR("Could not open %s: %s\n", dev->path, strerror(errno));
        return ALC_INVALID_VALUE;
    }

    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static void ALCplaybackOSS_close(ALCplaybackOSS *self)
{
    close(self->fd);
    self->fd = -1;
}

static ALCboolean ALCplaybackOSS_reset(ALCplaybackOSS *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    int numFragmentsLogSize;
    int log2FragmentSize;
    unsigned int periods;
    audio_buf_info info;
    ALuint frameSize;
    int numChannels;
    int ossFormat;
    int ossSpeed;
    char *err;

    switch(device->FmtType)
    {
        case DevFmtByte:
            ossFormat = AFMT_S8;
            break;
        case DevFmtUByte:
            ossFormat = AFMT_U8;
            break;
        case DevFmtUShort:
        case DevFmtInt:
        case DevFmtUInt:
        case DevFmtFloat:
            device->FmtType = DevFmtShort;
            /* fall-through */
        case DevFmtShort:
            ossFormat = AFMT_S16_NE;
            break;
    }

    periods = device->NumUpdates;
    numChannels = ChannelsFromDevFmt(device->FmtChans);
    frameSize = numChannels * BytesFromDevFmt(device->FmtType);

    ossSpeed = device->Frequency;
    log2FragmentSize = log2i(device->UpdateSize * frameSize);

    /* according to the OSS spec, 16 bytes are the minimum */
    if (log2FragmentSize < 4)
        log2FragmentSize = 4;
    /* Subtract one period since the temp mixing buffer counts as one. Still
     * need at least two on the card, though. */
    if(periods > 2) periods--;
    numFragmentsLogSize = (periods << 16) | log2FragmentSize;

#define CHECKERR(func) if((func) < 0) {                                       \
    err = #func;                                                              \
    goto err;                                                                 \
}
    /* Don't fail if SETFRAGMENT fails. We can handle just about anything
     * that's reported back via GETOSPACE */
    ioctl(self->fd, SNDCTL_DSP_SETFRAGMENT, &numFragmentsLogSize);
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_SETFMT, &ossFormat));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_CHANNELS, &numChannels));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_SPEED, &ossSpeed));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_GETOSPACE, &info));
    if(0)
    {
    err:
        ERR("%s failed: %s\n", err, strerror(errno));
        return ALC_FALSE;
    }
#undef CHECKERR

    if((int)ChannelsFromDevFmt(device->FmtChans) != numChannels)
    {
        ERR("Failed to set %s, got %d channels instead\n", DevFmtChannelsString(device->FmtChans), numChannels);
        return ALC_FALSE;
    }

    if(!((ossFormat == AFMT_S8 && device->FmtType == DevFmtByte) ||
         (ossFormat == AFMT_U8 && device->FmtType == DevFmtUByte) ||
         (ossFormat == AFMT_S16_NE && device->FmtType == DevFmtShort)))
    {
        ERR("Failed to set %s samples, got OSS format %#x\n", DevFmtTypeString(device->FmtType), ossFormat);
        return ALC_FALSE;
    }

    device->Frequency = ossSpeed;
    device->UpdateSize = info.fragsize / frameSize;
    device->NumUpdates = info.fragments + 1;

    SetDefaultChannelOrder(device);

    return ALC_TRUE;
}

static ALCboolean ALCplaybackOSS_start(ALCplaybackOSS *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    self->data_size = device->UpdateSize * FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    self->mix_data = calloc(1, self->data_size);

    self->killNow = 0;
    if(althrd_create(&self->thread, ALCplaybackOSS_mixerProc, self) != althrd_success)
    {
        free(self->mix_data);
        self->mix_data = NULL;
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCplaybackOSS_stop(ALCplaybackOSS *self)
{
    int res;

    if(self->killNow)
        return;

    self->killNow = 1;
    althrd_join(self->thread, &res);

    if(ioctl(self->fd, SNDCTL_DSP_RESET) != 0)
        ERR("Error resetting device: %s\n", strerror(errno));

    free(self->mix_data);
    self->mix_data = NULL;
}


typedef struct ALCcaptureOSS {
    DERIVE_FROM_TYPE(ALCbackend);

    int fd;

    ll_ringbuffer_t *ring;
    int doCapture;

    volatile int killNow;
    althrd_t thread;
} ALCcaptureOSS;

static int ALCcaptureOSS_recordProc(void *ptr);

static void ALCcaptureOSS_Construct(ALCcaptureOSS *self, ALCdevice *device);
static DECLARE_FORWARD(ALCcaptureOSS, ALCbackend, void, Destruct)
static ALCenum ALCcaptureOSS_open(ALCcaptureOSS *self, const ALCchar *name);
static void ALCcaptureOSS_close(ALCcaptureOSS *self);
static DECLARE_FORWARD(ALCcaptureOSS, ALCbackend, ALCboolean, reset)
static ALCboolean ALCcaptureOSS_start(ALCcaptureOSS *self);
static void ALCcaptureOSS_stop(ALCcaptureOSS *self);
static ALCenum ALCcaptureOSS_captureSamples(ALCcaptureOSS *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCcaptureOSS_availableSamples(ALCcaptureOSS *self);
static DECLARE_FORWARD(ALCcaptureOSS, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCcaptureOSS, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCcaptureOSS, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCcaptureOSS)
DEFINE_ALCBACKEND_VTABLE(ALCcaptureOSS);


static int ALCcaptureOSS_recordProc(void *ptr)
{
    ALCcaptureOSS *self = (ALCcaptureOSS*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    int frameSize;
    ssize_t amt;

    SetRTPriority();
    althrd_setname(althrd_current(), RECORD_THREAD_NAME);

    frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType);

    while(!self->killNow)
    {
        ll_ringbuffer_data_t vec[2];

        amt = 0;
        if(self->doCapture)
        {
            ll_ringbuffer_get_write_vector(self->ring, vec);
            if(vec[0].len > 0)
            {
                amt = read(self->fd, vec[0].buf, vec[0].len*frameSize);
                if(amt < 0)
                {
                    ERR("read failed: %s\n", strerror(errno));
                    ALCcaptureOSS_lock(self);
                    aluHandleDisconnect(device);
                    ALCcaptureOSS_unlock(self);
                    break;
                }
                ll_ringbuffer_write_advance(self->ring, amt/frameSize);
            }
        }
        if(amt == 0)
        {
            al_nssleep(1000000);
            continue;
        }
    }

    return 0;
}


static void ALCcaptureOSS_Construct(ALCcaptureOSS *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCcaptureOSS, ALCbackend, self);
}

static ALCenum ALCcaptureOSS_open(ALCcaptureOSS *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    struct oss_device *dev = &oss_capture;
    int numFragmentsLogSize;
    int log2FragmentSize;
    unsigned int periods;
    audio_buf_info info;
    ALuint frameSize;
    int numChannels;
    int ossFormat;
    int ossSpeed;
    char *err;

    if(!name)
        name = dev->handle;
    else
    {
        while (dev != NULL)
        {
            if (strcmp(dev->handle, name) == 0)
                break;
            dev = dev->next;
        }
        if (dev == NULL)
            return ALC_INVALID_VALUE;
    }

    self->fd = open(dev->path, O_RDONLY);
    if(self->fd == -1)
    {
        ERR("Could not open %s: %s\n", dev->path, strerror(errno));
        return ALC_INVALID_VALUE;
    }

    switch(device->FmtType)
    {
        case DevFmtByte:
            ossFormat = AFMT_S8;
            break;
        case DevFmtUByte:
            ossFormat = AFMT_U8;
            break;
        case DevFmtShort:
            ossFormat = AFMT_S16_NE;
            break;
        case DevFmtUShort:
        case DevFmtInt:
        case DevFmtUInt:
        case DevFmtFloat:
            ERR("%s capture samples not supported\n", DevFmtTypeString(device->FmtType));
            return ALC_INVALID_VALUE;
    }

    periods = 4;
    numChannels = ChannelsFromDevFmt(device->FmtChans);
    frameSize = numChannels * BytesFromDevFmt(device->FmtType);
    ossSpeed = device->Frequency;
    log2FragmentSize = log2i(device->UpdateSize * device->NumUpdates *
                             frameSize / periods);

    /* according to the OSS spec, 16 bytes are the minimum */
    if (log2FragmentSize < 4)
        log2FragmentSize = 4;
    numFragmentsLogSize = (periods << 16) | log2FragmentSize;

#define CHECKERR(func) if((func) < 0) {                                       \
    err = #func;                                                              \
    goto err;                                                                 \
}
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_SETFRAGMENT, &numFragmentsLogSize));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_SETFMT, &ossFormat));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_CHANNELS, &numChannels));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_SPEED, &ossSpeed));
    CHECKERR(ioctl(self->fd, SNDCTL_DSP_GETISPACE, &info));
    if(0)
    {
    err:
        ERR("%s failed: %s\n", err, strerror(errno));
        close(self->fd);
        self->fd = -1;
        return ALC_INVALID_VALUE;
    }
#undef CHECKERR

    if((int)ChannelsFromDevFmt(device->FmtChans) != numChannels)
    {
        ERR("Failed to set %s, got %d channels instead\n", DevFmtChannelsString(device->FmtChans), numChannels);
        close(self->fd);
        self->fd = -1;
        return ALC_INVALID_VALUE;
    }

    if(!((ossFormat == AFMT_S8 && device->FmtType == DevFmtByte) ||
         (ossFormat == AFMT_U8 && device->FmtType == DevFmtUByte) ||
         (ossFormat == AFMT_S16_NE && device->FmtType == DevFmtShort)))
    {
        ERR("Failed to set %s samples, got OSS format %#x\n", DevFmtTypeString(device->FmtType), ossFormat);
        close(self->fd);
        self->fd = -1;
        return ALC_INVALID_VALUE;
    }

    self->ring = ll_ringbuffer_create(device->UpdateSize*device->NumUpdates + 1, frameSize);
    if(!self->ring)
    {
        ERR("Ring buffer create failed\n");
        close(self->fd);
        self->fd = -1;
        return ALC_OUT_OF_MEMORY;
    }

    self->killNow = 0;
    if(althrd_create(&self->thread, ALCcaptureOSS_recordProc, self) != althrd_success)
    {
        ll_ringbuffer_free(self->ring);
        self->ring = NULL;
        close(self->fd);
        self->fd = -1;
        return ALC_OUT_OF_MEMORY;
    }

    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static void ALCcaptureOSS_close(ALCcaptureOSS *self)
{
    int res;

    self->killNow = 1;
    althrd_join(self->thread, &res);

    close(self->fd);
    self->fd = -1;

    ll_ringbuffer_free(self->ring);
    self->ring = NULL;
}

static ALCboolean ALCcaptureOSS_start(ALCcaptureOSS *self)
{
    self->doCapture = 1;
    return ALC_TRUE;
}

static void ALCcaptureOSS_stop(ALCcaptureOSS *self)
{
    self->doCapture = 0;
}

static ALCenum ALCcaptureOSS_captureSamples(ALCcaptureOSS *self, ALCvoid *buffer, ALCuint samples)
{
    ll_ringbuffer_read(self->ring, buffer, samples);
    return ALC_NO_ERROR;
}

static ALCuint ALCcaptureOSS_availableSamples(ALCcaptureOSS *self)
{
    return ll_ringbuffer_read_space(self->ring);
}


typedef struct ALCossBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCossBackendFactory;
#define ALCOSSBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCossBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCossBackendFactory_getFactory(void);

static ALCboolean ALCossBackendFactory_init(ALCossBackendFactory *self);
static void ALCossBackendFactory_deinit(ALCossBackendFactory *self);
static ALCboolean ALCossBackendFactory_querySupport(ALCossBackendFactory *self, ALCbackend_Type type);
static void ALCossBackendFactory_probe(ALCossBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCossBackendFactory_createBackend(ALCossBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCossBackendFactory);


ALCbackendFactory *ALCossBackendFactory_getFactory(void)
{
    static ALCossBackendFactory factory = ALCOSSBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


ALCboolean ALCossBackendFactory_init(ALCossBackendFactory* UNUSED(self))
{
    ConfigValueStr(NULL, "oss", "device", &oss_playback.path);
    ConfigValueStr(NULL, "oss", "capture", &oss_capture.path);

    return ALC_TRUE;
}

void  ALCossBackendFactory_deinit(ALCossBackendFactory* UNUSED(self))
{
    ALCossListFree(&oss_playback);
    ALCossListFree(&oss_capture);
}


ALCboolean ALCossBackendFactory_querySupport(ALCossBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || type == ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

void ALCossBackendFactory_probe(ALCossBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
        {
            struct oss_device *cur = &oss_playback;
            ALCossListFree(cur);
            ALCossListPopulate(cur, NULL);
            while (cur != NULL)
            {
                AppendAllDevicesList(cur->handle);
                cur = cur->next;
            }
        }
        break;

        case CAPTURE_DEVICE_PROBE:
        {
            struct oss_device *cur = &oss_capture;
            ALCossListFree(cur);
            ALCossListPopulate(NULL, cur);
            while (cur != NULL)
            {
                AppendCaptureDeviceList(cur->handle);
                cur = cur->next;
            }
        }
        break;
    }
}

ALCbackend* ALCossBackendFactory_createBackend(ALCossBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCplaybackOSS *backend;
        NEW_OBJ(backend, ALCplaybackOSS)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }
    if(type == ALCbackend_Capture)
    {
        ALCcaptureOSS *backend;
        NEW_OBJ(backend, ALCcaptureOSS)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
