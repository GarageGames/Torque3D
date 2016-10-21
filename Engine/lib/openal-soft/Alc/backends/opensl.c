/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This is an OpenAL backend for Android using the native audio APIs based on
 * OpenSL ES 1.0.1. It is based on source code for the native-audio sample app
 * bundled with NDK.
 */

#include "config.h"

#include <stdlib.h>

#include "alMain.h"
#include "alu.h"
#include "threads.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

/* Helper macros */
#define VCALL(obj, func)  ((*(obj))->func((obj), EXTRACT_VCALL_ARGS
#define VCALL0(obj, func)  ((*(obj))->func((obj) EXTRACT_VCALL_ARGS


typedef struct {
    /* engine interfaces */
    SLObjectItf engineObject;
    SLEngineItf engine;

    /* output mix interfaces */
    SLObjectItf outputMix;

    /* buffer queue player interfaces */
    SLObjectItf bufferQueueObject;

    void *buffer;
    ALuint bufferSize;
    ALuint curBuffer;

    ALuint frameSize;
} osl_data;


static const ALCchar opensl_device[] = "OpenSL";


static SLuint32 GetChannelMask(enum DevFmtChannels chans)
{
    switch(chans)
    {
        case DevFmtMono: return SL_SPEAKER_FRONT_CENTER;
        case DevFmtStereo: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT;
        case DevFmtQuad: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT|
                                SL_SPEAKER_BACK_LEFT|SL_SPEAKER_BACK_RIGHT;
        case DevFmtX51: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT|
                               SL_SPEAKER_FRONT_CENTER|SL_SPEAKER_LOW_FREQUENCY|
                               SL_SPEAKER_SIDE_LEFT|SL_SPEAKER_SIDE_RIGHT;
        case DevFmtX51Rear: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT|
                                   SL_SPEAKER_FRONT_CENTER|SL_SPEAKER_LOW_FREQUENCY|
                                   SL_SPEAKER_BACK_LEFT|SL_SPEAKER_BACK_RIGHT;
        case DevFmtX61: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT|
                               SL_SPEAKER_FRONT_CENTER|SL_SPEAKER_LOW_FREQUENCY|
                               SL_SPEAKER_BACK_CENTER|
                               SL_SPEAKER_SIDE_LEFT|SL_SPEAKER_SIDE_RIGHT;
        case DevFmtX71: return SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT|
                               SL_SPEAKER_FRONT_CENTER|SL_SPEAKER_LOW_FREQUENCY|
                               SL_SPEAKER_BACK_LEFT|SL_SPEAKER_BACK_RIGHT|
                               SL_SPEAKER_SIDE_LEFT|SL_SPEAKER_SIDE_RIGHT;
        case DevFmtAmbi1:
        case DevFmtAmbi2:
        case DevFmtAmbi3:
            break;
    }
    return 0;
}

static const char *res_str(SLresult result)
{
    switch(result)
    {
        case SL_RESULT_SUCCESS: return "Success";
        case SL_RESULT_PRECONDITIONS_VIOLATED: return "Preconditions violated";
        case SL_RESULT_PARAMETER_INVALID: return "Parameter invalid";
        case SL_RESULT_MEMORY_FAILURE: return "Memory failure";
        case SL_RESULT_RESOURCE_ERROR: return "Resource error";
        case SL_RESULT_RESOURCE_LOST: return "Resource lost";
        case SL_RESULT_IO_ERROR: return "I/O error";
        case SL_RESULT_BUFFER_INSUFFICIENT: return "Buffer insufficient";
        case SL_RESULT_CONTENT_CORRUPTED: return "Content corrupted";
        case SL_RESULT_CONTENT_UNSUPPORTED: return "Content unsupported";
        case SL_RESULT_CONTENT_NOT_FOUND: return "Content not found";
        case SL_RESULT_PERMISSION_DENIED: return "Permission denied";
        case SL_RESULT_FEATURE_UNSUPPORTED: return "Feature unsupported";
        case SL_RESULT_INTERNAL_ERROR: return "Internal error";
        case SL_RESULT_UNKNOWN_ERROR: return "Unknown error";
        case SL_RESULT_OPERATION_ABORTED: return "Operation aborted";
        case SL_RESULT_CONTROL_LOST: return "Control lost";
#ifdef SL_RESULT_READONLY
        case SL_RESULT_READONLY: return "ReadOnly";
#endif
#ifdef SL_RESULT_ENGINEOPTION_UNSUPPORTED
        case SL_RESULT_ENGINEOPTION_UNSUPPORTED: return "Engine option unsupported";
#endif
#ifdef SL_RESULT_SOURCE_SINK_INCOMPATIBLE
        case SL_RESULT_SOURCE_SINK_INCOMPATIBLE: return "Source/Sink incompatible";
#endif
    }
    return "Unknown error code";
}

