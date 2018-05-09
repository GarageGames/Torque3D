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
#include "ringbuffer.h"

#include <CoreServices/CoreServices.h>
#include <unistd.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include "backends/base.h"


static const ALCchar ca_device[] = "CoreAudio Default";


typedef struct ALCcoreAudioPlayback {
    DERIVE_FROM_TYPE(ALCbackend);

    AudioUnit audioUnit;

    ALuint frameSize;
    AudioStreamBasicDescription format;    // This is the OpenAL format as a CoreAudio ASBD
} ALCcoreAudioPlayback;

static void ALCcoreAudioPlayback_Construct(ALCcoreAudioPlayback *self, ALCdevice *device);
static void ALCcoreAudioPlayback_Destruct(ALCcoreAudioPlayback *self);
static ALCenum ALCcoreAudioPlayback_open(ALCcoreAudioPlayback *self, const ALCchar *name);
static ALCboolean ALCcoreAudioPlayback_reset(ALCcoreAudioPlayback *self);
static ALCboolean ALCcoreAudioPlayback_start(ALCcoreAudioPlayback *self);
static void ALCcoreAudioPlayback_stop(ALCcoreAudioPlayback *self);
static DECLARE_FORWARD2(ALCcoreAudioPlayback, ALCbackend, ALCenum, captureSamples, void*, ALCuint)
static DECLARE_FORWARD(ALCcoreAudioPlayback, ALCbackend, ALCuint, availableSamples)
static DECLARE_FORWARD(ALCcoreAudioPlayback, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCcoreAudioPlayback, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCcoreAudioPlayback, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCcoreAudioPlayback)

DEFINE_ALCBACKEND_VTABLE(ALCcoreAudioPlayback);


static void ALCcoreAudioPlayback_Construct(ALCcoreAudioPlayback *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCcoreAudioPlayback, ALCbackend, self);

    self->frameSize = 0;
    memset(&self->format, 0, sizeof(self->format));
}

