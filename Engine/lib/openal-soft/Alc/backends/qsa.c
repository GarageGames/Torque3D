/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011-2013 by authors.
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
#include <sched.h>
#include <errno.h>
#include <memory.h>
#include <sys/select.h>
#include <sys/asoundlib.h>
#include <sys/neutrino.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"


typedef struct {
    snd_pcm_t* pcmHandle;
    int audio_fd;

    snd_pcm_channel_setup_t  csetup;
    snd_pcm_channel_params_t cparams;

    ALvoid* buffer;
    ALsizei size;

    volatile int killNow;
    althrd_t thread;
} qsa_data;

typedef struct {
    ALCchar* name;
    int card;
    int dev;
} DevMap;
TYPEDEF_VECTOR(DevMap, vector_DevMap)

static vector_DevMap DeviceNameMap;
static vector_DevMap CaptureNameMap;

static const ALCchar qsaDevice[] = "QSA Default";

static const struct {
    int32_t format;
} formatlist[] = {
    {SND_PCM_SFMT_FLOAT_LE},
    {SND_PCM_SFMT_S32_LE},
    {SND_PCM_SFMT_U32_LE},
    {SND_PCM_SFMT_S16_LE},
    {SND_PCM_SFMT_U16_LE},
    {SND_PCM_SFMT_S8},
    {SND_PCM_SFMT_U8},
    {0},
};

static const struct {
    int32_t rate;
} ratelist[] = {
    {192000},
    {176400},
    {96000},
    {88200},
    {48000},
    {44100},
    {32000},
    {24000},
    {22050},
    {16000},
    {12000},
    {11025},
    {8000},
    {0},
};

static const struct {
    int32_t channels;
} channellist[] = {
    {8},
    {7},
    {6},
    {4},
    {2},
    {1},
    {0},
};

static void deviceList(int type, vector_DevMap *devmap)
{
    snd_ctl_t* handle;
    snd_pcm_info_t pcminfo;
    int max_cards, card, err, dev;
    DevMap entry;
    char name[1024];
    struct snd_ctl_hw_info info;

    max_cards = snd_cards();
    if(max_cards < 0)
        return;

    VECTOR_RESIZE(*devmap, 0, max_cards+1);

    entry.name = strdup(qsaDevice);
    entry.card = 0;
    entry.dev = 0;
    VECTOR_PUSH_BACK(*devmap, entry);

    for(card = 0;card < max_cards;card++)
    {
        if((err=snd_ctl_open(&handle, card)) < 0)
            continue;

        if((err=snd_ctl_hw_info(handle, &info)) < 0)
        {
            snd_ctl_close(handle);
            continue;
        }

        for(dev = 0;dev < (int)info.pcmdevs;dev++)
        {
            if((err=snd_ctl_pcm_info(handle, dev, &pcminfo)) < 0)
                continue;

            if((type==SND_PCM_CHANNEL_PLAYBACK && (pcminfo.flags&SND_PCM_INFO_PLAYBACK)) ||
               (type==SND_PCM_CHANNEL_CAPTURE && (pcminfo.flags&SND_PCM_INFO_CAPTURE)))
            {
                snprintf(name, sizeof(name), "%s [%s] (hw:%d,%d)", info.name, pcminfo.name, card, dev);
                entry.name = strdup(name);
                entry.card = card;
                entry.dev = dev;

                VECTOR_PUSH_BACK(*devmap, entry);
                TRACE("Got device \"%s\", card %d, dev %d\n", name, card, dev);
            }
        }
        snd_ctl_close(handle);
    }
}