#define PRINTERR(x, s) do {                                                      \
    if((x) != SL_RESULT_SUCCESS)                                                 \
        ERR("%s: %s\n", (s), res_str((x)));                                      \
} while(0)

/* this callback handler is called every time a buffer finishes playing */
static void opensl_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    ALCdevice *Device = context;
    osl_data *data = Device->ExtraData;
    ALvoid *buf;
    SLresult result;

    buf = (ALbyte*)data->buffer + data->curBuffer*data->bufferSize;
    aluMixData(Device, buf, data->bufferSize/data->frameSize);

    result = VCALL(bq,Enqueue)(buf, data->bufferSize);
    PRINTERR(result, "bq->Enqueue");

    data->curBuffer = (data->curBuffer+1) % Device->NumUpdates;
}


static ALCenum opensl_open_playback(ALCdevice *Device, const ALCchar *deviceName)
{
    osl_data *data = NULL;
    SLresult result;

    if(!deviceName)
        deviceName = opensl_device;
    else if(strcmp(deviceName, opensl_device) != 0)
        return ALC_INVALID_VALUE;

    data = calloc(1, sizeof(*data));
    if(!data)
        return ALC_OUT_OF_MEMORY;

    // create engine
    result = slCreateEngine(&data->engineObject, 0, NULL, 0, NULL, NULL);
    PRINTERR(result, "slCreateEngine");
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->engineObject,Realize)(SL_BOOLEAN_FALSE);
        PRINTERR(result, "engine->Realize");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->engineObject,GetInterface)(SL_IID_ENGINE, &data->engine);
        PRINTERR(result, "engine->GetInterface");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->engine,CreateOutputMix)(&data->outputMix, 0, NULL, NULL);
        PRINTERR(result, "engine->CreateOutputMix");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->outputMix,Realize)(SL_BOOLEAN_FALSE);
        PRINTERR(result, "outputMix->Realize");
    }

    if(SL_RESULT_SUCCESS != result)
    {
        if(data->outputMix != NULL)
            VCALL0(data->outputMix,Destroy)();
        data->outputMix = NULL;

        if(data->engineObject != NULL)
            VCALL0(data->engineObject,Destroy)();
        data->engineObject = NULL;
        data->engine = NULL;

        free(data);
        return ALC_INVALID_VALUE;
    }

    al_string_copy_cstr(&Device->DeviceName, deviceName);
    Device->ExtraData = data;

    return ALC_NO_ERROR;
}


static void opensl_close_playback(ALCdevice *Device)
{
    osl_data *data = Device->ExtraData;

    if(data->bufferQueueObject != NULL)
        VCALL0(data->bufferQueueObject,Destroy)();
    data->bufferQueueObject = NULL;

    VCALL0(data->outputMix,Destroy)();
    data->outputMix = NULL;

    VCALL0(data->engineObject,Destroy)();
    data->engineObject = NULL;
    data->engine = NULL;

    free(data);
    Device->ExtraData = NULL;
}

