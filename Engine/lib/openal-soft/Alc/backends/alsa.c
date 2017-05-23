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

#include "alMain.h"
#include "alu.h"
#include "threads.h"
#include "compat.h"

#include "backends/base.h"

#include <alsa/asoundlib.h>


static const ALCchar alsaDevice[] = "ALSA Default";


#ifdef HAVE_DYNLOAD
#define ALSA_FUNCS(MAGIC)                                                     \
    MAGIC(snd_strerror);                                                      \
    MAGIC(snd_pcm_open);                                                      \
    MAGIC(snd_pcm_close);                                                     \
    MAGIC(snd_pcm_nonblock);                                                  \
    MAGIC(snd_pcm_frames_to_bytes);                                           \
    MAGIC(snd_pcm_bytes_to_frames);                                           \
    MAGIC(snd_pcm_hw_params_malloc);                                          \
    MAGIC(snd_pcm_hw_params_free);                                            \
    MAGIC(snd_pcm_hw_params_any);                                             \
    MAGIC(snd_pcm_hw_params_current);                                         \
    MAGIC(snd_pcm_hw_params_set_access);                                      \
    MAGIC(snd_pcm_hw_params_set_format);                                      \
    MAGIC(snd_pcm_hw_params_set_channels);                                    \
    MAGIC(snd_pcm_hw_params_set_periods_near);                                \
    MAGIC(snd_pcm_hw_params_set_rate_near);                                   \
    MAGIC(snd_pcm_hw_params_set_rate);                                        \
    MAGIC(snd_pcm_hw_params_set_rate_resample);                               \
    MAGIC(snd_pcm_hw_params_set_buffer_time_near);                            \
    MAGIC(snd_pcm_hw_params_set_period_time_near);                            \
    MAGIC(snd_pcm_hw_params_set_buffer_size_near);                            \
    MAGIC(snd_pcm_hw_params_set_period_size_near);                            \
    MAGIC(snd_pcm_hw_params_set_buffer_size_min);                             \
    MAGIC(snd_pcm_hw_params_get_buffer_time_min);                             \
    MAGIC(snd_pcm_hw_params_get_buffer_time_max);                             \
    MAGIC(snd_pcm_hw_params_get_period_time_min);                             \
    MAGIC(snd_pcm_hw_params_get_period_time_max);                             \
    MAGIC(snd_pcm_hw_params_get_buffer_size);                                 \
    MAGIC(snd_pcm_hw_params_get_period_size);                                 \
    MAGIC(snd_pcm_hw_params_get_access);                                      \
    MAGIC(snd_pcm_hw_params_get_periods);                                     \
    MAGIC(snd_pcm_hw_params_test_format);                                     \
    MAGIC(snd_pcm_hw_params_test_channels);                                   \
    MAGIC(snd_pcm_hw_params);                                                 \
    MAGIC(snd_pcm_sw_params_malloc);                                          \
    MAGIC(snd_pcm_sw_params_current);                                         \
    MAGIC(snd_pcm_sw_params_set_avail_min);                                   \
    MAGIC(snd_pcm_sw_params_set_stop_threshold);                              \
    MAGIC(snd_pcm_sw_params);                                                 \
    MAGIC(snd_pcm_sw_params_free);                                            \
    MAGIC(snd_pcm_prepare);                                                   \
    MAGIC(snd_pcm_start);                                                     \
    MAGIC(snd_pcm_resume);                                                    \
    MAGIC(snd_pcm_reset);                                                     \
    MAGIC(snd_pcm_wait);                                                      \
    MAGIC(snd_pcm_delay);                                                     \
    MAGIC(snd_pcm_state);                                                     \
    MAGIC(snd_pcm_avail_update);                                              \
    MAGIC(snd_pcm_areas_silence);                                             \
    MAGIC(snd_pcm_mmap_begin);                                                \
    MAGIC(snd_pcm_mmap_commit);                                               \
    MAGIC(snd_pcm_readi);                                                     \
    MAGIC(snd_pcm_writei);                                                    \
    MAGIC(snd_pcm_drain);                                                     \
    MAGIC(snd_pcm_drop);                                                      \
    MAGIC(snd_pcm_recover);                                                   \
    MAGIC(snd_pcm_info_malloc);                                               \
    MAGIC(snd_pcm_info_free);                                                 \
    MAGIC(snd_pcm_info_set_device);                                           \
    MAGIC(snd_pcm_info_set_subdevice);                                        \
    MAGIC(snd_pcm_info_set_stream);                                           \
    MAGIC(snd_pcm_info_get_name);                                             \
    MAGIC(snd_ctl_pcm_next_device);                                           \
    MAGIC(snd_ctl_pcm_info);                                                  \
    MAGIC(snd_ctl_open);                                                      \
    MAGIC(snd_ctl_close);                                                     \
    MAGIC(snd_ctl_card_info_malloc);                                          \
    MAGIC(snd_ctl_card_info_free);                                            \
    MAGIC(snd_ctl_card_info);                                                 \
    MAGIC(snd_ctl_card_info_get_name);                                        \
    MAGIC(snd_ctl_card_info_get_id);                                          \
    MAGIC(snd_card_next);                                                     \
    MAGIC(snd_config_update_free_global)

static void *alsa_handle;
#define MAKE_FUNC(f) static __typeof(f) * p##f
ALSA_FUNCS(MAKE_FUNC);
#undef MAKE_FUNC