FORCE_ALIGN static int qsa_proc_playback(void* ptr)
{
    ALCdevice* device=(ALCdevice*)ptr;
    qsa_data* data=(qsa_data*)device->ExtraData;
    char* write_ptr;
    int avail;
    snd_pcm_channel_status_t status;
    struct sched_param param;
    fd_set wfds;
    int selectret;
    struct timeval timeout;

    SetRTPriority();
    althrd_setname(althrd_current(), MIXER_THREAD_NAME);

    /* Increase default 10 priority to 11 to avoid jerky sound */
    SchedGet(0, 0, &param);
    param.sched_priority=param.sched_curpriority+1;
    SchedSet(0, 0, SCHED_NOCHANGE, &param);

    ALint frame_size=FrameSizeFromDevFmt(device->FmtChans, device->FmtType);

    while (!data->killNow)
    {
        ALint len=data->size;
        write_ptr=data->buffer;

        avail=len/frame_size;
        aluMixData(device, write_ptr, avail);

        while (len>0 && !data->killNow)
        {
            FD_ZERO(&wfds);
            FD_SET(data->audio_fd, &wfds);
            timeout.tv_sec=2;
            timeout.tv_usec=0;

            /* Select also works like time slice to OS */
            selectret=select(data->audio_fd+1, NULL, &wfds, NULL, &timeout);
            switch (selectret)
            {
                case -1:
                     aluHandleDisconnect(device);
                     return 1;
                case 0:
                     break;
                default:
                     if (FD_ISSET(data->audio_fd, &wfds))
                     {
                         break;
                     }
                     break;
            }

            int wrote=snd_pcm_plugin_write(data->pcmHandle, write_ptr, len);

            if (wrote<=0)
            {
                if ((errno==EAGAIN) || (errno==EWOULDBLOCK))
                {
                    continue;
                }

                memset(&status, 0, sizeof (status));
                status.channel=SND_PCM_CHANNEL_PLAYBACK;

                snd_pcm_plugin_status(data->pcmHandle, &status);

                /* we need to reinitialize the sound channel if we've underrun the buffer */
                if ((status.status==SND_PCM_STATUS_UNDERRUN) ||
                    (status.status==SND_PCM_STATUS_READY))
                {
                    if ((snd_pcm_plugin_prepare(data->pcmHandle, SND_PCM_CHANNEL_PLAYBACK))<0)
                    {
                        aluHandleDisconnect(device);
                        break;
                    }
                }
            }
            else
            {
                write_ptr+=wrote;
                len-=wrote;
            }
        }
    }

    return 0;
}

/************/
/* Playback */
/************/

static ALCenum qsa_open_playback(ALCdevice* device, const ALCchar* deviceName)
{
    qsa_data *data;
    int card, dev;
    int status;

    data = (qsa_data*)calloc(1, sizeof(qsa_data));
    if(data == NULL)
        return ALC_OUT_OF_MEMORY;

    if(!deviceName)
        deviceName = qsaDevice;

    if(strcmp(deviceName, qsaDevice) == 0)
        status = snd_pcm_open_preferred(&data->pcmHandle, &card, &dev, SND_PCM_OPEN_PLAYBACK);
    else
    {
        const DevMap *iter;

        if(VECTOR_SIZE(DeviceNameMap) == 0)
            deviceList(SND_PCM_CHANNEL_PLAYBACK, &DeviceNameMap);

#define MATCH_DEVNAME(iter) ((iter)->name && strcmp(deviceName, (iter)->name)==0)
        VECTOR_FIND_IF(iter, const DevMap, DeviceNameMap, MATCH_DEVNAME);
#undef MATCH_DEVNAME
        if(iter == VECTOR_END(DeviceNameMap))
        {
            free(data);
            return ALC_INVALID_DEVICE;
        }

        status = snd_pcm_open(&data->pcmHandle, iter->card, iter->dev, SND_PCM_OPEN_PLAYBACK);
    }

    if(status < 0)
    {
        free(data);
        return ALC_INVALID_DEVICE;
    }

    data->audio_fd = snd_pcm_file_descriptor(data->pcmHandle, SND_PCM_CHANNEL_PLAYBACK);
    if(data->audio_fd < 0)
    {
        snd_pcm_close(data->pcmHandle);
        free(data);
        return ALC_INVALID_DEVICE;
    }

    al_string_copy_cstr(&device->DeviceName, deviceName);
    device->ExtraData = data;

    return ALC_NO_ERROR;
}

static void qsa_close_playback(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;

    if (data->buffer!=NULL)
    {
        free(data->buffer);
        data->buffer=NULL;
    }

    snd_pcm_close(data->pcmHandle);
    free(data);

    device->ExtraData=NULL;
}