static void ALCcoreAudioPlayback_Destruct(ALCcoreAudioPlayback *self)
{
    AudioUnitUninitialize(self->audioUnit);
    AudioComponentInstanceDispose(self->audioUnit);

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static OSStatus ALCcoreAudioPlayback_MixerProc(void *inRefCon,
  AudioUnitRenderActionFlags* UNUSED(ioActionFlags), const AudioTimeStamp* UNUSED(inTimeStamp),
  UInt32 UNUSED(inBusNumber), UInt32 UNUSED(inNumberFrames), AudioBufferList *ioData)
{
    ALCcoreAudioPlayback *self = inRefCon;
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;

    ALCcoreAudioPlayback_lock(self);
    aluMixData(device, ioData->mBuffers[0].mData,
               ioData->mBuffers[0].mDataByteSize / self->frameSize);
    ALCcoreAudioPlayback_unlock(self);

    return noErr;
}


static ALCenum ALCcoreAudioPlayback_open(ALCcoreAudioPlayback *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    AudioComponentDescription desc;
    AudioComponent comp;
    OSStatus err;

    if(!name)
        name = ca_device;
    else if(strcmp(name, ca_device) != 0)
        return ALC_INVALID_VALUE;

    /* open the default output unit */
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    comp = AudioComponentFindNext(NULL, &desc);
    if(comp == NULL)
    {
        ERR("AudioComponentFindNext failed\n");
        return ALC_INVALID_VALUE;
    }

    err = AudioComponentInstanceNew(comp, &self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioComponentInstanceNew failed\n");
        return ALC_INVALID_VALUE;
    }

    /* init and start the default audio unit... */
    err = AudioUnitInitialize(self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioUnitInitialize failed\n");
        AudioComponentInstanceDispose(self->audioUnit);
        return ALC_INVALID_VALUE;
    }

    alstr_copy_cstr(&device->DeviceName, name);
    return ALC_NO_ERROR;
}

static ALCboolean ALCcoreAudioPlayback_reset(ALCcoreAudioPlayback *self)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    AudioStreamBasicDescription streamFormat;
    AURenderCallbackStruct input;
    OSStatus err;
    UInt32 size;

    err = AudioUnitUninitialize(self->audioUnit);
    if(err != noErr)
        ERR("-- AudioUnitUninitialize failed.\n");

    /* retrieve default output unit's properties (output side) */
    size = sizeof(AudioStreamBasicDescription);
    err = AudioUnitGetProperty(self->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &streamFormat, &size);
    if(err != noErr || size != sizeof(AudioStreamBasicDescription))
    {
        ERR("AudioUnitGetProperty failed\n");
        return ALC_FALSE;
    }

#if 0
    TRACE("Output streamFormat of default output unit -\n");
    TRACE("  streamFormat.mFramesPerPacket = %d\n", streamFormat.mFramesPerPacket);
    TRACE("  streamFormat.mChannelsPerFrame = %d\n", streamFormat.mChannelsPerFrame);
    TRACE("  streamFormat.mBitsPerChannel = %d\n", streamFormat.mBitsPerChannel);
    TRACE("  streamFormat.mBytesPerPacket = %d\n", streamFormat.mBytesPerPacket);
    TRACE("  streamFormat.mBytesPerFrame = %d\n", streamFormat.mBytesPerFrame);
    TRACE("  streamFormat.mSampleRate = %5.0f\n", streamFormat.mSampleRate);
#endif

    /* set default output unit's input side to match output side */
    err = AudioUnitSetProperty(self->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &streamFormat, size);
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        return ALC_FALSE;
    }

    if(device->Frequency != streamFormat.mSampleRate)
    {
        device->NumUpdates = (ALuint)((ALuint64)device->NumUpdates *
                                      streamFormat.mSampleRate /
                                      device->Frequency);
        device->Frequency = streamFormat.mSampleRate;
    }

    /* FIXME: How to tell what channels are what in the output device, and how
     * to specify what we're giving?  eg, 6.0 vs 5.1 */
    switch(streamFormat.mChannelsPerFrame)
    {
        case 1:
            device->FmtChans = DevFmtMono;
            break;
        case 2:
            device->FmtChans = DevFmtStereo;
            break;
        case 4:
            device->FmtChans = DevFmtQuad;
            break;
        case 6:
            device->FmtChans = DevFmtX51;
            break;
        case 7:
            device->FmtChans = DevFmtX61;
            break;
        case 8:
            device->FmtChans = DevFmtX71;
            break;
        default:
            ERR("Unhandled channel count (%d), using Stereo\n", streamFormat.mChannelsPerFrame);
            device->FmtChans = DevFmtStereo;
            streamFormat.mChannelsPerFrame = 2;
            break;
    }
    SetDefaultWFXChannelOrder(device);

    /* use channel count and sample rate from the default output unit's current
     * parameters, but reset everything else */
    streamFormat.mFramesPerPacket = 1;
    streamFormat.mFormatFlags = 0;
    switch(device->FmtType)
    {
        case DevFmtUByte:
            device->FmtType = DevFmtByte;
            /* fall-through */
        case DevFmtByte:
            streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
            streamFormat.mBitsPerChannel = 8;
            break;
        case DevFmtUShort:
            device->FmtType = DevFmtShort;
            /* fall-through */
        case DevFmtShort:
            streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
            streamFormat.mBitsPerChannel = 16;
            break;
        case DevFmtUInt:
            device->FmtType = DevFmtInt;
            /* fall-through */
        case DevFmtInt:
            streamFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
            streamFormat.mBitsPerChannel = 32;
            break;
        case DevFmtFloat:
            streamFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
            streamFormat.mBitsPerChannel = 32;
            break;
    }
    streamFormat.mBytesPerFrame = streamFormat.mChannelsPerFrame *
                                  streamFormat.mBitsPerChannel / 8;
    streamFormat.mBytesPerPacket = streamFormat.mBytesPerFrame;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags |= kAudioFormatFlagsNativeEndian |
                                 kLinearPCMFormatFlagIsPacked;

    err = AudioUnitSetProperty(self->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &streamFormat, sizeof(AudioStreamBasicDescription));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        return ALC_FALSE;
    }

    /* setup callback */
    self->frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);
    input.inputProc = ALCcoreAudioPlayback_MixerProc;
    input.inputProcRefCon = self;

    err = AudioUnitSetProperty(self->audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(AURenderCallbackStruct));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        return ALC_FALSE;
    }

    /* init the default audio unit... */
    err = AudioUnitInitialize(self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioUnitInitialize failed\n");
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static ALCboolean ALCcoreAudioPlayback_start(ALCcoreAudioPlayback *self)
{
    OSStatus err = AudioOutputUnitStart(self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioOutputUnitStart failed\n");
        return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void ALCcoreAudioPlayback_stop(ALCcoreAudioPlayback *self)
{
    OSStatus err = AudioOutputUnitStop(self->audioUnit);
    if(err != noErr)
        ERR("AudioOutputUnitStop failed\n");
}




typedef struct ALCcoreAudioCapture {
    DERIVE_FROM_TYPE(ALCbackend);

    AudioUnit audioUnit;

    ALuint frameSize;
    ALdouble sampleRateRatio;              // Ratio of hardware sample rate / requested sample rate
    AudioStreamBasicDescription format;    // This is the OpenAL format as a CoreAudio ASBD

    AudioConverterRef audioConverter;      // Sample rate converter if needed
    AudioBufferList *bufferList;           // Buffer for data coming from the input device
    ALCvoid *resampleBuffer;               // Buffer for returned RingBuffer data when resampling

    ll_ringbuffer_t *ring;
} ALCcoreAudioCapture;

static void ALCcoreAudioCapture_Construct(ALCcoreAudioCapture *self, ALCdevice *device);
static void ALCcoreAudioCapture_Destruct(ALCcoreAudioCapture *self);
static ALCenum ALCcoreAudioCapture_open(ALCcoreAudioCapture *self, const ALCchar *name);
static DECLARE_FORWARD(ALCcoreAudioCapture, ALCbackend, ALCboolean, reset)
static ALCboolean ALCcoreAudioCapture_start(ALCcoreAudioCapture *self);
static void ALCcoreAudioCapture_stop(ALCcoreAudioCapture *self);
static ALCenum ALCcoreAudioCapture_captureSamples(ALCcoreAudioCapture *self, ALCvoid *buffer, ALCuint samples);
static ALCuint ALCcoreAudioCapture_availableSamples(ALCcoreAudioCapture *self);
static DECLARE_FORWARD(ALCcoreAudioCapture, ALCbackend, ClockLatency, getClockLatency)
static DECLARE_FORWARD(ALCcoreAudioCapture, ALCbackend, void, lock)
static DECLARE_FORWARD(ALCcoreAudioCapture, ALCbackend, void, unlock)
DECLARE_DEFAULT_ALLOCATORS(ALCcoreAudioCapture)

DEFINE_ALCBACKEND_VTABLE(ALCcoreAudioCapture);


static AudioBufferList *allocate_buffer_list(UInt32 channelCount, UInt32 byteSize)
{
    AudioBufferList *list;

    list = calloc(1, FAM_SIZE(AudioBufferList, mBuffers, 1) + byteSize);
    if(list)
    {
        list->mNumberBuffers = 1;

        list->mBuffers[0].mNumberChannels = channelCount;
        list->mBuffers[0].mDataByteSize = byteSize;
        list->mBuffers[0].mData = &list->mBuffers[1];
    }
    return list;
}

static void destroy_buffer_list(AudioBufferList *list)
{
    free(list);
}


static void ALCcoreAudioCapture_Construct(ALCcoreAudioCapture *self, ALCdevice *device)
{
    ALCbackend_Construct(STATIC_CAST(ALCbackend, self), device);
    SET_VTABLE2(ALCcoreAudioCapture, ALCbackend, self);

    self->audioUnit = 0;
    self->audioConverter = NULL;
    self->bufferList = NULL;
    self->resampleBuffer = NULL;
    self->ring = NULL;
}

static void ALCcoreAudioCapture_Destruct(ALCcoreAudioCapture *self)
{
    ll_ringbuffer_free(self->ring);
    self->ring = NULL;

    free(self->resampleBuffer);
    self->resampleBuffer = NULL;

    destroy_buffer_list(self->bufferList);
    self->bufferList = NULL;

    if(self->audioConverter)
        AudioConverterDispose(self->audioConverter);
    self->audioConverter = NULL;

    if(self->audioUnit)
        AudioComponentInstanceDispose(self->audioUnit);
    self->audioUnit = 0;

    ALCbackend_Destruct(STATIC_CAST(ALCbackend, self));
}


static OSStatus ALCcoreAudioCapture_RecordProc(void *inRefCon,
  AudioUnitRenderActionFlags* UNUSED(ioActionFlags),
  const AudioTimeStamp *inTimeStamp, UInt32 UNUSED(inBusNumber),
  UInt32 inNumberFrames, AudioBufferList* UNUSED(ioData))
{
    ALCcoreAudioCapture *self = inRefCon;
    AudioUnitRenderActionFlags flags = 0;
    OSStatus err;

    // fill the bufferList with data from the input device
    err = AudioUnitRender(self->audioUnit, &flags, inTimeStamp, 1, inNumberFrames, self->bufferList);
    if(err != noErr)
    {
        ERR("AudioUnitRender error: %d\n", err);
        return err;
    }

    ll_ringbuffer_write(self->ring, self->bufferList->mBuffers[0].mData, inNumberFrames);

    return noErr;
}

static OSStatus ALCcoreAudioCapture_ConvertCallback(AudioConverterRef UNUSED(inAudioConverter),
  UInt32 *ioNumberDataPackets, AudioBufferList *ioData,
  AudioStreamPacketDescription** UNUSED(outDataPacketDescription),
  void *inUserData)
{
    ALCcoreAudioCapture *self = inUserData;

    // Read from the ring buffer and store temporarily in a large buffer
    ll_ringbuffer_read(self->ring, self->resampleBuffer, *ioNumberDataPackets);

    // Set the input data
    ioData->mNumberBuffers = 1;
    ioData->mBuffers[0].mNumberChannels = self->format.mChannelsPerFrame;
    ioData->mBuffers[0].mData = self->resampleBuffer;
    ioData->mBuffers[0].mDataByteSize = (*ioNumberDataPackets) * self->format.mBytesPerFrame;

    return noErr;
}


static ALCenum ALCcoreAudioCapture_open(ALCcoreAudioCapture *self, const ALCchar *name)
{
    ALCdevice *device = STATIC_CAST(ALCbackend,self)->mDevice;
    AudioStreamBasicDescription requestedFormat;  // The application requested format
    AudioStreamBasicDescription hardwareFormat;   // The hardware format
    AudioStreamBasicDescription outputFormat;     // The AudioUnit output format
    AURenderCallbackStruct input;
    AudioComponentDescription desc;
    AudioDeviceID inputDevice;
    UInt32 outputFrameCount;
    UInt32 propertySize;
    AudioObjectPropertyAddress propertyAddress;
    UInt32 enableIO;
    AudioComponent comp;
    OSStatus err;

    if(!name)
        name = ca_device;
    else if(strcmp(name, ca_device) != 0)
        return ALC_INVALID_VALUE;

    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_HALOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    // Search for component with given description
    comp = AudioComponentFindNext(NULL, &desc);
    if(comp == NULL)
    {
        ERR("AudioComponentFindNext failed\n");
        return ALC_INVALID_VALUE;
    }

    // Open the component
    err = AudioComponentInstanceNew(comp, &self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioComponentInstanceNew failed\n");
        goto error;
    }

    // Turn off AudioUnit output
    enableIO = 0;
    err = AudioUnitSetProperty(self->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableIO, sizeof(ALuint));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        goto error;
    }

    // Turn on AudioUnit input
    enableIO = 1;
    err = AudioUnitSetProperty(self->audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableIO, sizeof(ALuint));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        goto error;
    }

    // Get the default input device

    propertySize = sizeof(AudioDeviceID);
    propertyAddress.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, &inputDevice);
    if(err != noErr)
    {
        ERR("AudioObjectGetPropertyData failed\n");
        goto error;
    }

    if(inputDevice == kAudioDeviceUnknown)
    {
        ERR("No input device found\n");
        goto error;
    }

    // Track the input device
    err = AudioUnitSetProperty(self->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &inputDevice, sizeof(AudioDeviceID));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        goto error;
    }

    // set capture callback
    input.inputProc = ALCcoreAudioCapture_RecordProc;
    input.inputProcRefCon = self;

    err = AudioUnitSetProperty(self->audioUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &input, sizeof(AURenderCallbackStruct));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        goto error;
    }

    // Initialize the device
    err = AudioUnitInitialize(self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioUnitInitialize failed\n");
        goto error;
    }

    // Get the hardware format
    propertySize = sizeof(AudioStreamBasicDescription);
    err = AudioUnitGetProperty(self->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &hardwareFormat, &propertySize);
    if(err != noErr || propertySize != sizeof(AudioStreamBasicDescription))
    {
        ERR("AudioUnitGetProperty failed\n");
        goto error;
    }

    // Set up the requested format description
    switch(device->FmtType)
    {
        case DevFmtUByte:
            requestedFormat.mBitsPerChannel = 8;
            requestedFormat.mFormatFlags = kAudioFormatFlagIsPacked;
            break;
        case DevFmtShort:
            requestedFormat.mBitsPerChannel = 16;
            requestedFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            break;
        case DevFmtInt:
            requestedFormat.mBitsPerChannel = 32;
            requestedFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            break;
        case DevFmtFloat:
            requestedFormat.mBitsPerChannel = 32;
            requestedFormat.mFormatFlags = kAudioFormatFlagIsPacked;
            break;
        case DevFmtByte:
        case DevFmtUShort:
        case DevFmtUInt:
            ERR("%s samples not supported\n", DevFmtTypeString(device->FmtType));
            goto error;
    }

    switch(device->FmtChans)
    {
        case DevFmtMono:
            requestedFormat.mChannelsPerFrame = 1;
            break;
        case DevFmtStereo:
            requestedFormat.mChannelsPerFrame = 2;
            break;

        case DevFmtQuad:
        case DevFmtX51:
        case DevFmtX51Rear:
        case DevFmtX61:
        case DevFmtX71:
        case DevFmtAmbi3D:
            ERR("%s not supported\n", DevFmtChannelsString(device->FmtChans));
            goto error;
    }

    requestedFormat.mBytesPerFrame = requestedFormat.mChannelsPerFrame * requestedFormat.mBitsPerChannel / 8;
    requestedFormat.mBytesPerPacket = requestedFormat.mBytesPerFrame;
    requestedFormat.mSampleRate = device->Frequency;
    requestedFormat.mFormatID = kAudioFormatLinearPCM;
    requestedFormat.mReserved = 0;
    requestedFormat.mFramesPerPacket = 1;

    // save requested format description for later use
    self->format = requestedFormat;
    self->frameSize = FrameSizeFromDevFmt(device->FmtChans, device->FmtType, device->AmbiOrder);

    // Use intermediate format for sample rate conversion (outputFormat)
    // Set sample rate to the same as hardware for resampling later
    outputFormat = requestedFormat;
    outputFormat.mSampleRate = hardwareFormat.mSampleRate;

    // Determine sample rate ratio for resampling
    self->sampleRateRatio = outputFormat.mSampleRate / device->Frequency;

    // The output format should be the requested format, but using the hardware sample rate
    // This is because the AudioUnit will automatically scale other properties, except for sample rate
    err = AudioUnitSetProperty(self->audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, (void *)&outputFormat, sizeof(outputFormat));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed\n");
        goto error;
    }

    // Set the AudioUnit output format frame count
    outputFrameCount = device->UpdateSize * self->sampleRateRatio;
    err = AudioUnitSetProperty(self->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Output, 0, &outputFrameCount, sizeof(outputFrameCount));
    if(err != noErr)
    {
        ERR("AudioUnitSetProperty failed: %d\n", err);
        goto error;
    }

    // Set up sample converter
    err = AudioConverterNew(&outputFormat, &requestedFormat, &self->audioConverter);
    if(err != noErr)
    {
        ERR("AudioConverterNew failed: %d\n", err);
        goto error;
    }

    // Create a buffer for use in the resample callback
    self->resampleBuffer = malloc(device->UpdateSize * self->frameSize * self->sampleRateRatio);

    // Allocate buffer for the AudioUnit output
    self->bufferList = allocate_buffer_list(outputFormat.mChannelsPerFrame, device->UpdateSize * self->frameSize * self->sampleRateRatio);
    if(self->bufferList == NULL)
        goto error;

    self->ring = ll_ringbuffer_create(
        (size_t)ceil(device->UpdateSize*self->sampleRateRatio*device->NumUpdates),
        self->frameSize, false
    );
    if(!self->ring) goto error;

    alstr_copy_cstr(&device->DeviceName, name);

    return ALC_NO_ERROR;