#define snd_strerror psnd_strerror
#define snd_pcm_open psnd_pcm_open
#define snd_pcm_close psnd_pcm_close
#define snd_pcm_nonblock psnd_pcm_nonblock
#define snd_pcm_frames_to_bytes psnd_pcm_frames_to_bytes
#define snd_pcm_bytes_to_frames psnd_pcm_bytes_to_frames
#define snd_pcm_hw_params_malloc psnd_pcm_hw_params_malloc
#define snd_pcm_hw_params_free psnd_pcm_hw_params_free
#define snd_pcm_hw_params_any psnd_pcm_hw_params_any
#define snd_pcm_hw_params_current psnd_pcm_hw_params_current
#define snd_pcm_hw_params_set_access psnd_pcm_hw_params_set_access
#define snd_pcm_hw_params_set_format psnd_pcm_hw_params_set_format
#define snd_pcm_hw_params_set_channels psnd_pcm_hw_params_set_channels
#define snd_pcm_hw_params_set_periods_near psnd_pcm_hw_params_set_periods_near
#define snd_pcm_hw_params_set_rate_near psnd_pcm_hw_params_set_rate_near
#define snd_pcm_hw_params_set_rate psnd_pcm_hw_params_set_rate
#define snd_pcm_hw_params_set_rate_resample psnd_pcm_hw_params_set_rate_resample
#define snd_pcm_hw_params_set_buffer_time_near psnd_pcm_hw_params_set_buffer_time_near
#define snd_pcm_hw_params_set_period_time_near psnd_pcm_hw_params_set_period_time_near
#define snd_pcm_hw_params_set_buffer_size_near psnd_pcm_hw_params_set_buffer_size_near
#define snd_pcm_hw_params_set_period_size_near psnd_pcm_hw_params_set_period_size_near
#define snd_pcm_hw_params_set_buffer_size_min psnd_pcm_hw_params_set_buffer_size_min
#define snd_pcm_hw_params_get_buffer_time_min psnd_pcm_hw_params_get_buffer_time_min
#define snd_pcm_hw_params_get_buffer_time_max psnd_pcm_hw_params_get_buffer_time_max
#define snd_pcm_hw_params_get_period_time_min psnd_pcm_hw_params_get_period_time_min
#define snd_pcm_hw_params_get_period_time_max psnd_pcm_hw_params_get_period_time_max
#define snd_pcm_hw_params_get_buffer_size psnd_pcm_hw_params_get_buffer_size
#define snd_pcm_hw_params_get_period_size psnd_pcm_hw_params_get_period_size
#define snd_pcm_hw_params_get_access psnd_pcm_hw_params_get_access
#define snd_pcm_hw_params_get_periods psnd_pcm_hw_params_get_periods
#define snd_pcm_hw_params_test_format psnd_pcm_hw_params_test_format
#define snd_pcm_hw_params_test_channels psnd_pcm_hw_params_test_channels
#define snd_pcm_hw_params psnd_pcm_hw_params
#define snd_pcm_sw_params_malloc psnd_pcm_sw_params_malloc
#define snd_pcm_sw_params_current psnd_pcm_sw_params_current
#define snd_pcm_sw_params_set_avail_min psnd_pcm_sw_params_set_avail_min
#define snd_pcm_sw_params_set_stop_threshold psnd_pcm_sw_params_set_stop_threshold
#define snd_pcm_sw_params psnd_pcm_sw_params
#define snd_pcm_sw_params_free psnd_pcm_sw_params_free
#define snd_pcm_prepare psnd_pcm_prepare
#define snd_pcm_start psnd_pcm_start
#define snd_pcm_resume psnd_pcm_resume
#define snd_pcm_reset psnd_pcm_reset
#define snd_pcm_wait psnd_pcm_wait
#define snd_pcm_delay psnd_pcm_delay
#define snd_pcm_state psnd_pcm_state
#define snd_pcm_avail_update psnd_pcm_avail_update
#define snd_pcm_areas_silence psnd_pcm_areas_silence
#define snd_pcm_mmap_begin psnd_pcm_mmap_begin
#define snd_pcm_mmap_commit psnd_pcm_mmap_commit
#define snd_pcm_readi psnd_pcm_readi
#define snd_pcm_writei psnd_pcm_writei
#define snd_pcm_drain psnd_pcm_drain
#define snd_pcm_drop psnd_pcm_drop
#define snd_pcm_recover psnd_pcm_recover
#define snd_pcm_info_malloc psnd_pcm_info_malloc
#define snd_pcm_info_free psnd_pcm_info_free
#define snd_pcm_info_set_device psnd_pcm_info_set_device
#define snd_pcm_info_set_subdevice psnd_pcm_info_set_subdevice
#define snd_pcm_info_set_stream psnd_pcm_info_set_stream
#define snd_pcm_info_get_name psnd_pcm_info_get_name
#define snd_ctl_pcm_next_device psnd_ctl_pcm_next_device
#define snd_ctl_pcm_info psnd_ctl_pcm_info
#define snd_ctl_open psnd_ctl_open
#define snd_ctl_close psnd_ctl_close
#define snd_ctl_card_info_malloc psnd_ctl_card_info_malloc
#define snd_ctl_card_info_free psnd_ctl_card_info_free
#define snd_ctl_card_info psnd_ctl_card_info
#define snd_ctl_card_info_get_name psnd_ctl_card_info_get_name
#define snd_ctl_card_info_get_id psnd_ctl_card_info_get_id
#define snd_card_next psnd_card_next
#define snd_config_update_free_global psnd_config_update_free_global
#endif


static ALCboolean alsa_load(void)
{
    ALCboolean error = ALC_FALSE;

#ifdef HAVE_DYNLOAD
    if(!alsa_handle)
    {
        alsa_handle = LoadLib("libasound.so.2");
        if(!alsa_handle)
            return ALC_FALSE;

        error = ALC_FALSE;
#define LOAD_FUNC(f) do {                                                     \
    p##f = GetSymbol(alsa_handle, #f);                                        \
    if(p##f == NULL) {                                                        \
        error = ALC_TRUE;                                                     \
    }                                                                         \
} while(0)
        ALSA_FUNCS(LOAD_FUNC);
#undef LOAD_FUNC

        if(error)
        {
            CloseLib(alsa_handle);
            alsa_handle = NULL;
            return ALC_FALSE;
        }
    }
#endif

    return !error;
}


typedef struct {
    al_string name;
    al_string device_name;
} DevMap;
TYPEDEF_VECTOR(DevMap, vector_DevMap)

static vector_DevMap PlaybackDevices;
static vector_DevMap CaptureDevices;

static void clear_devlist(vector_DevMap *devlist)
{
#define FREE_DEV(i) do {                                                      \
    AL_STRING_DEINIT((i)->name);                                              \
    AL_STRING_DEINIT((i)->device_name);                                       \
} while(0)
    VECTOR_FOR_EACH(DevMap, *devlist, FREE_DEV);
    VECTOR_RESIZE(*devlist, 0, 0);
#undef FREE_DEV
}


static const char *prefix_name(snd_pcm_stream_t stream)
{
    assert(stream == SND_PCM_STREAM_PLAYBACK || stream == SND_PCM_STREAM_CAPTURE);
    return (stream==SND_PCM_STREAM_PLAYBACK) ? "device-prefix" : "capture-prefix";
}