static ALCboolean qsa_reset_playback(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;
    int32_t format=-1;

    switch(device->FmtType)
    {
        case DevFmtByte:
             format=SND_PCM_SFMT_S8;
             break;
        case DevFmtUByte:
             format=SND_PCM_SFMT_U8;
             break;
        case DevFmtShort:
             format=SND_PCM_SFMT_S16_LE;
             break;
        case DevFmtUShort:
             format=SND_PCM_SFMT_U16_LE;
             break;
        case DevFmtInt:
             format=SND_PCM_SFMT_S32_LE;
             break;
        case DevFmtUInt:
             format=SND_PCM_SFMT_U32_LE;
             break;
        case DevFmtFloat:
             format=SND_PCM_SFMT_FLOAT_LE;
             break;
    }

    /* we actually don't want to block on writes */
    snd_pcm_nonblock_mode(data->pcmHandle, 1);
    /* Disable mmap to control data transfer to the audio device */
    snd_pcm_plugin_set_disable(data->pcmHandle, PLUGIN_DISABLE_MMAP);
    snd_pcm_plugin_set_disable(data->pcmHandle, PLUGIN_DISABLE_BUFFER_PARTIAL_BLOCKS);

    // configure a sound channel
    memset(&data->cparams, 0, sizeof(data->cparams));
    data->cparams.channel=SND_PCM_CHANNEL_PLAYBACK;
    data->cparams.mode=SND_PCM_MODE_BLOCK;
    data->cparams.start_mode=SND_PCM_START_FULL;
    data->cparams.stop_mode=SND_PCM_STOP_STOP;

    data->cparams.buf.block.frag_size=device->UpdateSize*
        ChannelsFromDevFmt(device->FmtChans)*BytesFromDevFmt(device->FmtType);
    data->cparams.buf.block.frags_max=device->NumUpdates;
    data->cparams.buf.block.frags_min=device->NumUpdates;

    data->cparams.format.interleave=1;
    data->cparams.format.rate=device->Frequency;
    data->cparams.format.voices=ChannelsFromDevFmt(device->FmtChans);
    data->cparams.format.format=format;

    if ((snd_pcm_plugin_params(data->pcmHandle, &data->cparams))<0)
    {
        int original_rate=data->cparams.format.rate;
        int original_voices=data->cparams.format.voices;
        int original_format=data->cparams.format.format;
        int it;
        int jt;

        for (it=0; it<1; it++)
        {
            /* Check for second pass */
            if (it==1)
            {
                original_rate=ratelist[0].rate;
                original_voices=channellist[0].channels;
                original_format=formatlist[0].format;
            }

            do {
                /* At first downgrade sample format */
                jt=0;
                do {
                    if (formatlist[jt].format==data->cparams.format.format)
                    {
                        data->cparams.format.format=formatlist[jt+1].format;
                        break;
                    }
                    if (formatlist[jt].format==0)
                    {
                        data->cparams.format.format=0;
                        break;
                    }
                    jt++;
                } while(1);

                if (data->cparams.format.format==0)
                {
                    data->cparams.format.format=original_format;

                    /* At secod downgrade sample rate */
                    jt=0;
                    do {
                        if (ratelist[jt].rate==data->cparams.format.rate)
                        {
                            data->cparams.format.rate=ratelist[jt+1].rate;
                            break;
                        }
                        if (ratelist[jt].rate==0)
                        {
                            data->cparams.format.rate=0;
                            break;
                        }
                        jt++;
                    } while(1);

                    if (data->cparams.format.rate==0)
                    {
                        data->cparams.format.rate=original_rate;
                        data->cparams.format.format=original_format;

                        /* At third downgrade channels number */
                        jt=0;
                        do {
                            if(channellist[jt].channels==data->cparams.format.voices)
                            {
                                data->cparams.format.voices=channellist[jt+1].channels;
                                break;
                            }
                            if (channellist[jt].channels==0)
                            {
                                data->cparams.format.voices=0;
                                break;
                            }
                           jt++;
                        } while(1);
                    }

                    if (data->cparams.format.voices==0)
                    {
                        break;
                    }
                }

                data->cparams.buf.block.frag_size=device->UpdateSize*
                    data->cparams.format.voices*
                    snd_pcm_format_width(data->cparams.format.format)/8;
                data->cparams.buf.block.frags_max=device->NumUpdates;
                data->cparams.buf.block.frags_min=device->NumUpdates;
                if ((snd_pcm_plugin_params(data->pcmHandle, &data->cparams))<0)
                {
                    continue;
                }
                else
                {
                    break;
                }
            } while(1);

            if (data->cparams.format.voices!=0)
            {
                break;
            }
        }

        if (data->cparams.format.voices==0)
        {
            return ALC_FALSE;
        }
    }

    if ((snd_pcm_plugin_prepare(data->pcmHandle, SND_PCM_CHANNEL_PLAYBACK))<0)
    {
        return ALC_FALSE;
    }

    memset(&data->csetup, 0, sizeof(data->csetup));
    data->csetup.channel=SND_PCM_CHANNEL_PLAYBACK;
    if (snd_pcm_plugin_setup(data->pcmHandle, &data->csetup)<0)
    {
        return ALC_FALSE;
    }

    /* now fill back to the our AL device */
    device->Frequency=data->cparams.format.rate;

    switch (data->cparams.format.voices)
    {
        case 1:
             device->FmtChans=DevFmtMono;
             break;
        case 2:
             device->FmtChans=DevFmtStereo;
             break;
        case 4:
             device->FmtChans=DevFmtQuad;
             break;
        case 6:
             device->FmtChans=DevFmtX51;
             break;
        case 7:
             device->FmtChans=DevFmtX61;
             break;
        case 8:
             device->FmtChans=DevFmtX71;
             break;
        default:
             device->FmtChans=DevFmtMono;
             break;
    }

    switch (data->cparams.format.format)
    {
        case SND_PCM_SFMT_S8:
             device->FmtType=DevFmtByte;
             break;
        case SND_PCM_SFMT_U8:
             device->FmtType=DevFmtUByte;
             break;
        case SND_PCM_SFMT_S16_LE:
             device->FmtType=DevFmtShort;
             break;
        case SND_PCM_SFMT_U16_LE:
             device->FmtType=DevFmtUShort;
             break;
        case SND_PCM_SFMT_S32_LE:
             device->FmtType=DevFmtInt;
             break;
        case SND_PCM_SFMT_U32_LE:
             device->FmtType=DevFmtUInt;
             break;
        case SND_PCM_SFMT_FLOAT_LE:
             device->FmtType=DevFmtFloat;
             break;
        default:
             device->FmtType=DevFmtShort;
             break;
    }

    SetDefaultChannelOrder(device);

    device->UpdateSize=data->csetup.buf.block.frag_size/
        (ChannelsFromDevFmt(device->FmtChans)*BytesFromDevFmt(device->FmtType));
    device->NumUpdates=data->csetup.buf.block.frags;

    data->size=data->csetup.buf.block.frag_size;
    data->buffer=malloc(data->size);
    if (!data->buffer)
    {
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static ALCboolean qsa_start_playback(ALCdevice* device)
{
    qsa_data *data = (qsa_data*)device->ExtraData;

    data->killNow = 0;
    if(althrd_create(&data->thread, qsa_proc_playback, device) != althrd_success)
        return ALC_FALSE;

    return ALC_TRUE;
}

static void qsa_stop_playback(ALCdevice* device)
{
    qsa_data *data = (qsa_data*)device->ExtraData;
    int res;

    if(data->killNow)
        return;

    data->killNow = 1;
    althrd_join(data->thread, &res);
}

/***********/
/* Capture */
/***********/

static ALCenum qsa_open_capture(ALCdevice* device, const ALCchar* deviceName)
{
    qsa_data *data;
    int card, dev;
    int format=-1;
    int status;

    data=(qsa_data*)calloc(1, sizeof(qsa_data));
    if (data==NULL)
    {
        return ALC_OUT_OF_MEMORY;
    }

    if(!deviceName)
        deviceName = qsaDevice;

    if(strcmp(deviceName, qsaDevice) == 0)
        status = snd_pcm_open_preferred(&data->pcmHandle, &card, &dev, SND_PCM_OPEN_CAPTURE);
    else
    {
        const DevMap *iter;

        if(VECTOR_SIZE(CaptureNameMap) == 0)
            deviceList(SND_PCM_CHANNEL_CAPTURE, &CaptureNameMap);

#define MATCH_DEVNAME(iter) ((iter)->name && strcmp(deviceName, (iter)->name)==0)
        VECTOR_FIND_IF(iter, const DevMap, CaptureNameMap, MATCH_DEVNAME);
#undef MATCH_DEVNAME
        if(iter == VECTOR_END(CaptureNameMap))
        {
            free(data);
            return ALC_INVALID_DEVICE;
        }

        status = snd_pcm_open(&data->pcmHandle, iter->card, iter->dev, SND_PCM_OPEN_CAPTURE);
    }

    if(status < 0)
    {
        free(data);
        return ALC_INVALID_DEVICE;
    }

    data->audio_fd = snd_pcm_file_descriptor(data->pcmHandle, SND_PCM_CHANNEL_CAPTURE);
    if(data->audio_fd < 0)
    {
        snd_pcm_close(data->pcmHandle);
        free(data);
        return ALC_INVALID_DEVICE;
    }

    al_string_copy_cstr(&device->DeviceName, deviceName);
    device->ExtraData = data;

    switch (device->FmtType)
    {
        case DevFmtByte:
             format=SND_PCM_SFMT_S8;
             break;
        case DevFmtUByte:
             format=SND_PCM_SFMT_U8;
             break;
        case DevFmtShort:
             format=SND_PCM_SFMT_S16_LE;
             break;
        case DevFmtUShort:
             format=SND_PCM_SFMT_U16_LE;
             break;
        case DevFmtInt:
             format=SND_PCM_SFMT_S32_LE;
             break;
        case DevFmtUInt:
             format=SND_PCM_SFMT_U32_LE;
             break;
        case DevFmtFloat:
             format=SND_PCM_SFMT_FLOAT_LE;
             break;
    }

    /* we actually don't want to block on reads */
    snd_pcm_nonblock_mode(data->pcmHandle, 1);
    /* Disable mmap to control data transfer to the audio device */
    snd_pcm_plugin_set_disable(data->pcmHandle, PLUGIN_DISABLE_MMAP);

    /* configure a sound channel */
    memset(&data->cparams, 0, sizeof(data->cparams));
    data->cparams.mode=SND_PCM_MODE_BLOCK;
    data->cparams.channel=SND_PCM_CHANNEL_CAPTURE;
    data->cparams.start_mode=SND_PCM_START_GO;
    data->cparams.stop_mode=SND_PCM_STOP_STOP;

    data->cparams.buf.block.frag_size=device->UpdateSize*
        ChannelsFromDevFmt(device->FmtChans)*BytesFromDevFmt(device->FmtType);
    data->cparams.buf.block.frags_max=device->NumUpdates;
    data->cparams.buf.block.frags_min=device->NumUpdates;

    data->cparams.format.interleave=1;
    data->cparams.format.rate=device->Frequency;
    data->cparams.format.voices=ChannelsFromDevFmt(device->FmtChans);
    data->cparams.format.format=format;

    if(snd_pcm_plugin_params(data->pcmHandle, &data->cparams) < 0)
    {
        snd_pcm_close(data->pcmHandle);
        free(data);
        device->ExtraData=NULL;

        return ALC_INVALID_VALUE;
    }

    return ALC_NO_ERROR;
}

static void qsa_close_capture(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;

    if (data->pcmHandle!=NULL)
        snd_pcm_close(data->pcmHandle);

    free(data);
    device->ExtraData=NULL;
}

static void qsa_start_capture(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;
    int rstatus;

    if ((rstatus=snd_pcm_plugin_prepare(data->pcmHandle, SND_PCM_CHANNEL_CAPTURE))<0)
    {
        ERR("capture prepare failed: %s\n", snd_strerror(rstatus));
        return;
    }

    memset(&data->csetup, 0, sizeof(data->csetup));
    data->csetup.channel=SND_PCM_CHANNEL_CAPTURE;
    if ((rstatus=snd_pcm_plugin_setup(data->pcmHandle, &data->csetup))<0)
    {
        ERR("capture setup failed: %s\n", snd_strerror(rstatus));
        return;
    }

    snd_pcm_capture_go(data->pcmHandle);
}

static void qsa_stop_capture(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;

    snd_pcm_capture_flush(data->pcmHandle);
}

static ALCuint qsa_available_samples(ALCdevice* device)
{
    qsa_data* data=(qsa_data*)device->ExtraData;
    snd_pcm_channel_status_t status;
    ALint frame_size=FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    ALint free_size;
    int rstatus;

    memset(&status, 0, sizeof (status));
    status.channel=SND_PCM_CHANNEL_CAPTURE;
    snd_pcm_plugin_status(data->pcmHandle, &status);
    if ((status.status==SND_PCM_STATUS_OVERRUN) ||
        (status.status==SND_PCM_STATUS_READY))
    {
        if ((rstatus=snd_pcm_plugin_prepare(data->pcmHandle, SND_PCM_CHANNEL_CAPTURE))<0)
        {
            ERR("capture prepare failed: %s\n", snd_strerror(rstatus));
            aluHandleDisconnect(device);
            return 0;
        }

        snd_pcm_capture_go(data->pcmHandle);
        return 0;
    }

    free_size=data->csetup.buf.block.frag_size*data->csetup.buf.block.frags;
    free_size-=status.free;

    return free_size/frame_size;
}

static ALCenum qsa_capture_samples(ALCdevice *device, ALCvoid *buffer, ALCuint samples)
{
    qsa_data* data=(qsa_data*)device->ExtraData;
    char* read_ptr;
    snd_pcm_channel_status_t status;
    fd_set rfds;
    int selectret;
    struct timeval timeout;
    int bytes_read;
    ALint frame_size=FrameSizeFromDevFmt(device->FmtChans, device->FmtType);
    ALint len=samples*frame_size;
    int rstatus;

    read_ptr=buffer;

    while (len>0)
    {
        FD_ZERO(&rfds);
        FD_SET(data->audio_fd, &rfds);
        timeout.tv_sec=2;
        timeout.tv_usec=0;

        /* Select also works like time slice to OS */
        bytes_read=0;
        selectret=select(data->audio_fd+1, &rfds, NULL, NULL, &timeout);
        switch (selectret)
        {
            case -1:
                 aluHandleDisconnect(device);
                 return ALC_INVALID_DEVICE;
            case 0:
                 break;
            default:
                 if (FD_ISSET(data->audio_fd, &rfds))
                 {
                     bytes_read=snd_pcm_plugin_read(data->pcmHandle, read_ptr, len);
                     break;
                 }
                 break;
        }

        if (bytes_read<=0)
        {
            if ((errno==EAGAIN) || (errno==EWOULDBLOCK))
            {
                continue;
            }

            memset(&status, 0, sizeof (status));
            status.channel=SND_PCM_CHANNEL_CAPTURE;
            snd_pcm_plugin_status(data->pcmHandle, &status);

            /* we need to reinitialize the sound channel if we've overrun the buffer */
            if ((status.status==SND_PCM_STATUS_OVERRUN) ||
                (status.status==SND_PCM_STATUS_READY))
            {
                if ((rstatus=snd_pcm_plugin_prepare(data->pcmHandle, SND_PCM_CHANNEL_CAPTURE))<0)
                {
                    ERR("capture prepare failed: %s\n", snd_strerror(rstatus));
                    aluHandleDisconnect(device);
                    return ALC_INVALID_DEVICE;
                }
                snd_pcm_capture_go(data->pcmHandle);
            }
        }
        else
        {
            read_ptr+=bytes_read;
            len-=bytes_read;
        }
    }

    return ALC_NO_ERROR;
}

static const BackendFuncs qsa_funcs= {
    qsa_open_playback,
    qsa_close_playback,
    qsa_reset_playback,
    qsa_start_playback,
    qsa_stop_playback,
    qsa_open_capture,
    qsa_close_capture,
    qsa_start_capture,
    qsa_stop_capture,
    qsa_capture_samples,
    qsa_available_samples
};

ALCboolean alc_qsa_init(BackendFuncs* func_list)
{
    *func_list = qsa_funcs;
    return ALC_TRUE;
}

void alc_qsa_deinit(void)
{
#define FREE_NAME(iter) free((iter)->name)
    VECTOR_FOR_EACH(DevMap, DeviceNameMap, FREE_NAME);
    VECTOR_DEINIT(DeviceNameMap);

    VECTOR_FOR_EACH(DevMap, CaptureNameMap, FREE_NAME);
    VECTOR_DEINIT(CaptureNameMap);
#undef FREE_NAME
}

void alc_qsa_probe(enum DevProbe type)
{
    switch (type)
    {
        case ALL_DEVICE_PROBE:
#define FREE_NAME(iter) free((iter)->name)
            VECTOR_FOR_EACH(DevMap, DeviceNameMap, FREE_NAME);
            VECTOR_RESIZE(DeviceNameMap, 0, 0);
#undef FREE_NAME

            deviceList(SND_PCM_CHANNEL_PLAYBACK, &DeviceNameMap);
#define APPEND_DEVICE(iter) AppendAllDevicesList((iter)->name)
            VECTOR_FOR_EACH(const DevMap, DeviceNameMap, APPEND_DEVICE);
#undef APPEND_DEVICE
            break;

        case CAPTURE_DEVICE_PROBE:
#define FREE_NAME(iter) free((iter)->name)
            VECTOR_FOR_EACH(DevMap, CaptureNameMap, FREE_NAME);
            VECTOR_RESIZE(CaptureNameMap, 0, 0);
#undef FREE_NAME

            deviceList(SND_PCM_CHANNEL_CAPTURE, &CaptureNameMap);
#define APPEND_DEVICE(iter) AppendCaptureDeviceList((iter)->name)
            VECTOR_FOR_EACH(const DevMap, CaptureNameMap, APPEND_DEVICE);
#undef APPEND_DEVICE
            break;
    }
}