error:
    ll_ringbuffer_free(self->ring);
    self->ring = NULL;
    free(self->resampleBuffer);
    self->resampleBuffer = NULL;
    destroy_buffer_list(self->bufferList);
    self->bufferList = NULL;

    if(self->audioConverter)
        AudioConverterDispose(self->audioConverter);
    self->audioConverter = NULL;
    if(self->audioUnit)
        AudioComponentInstanceDispose(self->audioUnit);
    self->audioUnit = 0;

    return ALC_INVALID_VALUE;
}


static ALCboolean ALCcoreAudioCapture_start(ALCcoreAudioCapture *self)
{
    OSStatus err = AudioOutputUnitStart(self->audioUnit);
    if(err != noErr)
    {
        ERR("AudioOutputUnitStart failed\n");
        return ALC_FALSE;
    }
    return ALC_TRUE;
}

static void ALCcoreAudioCapture_stop(ALCcoreAudioCapture *self)
{
    OSStatus err = AudioOutputUnitStop(self->audioUnit);
    if(err != noErr)
        ERR("AudioOutputUnitStop failed\n");
}

static ALCenum ALCcoreAudioCapture_captureSamples(ALCcoreAudioCapture *self, ALCvoid *buffer, ALCuint samples)
{
    union {
        ALbyte _[sizeof(AudioBufferList) + sizeof(AudioBuffer)];
        AudioBufferList list;
    } audiobuf = { { 0 } };
    UInt32 frameCount;
    OSStatus err;

    // If no samples are requested, just return
    if(samples == 0) return ALC_NO_ERROR;

    // Point the resampling buffer to the capture buffer
    audiobuf.list.mNumberBuffers = 1;
    audiobuf.list.mBuffers[0].mNumberChannels = self->format.mChannelsPerFrame;
    audiobuf.list.mBuffers[0].mDataByteSize = samples * self->frameSize;
    audiobuf.list.mBuffers[0].mData = buffer;

    // Resample into another AudioBufferList
    frameCount = samples;
    err = AudioConverterFillComplexBuffer(self->audioConverter,
        ALCcoreAudioCapture_ConvertCallback, self, &frameCount, &audiobuf.list, NULL
    );
    if(err != noErr)
    {
        ERR("AudioConverterFillComplexBuffer error: %d\n", err);
        return ALC_INVALID_VALUE;
    }
    return ALC_NO_ERROR;
}