static ALCboolean opensl_reset_playback(ALCdevice *Device)
{
    osl_data *data = Device->ExtraData;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq;
    SLDataLocator_OutputMix loc_outmix;
    SLDataFormat_PCM format_pcm;
    SLDataSource audioSrc;
    SLDataSink audioSnk;
    SLInterfaceID id;
    SLboolean req;
    SLresult result;


    Device->UpdateSize = (ALuint64)Device->UpdateSize * 44100 / Device->Frequency;
    Device->UpdateSize = Device->UpdateSize * Device->NumUpdates / 2;
    Device->NumUpdates = 2;

    Device->Frequency = 44100;
    Device->FmtChans = DevFmtStereo;
    Device->FmtType = DevFmtShort;

    SetDefaultWFXChannelOrder(Device);


    id  = SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    req = SL_BOOLEAN_TRUE;

    loc_bufq.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    loc_bufq.numBuffers = Device->NumUpdates;

    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = ChannelsFromDevFmt(Device->FmtChans);
    format_pcm.samplesPerSec = Device->Frequency * 1000;
    format_pcm.bitsPerSample = BytesFromDevFmt(Device->FmtType) * 8;
    format_pcm.containerSize = format_pcm.bitsPerSample;
    format_pcm.channelMask = GetChannelMask(Device->FmtChans);
    format_pcm.endianness = IS_LITTLE_ENDIAN ? SL_BYTEORDER_LITTLEENDIAN :
                                               SL_BYTEORDER_BIGENDIAN;

    audioSrc.pLocator = &loc_bufq;
    audioSrc.pFormat = &format_pcm;

    loc_outmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    loc_outmix.outputMix = data->outputMix;
    audioSnk.pLocator = &loc_outmix;
    audioSnk.pFormat = NULL;


    if(data->bufferQueueObject != NULL)
        VCALL0(data->bufferQueueObject,Destroy)();
    data->bufferQueueObject = NULL;

    result = VCALL(data->engine,CreateAudioPlayer)(&data->bufferQueueObject, &audioSrc, &audioSnk, 1, &id, &req);
    PRINTERR(result, "engine->CreateAudioPlayer");
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->bufferQueueObject,Realize)(SL_BOOLEAN_FALSE);
        PRINTERR(result, "bufferQueue->Realize");
    }

    if(SL_RESULT_SUCCESS != result)
    {
        if(data->bufferQueueObject != NULL)
            VCALL0(data->bufferQueueObject,Destroy)();
        data->bufferQueueObject = NULL;

        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static ALCboolean opensl_start_playback(ALCdevice *Device)
{
    osl_data *data = Device->ExtraData;
    SLAndroidSimpleBufferQueueItf bufferQueue;
    SLPlayItf player;
    SLresult result;
    ALuint i;

    result = VCALL(data->bufferQueueObject,GetInterface)(SL_IID_BUFFERQUEUE, &bufferQueue);
    PRINTERR(result, "bufferQueue->GetInterface");
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(bufferQueue,RegisterCallback)(opensl_callback, Device);
        PRINTERR(result, "bufferQueue->RegisterCallback");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        data->frameSize = FrameSizeFromDevFmt(Device->FmtChans, Device->FmtType);
        data->bufferSize = Device->UpdateSize * data->frameSize;
        data->buffer = calloc(Device->NumUpdates, data->bufferSize);
        if(!data->buffer)
        {
            result = SL_RESULT_MEMORY_FAILURE;
            PRINTERR(result, "calloc");
        }
    }
    /* enqueue the first buffer to kick off the callbacks */
    for(i = 0;i < Device->NumUpdates;i++)
    {
        if(SL_RESULT_SUCCESS == result)
        {
            ALvoid *buf = (ALbyte*)data->buffer + i*data->bufferSize;
            result = VCALL(bufferQueue,Enqueue)(buf, data->bufferSize);
            PRINTERR(result, "bufferQueue->Enqueue");
        }
    }
    data->curBuffer = 0;
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(data->bufferQueueObject,GetInterface)(SL_IID_PLAY, &player);
        PRINTERR(result, "bufferQueue->GetInterface");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(player,SetPlayState)(SL_PLAYSTATE_PLAYING);
        PRINTERR(result, "player->SetPlayState");
    }

    if(SL_RESULT_SUCCESS != result)
    {
        if(data->bufferQueueObject != NULL)
            VCALL0(data->bufferQueueObject,Destroy)();
        data->bufferQueueObject = NULL;

        free(data->buffer);
        data->buffer = NULL;
        data->bufferSize = 0;

        return ALC_FALSE;
    }

    return ALC_TRUE;
}


static void opensl_stop_playback(ALCdevice *Device)
{
    osl_data *data = Device->ExtraData;
    SLPlayItf player;
    SLAndroidSimpleBufferQueueItf bufferQueue;
    SLresult result;

    result = VCALL(data->bufferQueueObject,GetInterface)(SL_IID_PLAY, &player);
    PRINTERR(result, "bufferQueue->GetInterface");
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL(player,SetPlayState)(SL_PLAYSTATE_STOPPED);
        PRINTERR(result, "player->SetPlayState");
    }

    result = VCALL(data->bufferQueueObject,GetInterface)(SL_IID_BUFFERQUEUE, &bufferQueue);
    PRINTERR(result, "bufferQueue->GetInterface");
    if(SL_RESULT_SUCCESS == result)
    {
        result = VCALL0(bufferQueue,Clear)();
        PRINTERR(result, "bufferQueue->Clear");
    }
    if(SL_RESULT_SUCCESS == result)
    {
        SLAndroidSimpleBufferQueueState state;
        do {
            althrd_yield();
            result = VCALL(bufferQueue,GetState)(&state);
        } while(SL_RESULT_SUCCESS == result && state.count > 0);
        PRINTERR(result, "bufferQueue->GetState");
    }

    free(data->buffer);
    data->buffer = NULL;
    data->bufferSize = 0;
}


static const BackendFuncs opensl_funcs = {
    opensl_open_playback,
    opensl_close_playback,
    opensl_reset_playback,
    opensl_start_playback,
    opensl_stop_playback,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


ALCboolean alc_opensl_init(BackendFuncs *func_list)
{
    *func_list = opensl_funcs;
    return ALC_TRUE;
}

void alc_opensl_deinit(void)
{
}

void alc_opensl_probe(enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(opensl_device);
            break;
        case CAPTURE_DEVICE_PROBE:
            break;
    }
}