static void probe_devices(snd_pcm_stream_t stream, vector_DevMap *DeviceList)
{
    const char *main_prefix = "plughw:";
    snd_ctl_t *handle;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    int card, err, dev;
    DevMap entry;

    clear_devlist(DeviceList);

    snd_ctl_card_info_malloc(&info);
    snd_pcm_info_malloc(&pcminfo);

    AL_STRING_INIT(entry.name);
    AL_STRING_INIT(entry.device_name);
    al_string_copy_cstr(&entry.name, alsaDevice);
    al_string_copy_cstr(&entry.device_name, GetConfigValue(NULL, "alsa", (stream==SND_PCM_STREAM_PLAYBACK) ?
                                                           "device" : "capture", "default"));
    VECTOR_PUSH_BACK(*DeviceList, entry);

    card = -1;
    if((err=snd_card_next(&card)) < 0)
        ERR("Failed to find a card: %s\n", snd_strerror(err));
    ConfigValueStr(NULL, "alsa", prefix_name(stream), &main_prefix);
    while(card >= 0)
    {
        const char *card_prefix = main_prefix;
        const char *cardname, *cardid;
        char name[256];

        snprintf(name, sizeof(name), "hw:%d", card);
        if((err = snd_ctl_open(&handle, name, 0)) < 0)
        {
            ERR("control open (hw:%d): %s\n", card, snd_strerror(err));
            goto next_card;
        }
        if((err = snd_ctl_card_info(handle, info)) < 0)
        {
            ERR("control hardware info (hw:%d): %s\n", card, snd_strerror(err));
            snd_ctl_close(handle);
            goto next_card;
        }

        cardname = snd_ctl_card_info_get_name(info);
        cardid = snd_ctl_card_info_get_id(info);

        snprintf(name, sizeof(name), "%s-%s", prefix_name(stream), cardid);
        ConfigValueStr(NULL, "alsa", name, &card_prefix);

        dev = -1;
        while(1)
        {
            const char *device_prefix = card_prefix;
            const char *devname;
            char device[128];

            if(snd_ctl_pcm_next_device(handle, &dev) < 0)
                ERR("snd_ctl_pcm_next_device failed\n");
            if(dev < 0)
                break;

            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, stream);
            if((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                if(err != -ENOENT)
                    ERR("control digital audio info (hw:%d): %s\n", card, snd_strerror(err));
                continue;
            }

            devname = snd_pcm_info_get_name(pcminfo);

            snprintf(name, sizeof(name), "%s-%s-%d", prefix_name(stream), cardid, dev);
            ConfigValueStr(NULL, "alsa", name, &device_prefix);

            snprintf(name, sizeof(name), "%s, %s (CARD=%s,DEV=%d)",
                        cardname, devname, cardid, dev);
            snprintf(device, sizeof(device), "%sCARD=%s,DEV=%d",
                        device_prefix, cardid, dev);

            TRACE("Got device \"%s\", \"%s\"\n", name, device);
            AL_STRING_INIT(entry.name);
            AL_STRING_INIT(entry.device_name);
            al_string_copy_cstr(&entry.name, name);
            al_string_copy_cstr(&entry.device_name, device);
            VECTOR_PUSH_BACK(*DeviceList, entry);
        }
        snd_ctl_close(handle);
    next_card:
        if(snd_card_next(&card) < 0) {
            ERR("snd_card_next failed\n");
            break;
        }
    }

    snd_pcm_info_free(pcminfo);
    snd_ctl_card_info_free(info);
}


static int verify_state(snd_pcm_t *handle)
{
    snd_pcm_state_t state = snd_pcm_state(handle);
    int err;

    switch(state)
    {
        case SND_PCM_STATE_OPEN:
        case SND_PCM_STATE_SETUP:
        case SND_PCM_STATE_PREPARED:
        case SND_PCM_STATE_RUNNING:
        case SND_PCM_STATE_DRAINING:
        case SND_PCM_STATE_PAUSED:
            /* All Okay */
            break;

        case SND_PCM_STATE_XRUN:
            if((err=snd_pcm_recover(handle, -EPIPE, 1)) < 0)
                return err;
            break;
        case SND_PCM_STATE_SUSPENDED:
            if((err=snd_pcm_recover(handle, -ESTRPIPE, 1)) < 0)
                return err;
            break;
        case SND_PCM_STATE_DISCONNECTED:
            return -ENODEV;
    }

    return state;
}


typedef struct ALCplaybackAlsa {
    DERIVE_FROM_TYPE(ALCbackend);

    snd_pcm_t *pcmHandle;

    ALvoid *buffer;
    ALsizei size;

    volatile int killNow;
    althrd_t thread;
} ALCplaybackAlsa;

static int ALCplaybackAlsa_mixerProc(void *ptr);
static int ALCplaybackAlsa_mixerNoMMapProc(void *ptr);

static void ALCplaybackAlsa_Construct(ALCplaybackAlsa *self, ALCdevice *device);
static DECLARE_FORWARD(ALCplaybackAlsa, ALCbackend, void, Destruct)
static ALCenum ALCplaybackAlsa_open(ALCplaybackAlsa *self, const ALCchar *name);
static void ALCplaybackAlsa_close(ALCplaybackAlsa *self);
static ALCboolean ALCplaybackAlsa_reset(ALCplaybackAlsa *self);
static ALCboolean ALCplaybackAlsa_start(ALCplaybackAlsa *self);
static void ALCplaybackAlsa_stop(ALCplaybackAlsa *self);
static DECLARE_FORWARD2(ALCplaybackAlsa, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCplaybackAlsa, ALCbackend, ALCuint, availableSamples)
static ClockLatency ALCplaybackAlsa_getClockLatency(ALCplaybackAlsa *self);
static DECLARE_FORWARD(ALCplaybackAlsa, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCplaybackAlsa, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCplaybackAlsa)

DEFINE_ALCBACKEND_VTABLE(ALCplaybackAlsa);


static void ALCplaybackAlsa_Construct(ALCplaybackAlsa *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCplaybackAlsa, ALCbackend, self);
}