static ALCuint ALCcoreAudioCapture_availableSamples(ALCcoreAudioCapture *self)
{
    return ll_ringbuffer_read_space(self->ring) / self->sampleRateRatio;
}


typedef struct ALCcoreAudioBackendFactory {
    DERIVE_FROM_TYPE(ALCbackendFactory);
} ALCcoreAudioBackendFactory;
#define ALCCOREAUDIOBACKENDFACTORY_INITIALIZER { { GET_VTABLE2(ALCcoreAudioBackendFactory, ALCbackendFactory) } }

ALCbackendFactory *ALCcoreAudioBackendFactory_getFactory(void);

static ALCboolean ALCcoreAudioBackendFactory_init(ALCcoreAudioBackendFactory *self);
static DECLARE_FORWARD(ALCcoreAudioBackendFactory, ALCbackendFactory, void, deinit)
static ALCboolean ALCcoreAudioBackendFactory_querySupport(ALCcoreAudioBackendFactory *self, ALCbackend_Type type);
static void ALCcoreAudioBackendFactory_probe(ALCcoreAudioBackendFactory *self, enum DevProbe type);
static ALCbackend* ALCcoreAudioBackendFactory_createBackend(ALCcoreAudioBackendFactory *self, ALCdevice *device, ALCbackend_Type type);
DEFINE_ALCBACKENDFACTORY_VTABLE(ALCcoreAudioBackendFactory);


ALCbackendFactory *ALCcoreAudioBackendFactory_getFactory(void)
{
    static ALCcoreAudioBackendFactory factory = ALCCOREAUDIOBACKENDFACTORY_INITIALIZER;
    return STATIC_CAST(ALCbackendFactory, &factory);
}


static ALCboolean ALCcoreAudioBackendFactory_init(ALCcoreAudioBackendFactory* UNUSED(self))
{
    return ALC_TRUE;
}

static ALCboolean ALCcoreAudioBackendFactory_querySupport(ALCcoreAudioBackendFactory* UNUSED(self), ALCbackend_Type type)
{
    if(type == ALCbackend_Playback || ALCbackend_Capture)
        return ALC_TRUE;
    return ALC_FALSE;
}

static void ALCcoreAudioBackendFactory_probe(ALCcoreAudioBackendFactory* UNUSED(self), enum DevProbe type)
{
    switch(type)
    {
        case ALL_DEVICE_PROBE:
            AppendAllDevicesList(ca_device);
            break;
        case CAPTURE_DEVICE_PROBE:
            AppendCaptureDeviceList(ca_device);
            break;
    }
}

static ALCbackend* ALCcoreAudioBackendFactory_createBackend(ALCcoreAudioBackendFactory* UNUSED(self), ALCdevice *device, ALCbackend_Type type)
{
    if(type == ALCbackend_Playback)
    {
        ALCcoreAudioPlayback *backend;
        NEW_OBJ(backend, ALCcoreAudioPlayback)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }
    if(type == ALCbackend_Capture)
    {
        ALCcoreAudioCapture *backend;
        NEW_OBJ(backend, ALCcoreAudioCapture)(device);
        if(!backend) return NULL;
        return STATIC_CAST(ALCbackend, backend);
    }

    return NULL;
}