static int ALCplaybackAlsa_mixerProc(void *ptr)
{
    ALCplaybackAlsa *self = (ALCplaybackAlsa*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const snd_pcm_channel_area_t *areas = NULL;
    snd_pcm_uframes_t update_size, num_updates;
    snd_pcm_sframes_t avail, commitres;
    snd_pcm_uframes_t offset, frames;
    char *WritePtr;
    int err;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    update_size = device->UpdateSize;
    num_updates = device->NumUpdates;
    while(!self->killNow)
    {
        int state = verify_state(self->pcmHandle);
        if(state < 0)
        {
            ERR("Invalid state detected: %s\n", snd_strerror(state));
            ALCplaybackAlsa_lock(self);
            aluHandleDisconnect(device);
            ALCplaybackAlsa_unlock(self);
            break;
        }

        avail = snd_pcm_avail_update(self->pcmHandle);
        if(avail < 0)
        {
            ERR("available update failed: %s\n", snd_strerror(avail));
            continue;
        }

        if((snd_pcm_uframes_t)avail > update_size*(num_updates+1))
        {
            WARN("available samples exceeds the buffer size\n");
            snd_pcm_reset(self->pcmHandle);
            continue;
        }

        // make sure there's frames to process
        if((snd_pcm_uframes_t)avail < update_size)
        {
            if(state != SND_PCM_STATE_RUNNING)
            {
                err = snd_pcm_start(self->pcmHandle);
                if(err < 0)
                {
                    ERR("start failed: %s\n", snd_strerror(err));
                    continue;
                }
            }
            if(snd_pcm_wait(self->pcmHandle, 1000) == 0)
                ERR("Wait timeout... buffer size too low?\n");
            continue;
        }
        avail -= avail%update_size;

        // it is possible that contiguous areas are smaller, thus we use a loop
        ALCplaybackAlsa_lock(self);
        while(avail > 0)
        {
            frames = avail;

            err = snd_pcm_mmap_begin(self->pcmHandle, &areas, &offset, &frames);
            if(err < 0)
            {
                ERR("mmap begin error: %s\n", snd_strerror(err));
                break;
            }

            WritePtr = (char*)areas->addr + (offset * areas->step / 8);
            aluMixData(device, WritePtr, frames);

            commitres = snd_pcm_mmap_commit(self->pcmHandle, offset, frames);
            if(commitres < 0 || (commitres-frames) != 0)
            {
                ERR("mmap commit error: %s\n",
                    snd_strerror(commitres >= 0 ? -EPIPE : commitres));
                break;
            }

            avail -= frames;
        }
        ALCplaybackAlsa_unlock(self);
    }

    return 0;
}

static int ALCplaybackAlsa_mixerNoMMapProc(void *ptr)
{
    ALCplaybackAlsa *self = (ALCplaybackAlsa*)ptr;
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    snd_pcm_uframes_t update_size, num_updates;
    snd_pcm_sframes_t avail;
    char *WritePtr;
    int err;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    update_size = device->UpdateSize;
    num_updates = device->NumUpdates;
    while(!self->killNow)
    {
        int state = verify_state(self->pcmHandle);
        if(state < 0)
        {
            ERR("Invalid state detected: %s\n", snd_strerror(state));
            ALCplaybackAlsa_lock(self);
            aluHandleDisconnect(device);
            ALCplaybackAlsa_unlock(self);
            break;
        }

        avail = snd_pcm_avail_update(self->pcmHandle);
        if(avail < 0)
        {
            ERR("available update failed: %s\n", snd_strerror(avail));
            continue;
        }

        if((snd_pcm_uframes_t)avail > update_size*num_updates)
        {
            WARN("available samples exceeds the buffer size\n");
            snd_pcm_reset(self->pcmHandle);
            continue;
        }

        if((snd_pcm_uframes_t)avail < update_size)
        {
            if(state != SND_PCM_STATE_RUNNING)
            {
                err = snd_pcm_start(self->pcmHandle);
                if(err < 0)
                {
                    ERR("start failed: %s\n", snd_strerror(err));
                    continue;
                }
            }
            if(snd_pcm_wait(self->pcmHandle, 1000) == 0)
                ERR("Wait timeout... buffer size too low?\n");
            continue;
        }

        ALCplaybackAlsa_lock(self);
        WritePtr = self->buffer;
        avail = snd_pcm_bytes_to_frames(self->pcmHandle, self->size);
        aluMixData(device, WritePtr, avail);

        while(avail > 0)
        {
            int ret = snd_pcm_writei(self->pcmHandle, WritePtr, avail);
            switch (ret)
            {
            case -EAGAIN:
                continue;
#if ESTRPIPE != EPIPE
            case -ESTRPIPE:
#endif
            case -EPIPE:
            case -EINTR:
                ret = snd_pcm_recover(self->pcmHandle, ret, 1);
                if(ret < 0)
                    avail = 0;
                break;
            default:
                if (ret >= 0)
                {
                    WritePtr += snd_pcm_frames_to_bytes(self->pcmHandle, ret);
                    avail -= ret;
                }
                break;
            }
            if (ret < 0)
            {
                ret = snd_pcm_prepare(self->pcmHandle);
                if(ret < 0)
                    break;
            }
        }
        ALCplaybackAlsa_unlock(self);
    }

    return 0;
}


static ALCenum ALCplaybackAlsa_open(ALCplaybackAlsa *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const char *driver = NULL;
    int err;

    if(name)
    {
        const DevMap *iter;

        if(VECTOR_SIZE(PlaybackDevices) == 0)
            probe_devices(SND_PCM_STREAM_PLAYBACK, &PlaybackDevices);

#define MATCH_NAME(i)  (al_string_cmp_cstr((i)->name, name) == 0)
        VECTOR_FIND_IF(iter, const DevMap, PlaybackDevices, MATCH_NAME);
#undef MATCH_NAME
        if(iter == VECTOR_END(PlaybackDevices))
            return ALC_INVALID_VALUE;
        driver = al_string_get_cstr(iter->device_name);
    }
    else
    {
        name = alsaDevice;
        driver = GetConfigValue(NULL, "alsa", "device", "default");
    }

    TRACE("Opening device \"%s\"\n", driver);
    err = snd_pcm_open(&self->pcmHandle, driver, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if(err < 0)
    {
        ERR("Could not open playback device '%s': %s\n", driver, snd_strerror(err));
        return ALC_OUT_OF_MEMORY;
    }

    /* Free alsa's global config tree. Otherwise valgrind reports a ton of leaks. */
    snd_config_update_free_global();

    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;
}

static void ALCplaybackAlsa_close(ALCplaybackAlsa *self)
{
    snd_pcm_close(self->pcmHandle);
}

static ALCboolean ALCplaybackAlsa_reset(ALCplaybackAlsa *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    snd_pcm_uframes_t periodSizeInFrames;
    unsigned int periodLen, bufferLen;
    snd_pcm_sw_params_t *sp = NULL;
    snd_pcm_hw_params_t *hp = NULL;
    snd_pcm_format_t format = -1;
    snd_pcm_access_t access;
    unsigned int periods;
    unsigned int rate;
    const char *funcerr;
    int allowmmap;
    int dir;
    int err;

    switch(device->FmtType)
    {
        case DevFmtByte:
            format = SND_PCM_FORMAT_S8;
            break;
        case DevFmtUByte:
            format = SND_PCM_FORMAT_U8;
            break;
        case DevFmtShort:
            format = SND_PCM_FORMAT_S16;
            break;
        case DevFmtUShort:
            format = SND_PCM_FORMAT_U16;
            break;
        case DevFmtInt:
            format = SND_PCM_FORMAT_S32;
            break;
        case DevFmtUInt:
            format = SND_PCM_FORMAT_U32;
            break;
        case DevFmtFloat:
            format = SND_PCM_FORMAT_FLOAT;
            break;
    }

    allowmmap = GetConfigValueBool(al_string_get_cstr(device->DeviceName), "alsa", "mmap", 1);
    periods = device->NumUpdates;
    periodLen = (ALuint64)device->UpdateSize * 1000000 / device->Frequency;
    bufferLen = periodLen * periods;
    rate = device->Frequency;

    snd_pcm_hw_params_malloc(&hp);
#define CHECK(x) if((funcerr=#x),(err=(x)) < 0) goto error
    CHECK(snd_pcm_hw_params_any(self->pcmHandle, hp));
    /* set interleaved access */
    if(!allowmmap || snd_pcm_hw_params_set_access(self->pcmHandle, hp, SND_PCM_ACCESS_MMAP_INTERLEAVED) < 0)
    {
        /* No mmap */
        CHECK(snd_pcm_hw_params_set_access(self->pcmHandle, hp, SND_PCM_ACCESS_RW_INTERLEAVED));
    }
    /* test and set format (implicitly sets sample bits) */
    if(snd_pcm_hw_params_test_format(self->pcmHandle, hp, format) < 0)
    {
        static const struct {
            snd_pcm_format_t format;
            enum DevFmtType fmttype;
        } formatlist[] = {
            { SND_PCM_FORMAT_FLOAT, DevFmtFloat  },
            { SND_PCM_FORMAT_S32,   DevFmtInt    },
            { SND_PCM_FORMAT_U32,   DevFmtUInt   },
            { SND_PCM_FORMAT_S16,   DevFmtShort  },
            { SND_PCM_FORMAT_U16,   DevFmtUShort },
            { SND_PCM_FORMAT_S8,    DevFmtByte   },
            { SND_PCM_FORMAT_U8,    DevFmtUByte  },
        };
        size_t k;

        for(k = 0;k < COUNTOF(formatlist);k++)
        {
            format = formatlist[k].format;
            if(snd_pcm_hw_params_test_format(self->pcmHandle, hp, format) >= 0)
            {
                device->FmtType = formatlist[k].fmttype;
                break;
            }
        }
    }
    CHECK(snd_pcm_hw_params_set_format(self->pcmHandle, hp, format));
    /* test and set channels (implicitly sets frame bits) */
    if(snd_pcm_hw_params_test_channels(self->pcmHandle, hp, ChannelsFromDevFmt(device->FmtChans)) < 0)
    {
        static const enum DevFmtChannels channellist[] = {
            DevFmtStereo,
            DevFmtQuad,
            DevFmtX51,
            DevFmtX71,
            DevFmtMono,
        };
        size_t k;

        for(k = 0;k < COUNTOF(channellist);k++)
        {
            if(snd_pcm_hw_params_test_channels(self->pcmHandle, hp, ChannelsFromDevFmt(channellist[k])) >= 0)
            {
                device->FmtChans = channellist[k];
                break;
            }
        }
    }
    CHECK(snd_pcm_hw_params_set_channels(self->pcmHandle, hp, ChannelsFromDevFmt(device->FmtChans)));
    /* set rate (implicitly constrains period/buffer parameters) */
    if(!GetConfigValueBool(al_string_get_cstr(device->DeviceName), "alsa", "allow-resampler", 0) ||
       !(device->Flags&DEVICE_FREQUENCY_REQUEST))
    {
        if(snd_pcm_hw_params_set_rate_resample(self->pcmHandle, hp, 0) < 0)
            ERR("Failed to disable ALSA resampler\n");
    }
    else if(snd_pcm_hw_params_set_rate_resample(self->pcmHandle, hp, 1) < 0)
        ERR("Failed to enable ALSA resampler\n");
    CHECK(snd_pcm_hw_params_set_rate_near(self->pcmHandle, hp, &rate, NULL));
    /* set buffer time (implicitly constrains period/buffer parameters) */
    if((err=snd_pcm_hw_params_set_buffer_time_near(self->pcmHandle, hp, &bufferLen, NULL)) < 0)
        ERR("snd_pcm_hw_params_set_buffer_time_near failed: %s\n", snd_strerror(err));
    /* set period time (implicitly sets buffer size/bytes/time and period size/bytes) */
    if((err=snd_pcm_hw_params_set_period_time_near(self->pcmHandle, hp, &periodLen, NULL)) < 0)
        ERR("snd_pcm_hw_params_set_period_time_near failed: %s\n", snd_strerror(err));
    /* install and prepare hardware configuration */
    CHECK(snd_pcm_hw_params(self->pcmHandle, hp));
    /* retrieve configuration info */
    CHECK(snd_pcm_hw_params_get_access(hp, &access));
    CHECK(snd_pcm_hw_params_get_period_size(hp, &periodSizeInFrames, NULL));
    CHECK(snd_pcm_hw_params_get_periods(hp, &periods, &dir));
    if(dir != 0)
        WARN("Inexact period count: %u (%d)\n", periods, dir);

    snd_pcm_hw_params_free(hp);
    hp = NULL;
    snd_pcm_sw_params_malloc(&sp);

    CHECK(snd_pcm_sw_params_current(self->pcmHandle, sp));
    CHECK(snd_pcm_sw_params_set_avail_min(self->pcmHandle, sp, periodSizeInFrames));
    CHECK(snd_pcm_sw_params_set_stop_threshold(self->pcmHandle, sp, periodSizeInFrames*periods));
    CHECK(snd_pcm_sw_params(self->pcmHandle, sp));
#undef CHECK
    snd_pcm_sw_params_free(sp);
    sp = NULL;

    device->NumUpdates = periods;
    device->UpdateSize = periodSizeInFrames;
    device->Frequency = rate;

    SetDefaultChannelOrder(device);

    return ALC_TRUE;

error:
    ERR("%s failed: %s\n", funcerr, snd_strerror(err));
    if(hp) snd_pcm_hw_params_free(hp);
    if(sp) snd_pcm_sw_params_free(sp);
    return ALC_FALSE;
}

static ALCboolean ALCplaybackAlsa_start(ALCplaybackAlsa *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    int (*thread_func)(void*) = NULL;
    snd_pcm_hw_params_t *hp = NULL;
    snd_pcm_access_t access;
    const char *funcerr;
    int err;

    snd_pcm_hw_params_malloc(&hp);
#define CHECK(x) if((funcerr=#x),(err=(x)) < 0) goto error
    CHECK(snd_pcm_hw_params_current(self->pcmHandle, hp));
    /* retrieve configuration info */
    CHECK(snd_pcm_hw_params_get_access(hp, &access));
#undef CHECK
    snd_pcm_hw_params_free(hp);
    hp = NULL;

    self->size = snd_pcm_frames_to_bytes(self->pcmHandle, device->UpdateSize);
    if(access == SND_PCM_ACCESS_RW_INTERLEAVED)
    {
        self->buffer = al_malloc(16, self->size);
        if(!self->buffer)
        {
            ERR("buffer malloc failed\n");
            return ALC_FALSE;
        }
        thread_func = ALCplaybackAlsa_mixerNoMMapProc;
    }
    else
    {
        err = snd_pcm_prepare(self->pcmHandle);
        if(err < 0)
        {
            ERR("snd_pcm_prepare(data->pcmHandle) failed: %s\n", snd_strerror(err));
            return ALC_FALSE;
        }
        thread_func = ALCplaybackAlsa_mixerProc;
    }
    self->killNow = 0;
    if(althrd_create(&self->thread, thread_func, self) != althrd_success)
    {
        ERR("Could not create playback thread\n");
        al_free(self->buffer);
        self->buffer = NULL;
        return ALC_FALSE;
    }

    return ALC_TRUE;

error:
    ERR("%s failed: %s\n", funcerr, snd_strerror(err));
    if(hp) snd_pcm_hw_params_free(hp);
    return ALC_FALSE;
}

static void ALCplaybackAlsa_stop(ALCplaybackAlsa *self)
{
    int res;

    if(self->killNow)
        return;

    self->killNow = 1;
    althrd_join(self->thread, &res);

    al_free(self->buffer);
    self->buffer = NULL;
}

static ClockLatency ALCplaybackAlsa_getClockLatency(ALCplaybackAlsa *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    snd_pcm_sframes_t delay = 0;
    ClockLatency ret;
    int err;

    ALCplaybackAlsa_lock(self);
    ret.ClockTime = GetDeviceClockTime(device);
    if((err=snd_pcm_delay(self->pcmHandle, &delay)) < 0)
    {
        ERR("Failed to get pcm delay: %s\n", snd_strerror(err));
        delay = 0;
    }
    if(delay < 0) delay = 0;
    ret.Latency = delay * DEVICE_CLOCK_RES / device->Frequency;
    ALCplaybackAlsa_unlock(self);

    return ret;
}


typedef struct ALCcaptureAlsa {
    DERIVE_FROM_TYPE(ALCbackend);

    snd_pcm_t *pcmHandle;

    ALvoid *buffer;
    ALsizei size;

    ALboolean doCapture;
    ll_ringbuffer_t *ring;

    snd_pcm_sframes_t last_avail;
} ALCcaptureAlsa;

static void ALCcaptureAlsa_Construct(ALCcaptureAlsa *self, ALCdevice *device);
static DECLARE_FORWARD(ALCcaptureAlsa, ALCbackend, void, Destruct)
static ALCenum ALCcaptureAlsa_open(ALCcaptureAlsa *self, const ALCchar *name);
static void ALCcaptureAlsa_close(ALCcaptureAlsa *self);
static DECLARE_FORWARD(ALCcaptureAlsa, ALCbackend, ALCboolean, reset)
static ALCboolean ALCcaptureAlsa_start(ALCcaptureAlsa *self);
static void ALCcaptureAlsa_stop(ALCcaptureAlsa *self);
static ALCenum ALCcaptureAlsa_captureSamples(ALCcaptureAlsa *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCcaptureAlsa_availableSamples(ALCcaptureAlsa *self);
static ClockLatency ALCcaptureAlsa_getClockLatency(ALCcaptureAlsa *self);
static DECLARE_FORWARD(ALCcaptureAlsa, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCcaptureAlsa, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCcaptureAlsa)

DEFINE_ALCBACKEND_VTABLE(ALCcaptureAlsa);


static void ALCcaptureAlsa_Construct(ALCcaptureAlsa *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCcaptureAlsa, ALCbackend, self);
}


static ALCenum ALCcaptureAlsa_open(ALCcaptureAlsa *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    const char *driver = NULL;
    snd_pcm_hw_params_t *hp;
    snd_pcm_uframes_t bufferSizeInFrames;
    snd_pcm_uframes_t periodSizeInFrames;
    ALboolean needring = AL_FALSE;
    snd_pcm_format_t format = -1;
    const char *funcerr;
    int err;

    if(name)
    {
        const DevMap *iter;

        if(VECTOR_SIZE(CaptureDevices) == 0)
            probe_devices(SND_PCM_STREAM_CAPTURE, &CaptureDevices);

#define MATCH_NAME(i)  (al_string_cmp_cstr((i)->name, name) == 0)
        VECTOR_FIND_IF(iter, const DevMap, CaptureDevices, MATCH_NAME);
#undef MATCH_NAME
        if(iter == VECTOR_END(CaptureDevices))
            return ALC_INVALID_VALUE;
        driver = al_string_get_cstr(iter->device_name);
    }
    else
    {
        name = alsaDevice;
        driver = GetConfigValue(NULL, "alsa", "capture", "default");
    }

    TRACE("Opening device \"%s\"\n", driver);
    err = snd_pcm_open(&self->pcmHandle, driver, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if(err < 0)
    {
        ERR("Could not open capture device '%s': %s\n", driver, snd_strerror(err));
        return ALC_INVALID_VALUE;
    }

    /* Free alsa's global config tree. Otherwise valgrind reports a ton of leaks. */
    snd_config_update_free_global();

    switch(device->FmtType)
    {
        case DevFmtByte:
            format = SND_PCM_FORMAT_S8;
            break;
        case DevFmtUByte:
            format = SND_PCM_FORMAT_U8;
            break;
        case DevFmtShort:
            format = SND_PCM_FORMAT_S16;
            break;
        case DevFmtUShort:
            format = SND_PCM_FORMAT_U16;
            break;
        case DevFmtInt:
            format = SND_PCM_FORMAT_S32;
            break;
        case DevFmtUInt:
            format = SND_PCM_FORMAT_U32;
            break;
        case DevFmtFloat:
            format = SND_PCM_FORMAT_FLOAT;
            break;
    }

    funcerr = NULL;
    bufferSizeInFrames = maxu(device->UpdateSize*device->NumUpdates,
                              100*device->Frequency/1000);
    periodSizeInFrames = minu(bufferSizeInFrames, 25*device->Frequency/1000);

    snd_pcm_hw_params_malloc(&hp);
#define CHECK(x) if((funcerr=#x),(err=(x)) < 0) goto error
    CHECK(snd_pcm_hw_params_any(self->pcmHandle, hp));
    /* set interleaved access */
    CHECK(snd_pcm_hw_params_set_access(self->pcmHandle, hp, SND_PCM_ACCESS_RW_INTERLEAVED));
    /* set format (implicitly sets sample bits) */
    CHECK(snd_pcm_hw_params_set_format(self->pcmHandle, hp, format));
    /* set channels (implicitly sets frame bits) */
    CHECK(snd_pcm_hw_params_set_channels(self->pcmHandle, hp, ChannelsFromDevFmt(device->FmtChans)));
    /* set rate (implicitly constrains period/buffer parameters) */
    CHECK(snd_pcm_hw_params_set_rate(self->pcmHandle, hp, device->Frequency, 0));
    /* set buffer size in frame units (implicitly sets period size/bytes/time and buffer time/bytes) */
    if(snd_pcm_hw_params_set_buffer_size_min(self->pcmHandle, hp, &bufferSizeInFrames) < 0)
    {
        TRACE("Buffer too large, using intermediate ring buffer\n");
        needring = AL_TRUE;
        CHECK(snd_pcm_hw_params_set_buffer_size_near(self->pcmHandle, hp, &bufferSizeInFrames));
    }
    /* set buffer size in frame units (implicitly sets period size/bytes/time and buffer time/bytes) */
    CHECK(snd_pcm_hw_params_set_period_size_near(self->pcmHandle, hp, &periodSizeInFrames, NULL));
    /* install and prepare hardware configuration */
    CHECK(snd_pcm_hw_params(self->pcmHandle, hp));
    /* retrieve configuration info */
    CHECK(snd_pcm_hw_params_get_period_size(hp, &periodSizeInFrames, NULL));
#undef CHECK
    snd_pcm_hw_params_free(hp);
    hp = NULL;

    if(needring)
    {
        self->ring = ll_ringbuffer_create(
            device->UpdateSize*device->NumUpdates + 1,
            FrameSizeFromDevFmt(device->FmtChans, device->FmtType)
        );
        if(!self->ring)
        {
            ERR("ring buffer create failed\n");
            goto error2;
        }
    }

    al_string_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;

error:
    ERR("%s failed: %s\n", funcerr, snd_strerror(err));
    if(hp) snd_pcm_hw_params_free(hp);

error2:
    ll_ringbuffer_free(self->ring);
    self->ring = NULL;
    snd_pcm_close(self->pcmHandle);

    return ALC_INVALID_VALUE;
}

static void ALCcaptureAlsa_close(ALCcaptureAlsa *self)
{
    snd_pcm_close(self->pcmHandle);
    ll_ringbuffer_free(self->ring);

    al_free(self->buffer);
    self->buffer = NULL;
}

static ALCboolean ALCcaptureAlsa_start(ALCcaptureAlsa *self)
{
    int err = snd_pcm_start(self->pcmHandle);
    if(err < 0)
    {
        ERR("start failed: %s\n", snd_strerror(err));
        aluHandleDisconnect(STATIC_CAST(ALCbackend, self)->mDevice);
        return ALC_FALSE;
    }

    self->doCapture = AL_TRUE;
    return ALC_TRUE;
}

static void ALCcaptureAlsa_stop(ALCcaptureAlsa *self)
{
    ALCuint avail;
    int err;

    /* OpenAL requires access to unread audio after stopping, but ALSA's
     * snd_pcm_drain is unreliable and snd_pcm_drop drops it. Capture what's
     * available now so it'll be available later after the drop. */
    avail = ALCcaptureAlsa_availableSamples(self);
    if(!self->ring && avail > 0)
    {
        /* The ring buffer implicitly captures when checking availability.
         * Direct access needs to explicitly capture it into temp storage. */
        ALsizei size;
        void *ptr;

        size = snd_pcm_frames_to_bytes(self->pcmHandle, avail);
        ptr = al_malloc(16, size);
        if(ptr)
        {
            ALCcaptureAlsa_captureSamples(self, ptr, avail);
            al_free(self->buffer);
            self->buffer = ptr;
            self->size = size;
        }
    }
    err = snd_pcm_drop(self->pcmHandle);
    if(err < 0)
        ERR("drop failed: %s\n", snd_strerror(err));
    self->doCapture = AL_FALSE;
}

static ALCenum ALCcaptureAlsa_captureSamples(ALCcaptureAlsa *self, ALCvoid *buffer, ALCuint samples)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;

    if(self->ring)
    {
        ll_ringbuffer_read(self->ring, buffer, samples);
        return ALC_NO_ERROR;
    }

    self->last_avail -= samples;
    while(device->Connected && samples > 0)
    {
        snd_pcm_sframes_t amt = 0;

        if(self->size > 0)
        {
            /* First get any data stored from the last stop */
            amt = snd_pcm_bytes_to_frames(self->pcmHandle, self->size);
            if((snd_pcm_uframes_t)amt > samples) amt = samples;

            amt = snd_pcm_frames_to_bytes(self->pcmHandle, amt);
            memcpy(buffer, self->buffer, amt);

            if(self->size > amt)
            {
                memmove(self->buffer, self->buffer+amt, self->size - amt);
                self->size -= amt;
            }
            else
            {
                al_free(self->buffer);
                self->buffer = NULL;
                self->size = 0;
            }
            amt = snd_pcm_bytes_to_frames(self->pcmHandle, amt);
        }
        else if(self->doCapture)
            amt = snd_pcm_readi(self->pcmHandle, buffer, samples);
        if(amt < 0)
        {
            ERR("read error: %s\n", snd_strerror(amt));

            if(amt == -EAGAIN)
                continue;
            if((amt=snd_pcm_recover(self->pcmHandle, amt, 1)) >= 0)
            {
                amt = snd_pcm_start(self->pcmHandle);
                if(amt >= 0)
                    amt = snd_pcm_avail_update(self->pcmHandle);
            }
            if(amt < 0)
            {
                ERR("restore error: %s\n", snd_strerror(amt));
                aluHandleDisconnect(device);
                break;
            }
            /* If the amount available is less than what's asked, we lost it
             * during recovery. So just give silence instead. */
            if((snd_pcm_uframes_t)amt < samples)
                break;
            continue;
        }

        buffer = (ALbyte*)buffer + amt;
        samples -= amt;
    }
    if(samples > 0)
        memset(buffer, ((device->FmtType == DevFmtUByte) ? 0x80 : 0),
               snd_pcm_frames_to_bytes(self->pcmHandle, samples));

    return ALC_NO_ERROR;
}

static ALCuint ALCcaptureAlsa_availableSamples(ALCcaptureAlsa *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    snd_pcm_sframes_t avail = 0;

    if(device->Connected && self->doCapture)
        avail = snd_pcm_avail_update(self->pcmHandle);
    if(avail < 0)
    {
        ERR("avail update failed: %s\n", snd_strerror(avail));

        if((avail=snd_pcm_recover(self->pcmHandle, avail, 1)) >= 0)
        {
            if(self->doCapture)
                avail = snd_pcm_start(self->pcmHandle);
            if(avail >= 0)
                avail = snd_pcm_avail_update(self->pcmHandle);
        }
        if(avail < 0)
        {
            ERR("restore error: %s\n", snd_strerror(avail));
            aluHandleDisconnect(device);
        }
    }

    if(!self->ring)
    {
        if(avail < 0) avail = 0;
        avail += snd_pcm_bytes_to_frames(self->pcmHandle, self->size);
        if(avail > self->last_avail) self->last_avail = avail;
        return self->last_avail;
    }

    while(avail > 0)
    {
        ll_ringbuffer_data_t vec[2];
        snd_pcm_sframes_t amt;

        ll_ringbuffer_get_write_vector(self->ring, vec);
        if(vec[0].len == 0) break;

        amt = (vec[0].len < (snd_pcm_uframes_t)avail) ?
              vec[0].len : (snd_pcm_uframes_t)avail;
        amt = snd_pcm_readi(self->pcmHandle, vec[0].buf, amt);
        if(amt < 0)
        {
            ERR("read error: %s\n", snd_strerror(amt));

            if(amt == -EAGAIN)
                continue;
            if((amt=snd_pcm_recover(self->pcmHandle, amt, 1)) >= 0)
            {
                if(self->doCapture)
                    amt = snd_pcm_start(self->pcmHandle);
                if(amt >= 0)
                    amt = snd_pcm_avail_update(self->pcmHandle);
            }
            if(amt < 0)
            {
                ERR("restore error: %s\n", snd_strerror(amt));
                aluHandleDisconnect(device);
                break;
            }
            avail = amt;
            continue;
        }

        ll_ringbuffer_write_advance(self->ring, amt);
        avail -= amt;
    }

    return ll_ringbuffer_read_space(self->ring);
}

static ClockLatency ALCcaptureAlsa_getClockLatency(ALCcaptureAlsa *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend, self)->mDevice;
    snd_pcm_sframes_t delay = 0;
    ClockLatency ret;
    int err;

    ALCcaptureAlsa_lock(self);
    ret.ClockTime = GetDeviceClockTime(device);
    if((err=snd_pcm_delay(self->pcmHandle, &delay)) < 0)
    {
        ERR("Failed to get pcm delay: %s\n", snd_strerror(err));
        delay = 0;
    }
    if(delay < 0) delay = 0;
    ret.Latency = delay * DEVICE_CLOCK_RES / device->Frequency;
    ALCcaptureAlsa_unlock(self);

    return ret;
}


static inline void AppendAllDevicesList2(const DevMap *entry)
{ AppendAllDevicesList(al_string_get_cstr(entry->name)); }
static inline void AppendCaptureDeviceList2(const DevMap *entry)
{ AppendCaptureDeviceList(al_string_get_cstr(entry->name)); }

typedef struct ALCalsaBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCalsaBackendFactory;
#define ALCALSABACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCalsaBackendFactory, ALCbackendFactory) } }

static ALCboolean ALCalsaBackendFactory_init(ALCalsaBackendFactory* UNUSED(self))
{
    VECTOR_INIT(PlaybackDevices);
    VECTOR_INIT(CaptureDevices);

    if(!alsa_load())
        return ALC_FALSE;
    return ALC_TRUE;
}

static void ALCalsaBackendFactory_deinit(ALCalsaBackendFactory* UNUSED(self))
{
    clear_devlist(&PlaybackDevices);
    VECTOR_DEINIT(PlaybackDevices);

    clear_devlist(&CaptureDevices);
    VECTOR_DEINIT(CaptureDevices);

#ifdef HAVE_DYNLOAD
    if(alsa_handle)
        CloseLib(alsa_handle);
    alsa_handle = NULL;
#endif
}

static ALCboolean ALCalsaBackendFactory_querySupport(ALCalsaBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || type == ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCalsaBackendFactory_probe(ALCalsaBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            probe_devices(SND_PCM_STREAM_PLAYBACK, &PlaybackDevices);
            VECTOR_FOR_EACH(const DevMap, PlaybackDevices, AppendAllDevicesList2);
            break;

        case CAPTURE_DEVICE_PROBE:
            probe_devices(SND_PCM_STREAM_CAPTURE, &CaptureDevices);
            VECTOR_FOR_EACH(const DevMap, CaptureDevices, AppendCaptureDeviceList2);
            break;
    }
}

static ALCbackend* ALCalsaBackendFactory_createBackend(ALCalsaBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCplaybackAlsa *backend;
        NEW_OBJ(backend, ALCplaybackAlsa)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }
    if(type == ALCbackend_Capture)
    {
        ALCcaptureAlsa *backend;
        NEW_OBJ(backend, ALCcaptureAlsa)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}

DEFINE_ALCBACKENDFACTORY_VTABLE(ALCalsaBackendFactory);


ALCbackendFactory *ALCalsaBackendFactory_getFactory(void)
{
    static ALCalsaBackendFactory factory = ALCALSABACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}
