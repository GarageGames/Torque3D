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
#include <limits.h>
#include <math.h>
#include <float.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alError.h"
#include "alSource.h"
#include "alBuffer.h"
#include "alThunk.h"
#include "alAuxEffectSlot.h"

#include "backends/base.h"

#include "threads.h"
#include "almalloc.h"


extern inline void LockSourcesRead(ALCcontext *context);
extern inline void UnlockSourcesRead(ALCcontext *context);
extern inline void LockSourcesWrite(ALCcontext *context);
extern inline void UnlockSourcesWrite(ALCcontext *context);
extern inline struct ALsource *LookupSource(ALCcontext *context, ALuint id);
extern inline struct ALsource *RemoveSource(ALCcontext *context, ALuint id);

static void InitSourceParams(ALsource *Source);
static void DeinitSource(ALsource *source);
static void UpdateSourceProps(ALsource *source, ALuint num_sends);
static ALint64 GetSourceSampleOffset(ALsource *Source, ALCdevice *device, ALuint64 *clocktime);
static ALdouble GetSourceSecOffset(ALsource *Source, ALCdevice *device, ALuint64 *clocktime);
static ALdouble GetSourceOffset(ALsource *Source, ALenum name, ALCdevice *device);
static ALboolean GetSampleOffset(ALsource *Source, ALuint *offset, ALuint *frac);

typedef enum SourceProp {
    srcPitch = AL_PITCH,
    srcGain = AL_GAIN,
    srcMinGain = AL_MIN_GAIN,
    srcMaxGain = AL_MAX_GAIN,
    srcMaxDistance = AL_MAX_DISTANCE,
    srcRolloffFactor = AL_ROLLOFF_FACTOR,
    srcDopplerFactor = AL_DOPPLER_FACTOR,
    srcConeOuterGain = AL_CONE_OUTER_GAIN,
    srcSecOffset = AL_SEC_OFFSET,
    srcSampleOffset = AL_SAMPLE_OFFSET,
    srcByteOffset = AL_BYTE_OFFSET,
    srcConeInnerAngle = AL_CONE_INNER_ANGLE,
    srcConeOuterAngle = AL_CONE_OUTER_ANGLE,
    srcRefDistance = AL_REFERENCE_DISTANCE,

    srcPosition = AL_POSITION,
    srcVelocity = AL_VELOCITY,
    srcDirection = AL_DIRECTION,

    srcSourceRelative = AL_SOURCE_RELATIVE,
    srcLooping = AL_LOOPING,
    srcBuffer = AL_BUFFER,
    srcSourceState = AL_SOURCE_STATE,
    srcBuffersQueued = AL_BUFFERS_QUEUED,
    srcBuffersProcessed = AL_BUFFERS_PROCESSED,
    srcSourceType = AL_SOURCE_TYPE,

    /* ALC_EXT_EFX */
    srcConeOuterGainHF = AL_CONE_OUTER_GAINHF,
    srcAirAbsorptionFactor = AL_AIR_ABSORPTION_FACTOR,
    srcRoomRolloffFactor =  AL_ROOM_ROLLOFF_FACTOR,
    srcDirectFilterGainHFAuto = AL_DIRECT_FILTER_GAINHF_AUTO,
    srcAuxSendFilterGainAuto = AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,
    srcAuxSendFilterGainHFAuto = AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO,
    srcDirectFilter = AL_DIRECT_FILTER,
    srcAuxSendFilter = AL_AUXILIARY_SEND_FILTER,

    /* AL_SOFT_direct_channels */
    srcDirectChannelsSOFT = AL_DIRECT_CHANNELS_SOFT,

    /* AL_EXT_source_distance_model */
    srcDistanceModel = AL_DISTANCE_MODEL,

    srcByteLengthSOFT = AL_BYTE_LENGTH_SOFT,
    srcSampleLengthSOFT = AL_SAMPLE_LENGTH_SOFT,
    srcSecLengthSOFT = AL_SEC_LENGTH_SOFT,

    /* AL_SOFT_source_latency */
    srcSampleOffsetLatencySOFT = AL_SAMPLE_OFFSET_LATENCY_SOFT,
    srcSecOffsetLatencySOFT = AL_SEC_OFFSET_LATENCY_SOFT,

    /* AL_EXT_STEREO_ANGLES */
    srcAngles = AL_STEREO_ANGLES,

    /* AL_EXT_SOURCE_RADIUS */
    srcRadius = AL_SOURCE_RADIUS,

    /* AL_EXT_BFORMAT */
    srcOrientation = AL_ORIENTATION,
} SourceProp;

static ALboolean SetSourcefv(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALfloat *values);
static ALboolean SetSourceiv(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALint *values);
static ALboolean SetSourcei64v(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALint64SOFT *values);

static ALboolean GetSourcedv(ALsource *Source, ALCcontext *Context, SourceProp prop, ALdouble *values);
static ALboolean GetSourceiv(ALsource *Source, ALCcontext *Context, SourceProp prop, ALint *values);
static ALboolean GetSourcei64v(ALsource *Source, ALCcontext *Context, SourceProp prop, ALint64 *values);

static inline bool SourceShouldUpdate(const ALsource *source, const ALCcontext *context)
{
    return (source->state == AL_PLAYING || source->state == AL_PAUSED) &&
           !ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire);
}

static ALint FloatValsByProp(ALenum prop)
{
    if(prop != (ALenum)((SourceProp)prop))
        return 0;
    switch((SourceProp)prop)
    {
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            return 1;

        case AL_STEREO_ANGLES:
            return 2;

        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            return 3;

        case AL_ORIENTATION:
            return 6;

        case AL_SEC_OFFSET_LATENCY_SOFT:
            break; /* Double only */

        case AL_BUFFER:
        case AL_DIRECT_FILTER:
        case AL_AUXILIARY_SEND_FILTER:
            break; /* i/i64 only */
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break; /* i64 only */
    }
    return 0;
}
static ALint DoubleValsByProp(ALenum prop)
{
    if(prop != (ALenum)((SourceProp)prop))
        return 0;
    switch((SourceProp)prop)
    {
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            return 1;

        case AL_SEC_OFFSET_LATENCY_SOFT:
        case AL_STEREO_ANGLES:
            return 2;

        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            return 3;

        case AL_ORIENTATION:
            return 6;

        case AL_BUFFER:
        case AL_DIRECT_FILTER:
        case AL_AUXILIARY_SEND_FILTER:
            break; /* i/i64 only */
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break; /* i64 only */
    }
    return 0;
}

static ALint IntValsByProp(ALenum prop)
{
    if(prop != (ALenum)((SourceProp)prop))
        return 0;
    switch((SourceProp)prop)
    {
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_DIRECT_FILTER:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            return 1;

        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
        case AL_AUXILIARY_SEND_FILTER:
            return 3;

        case AL_ORIENTATION:
            return 6;

        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break; /* i64 only */
        case AL_SEC_OFFSET_LATENCY_SOFT:
            break; /* Double only */
        case AL_STEREO_ANGLES:
            break; /* Float/double only */
    }
    return 0;
}
static ALint Int64ValsByProp(ALenum prop)
{
    if(prop != (ALenum)((SourceProp)prop))
        return 0;
    switch((SourceProp)prop)
    {
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_DIRECT_FILTER:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            return 1;

        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            return 2;

        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
        case AL_AUXILIARY_SEND_FILTER:
            return 3;

        case AL_ORIENTATION:
            return 6;

        case AL_SEC_OFFSET_LATENCY_SOFT:
            break; /* Double only */
        case AL_STEREO_ANGLES:
            break; /* Float/double only */
    }
    return 0;
}


#define CHECKVAL(x) do {                                                      \
    if(!(x))                                                                  \
        SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);      \
} while(0)

#define DO_UPDATEPROPS() do {                                                 \
    if(SourceShouldUpdate(Source, Context))                                   \
        UpdateSourceProps(Source, device->NumAuxSends);                       \
} while(0)

static ALboolean SetSourcefv(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALfloat *values)
{
    ALCdevice *device = Context->Device;
    ALint ival;

    switch(prop)
    {
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
        case AL_SEC_OFFSET_LATENCY_SOFT:
            /* Query only */
            SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_OPERATION, AL_FALSE);

        case AL_PITCH:
            CHECKVAL(*values >= 0.0f);

            Source->Pitch = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_CONE_INNER_ANGLE:
            CHECKVAL(*values >= 0.0f && *values <= 360.0f);

            Source->InnerAngle = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_CONE_OUTER_ANGLE:
            CHECKVAL(*values >= 0.0f && *values <= 360.0f);

            Source->OuterAngle = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_GAIN:
            CHECKVAL(*values >= 0.0f);

            Source->Gain = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_MAX_DISTANCE:
            CHECKVAL(*values >= 0.0f);

            Source->MaxDistance = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_ROLLOFF_FACTOR:
            CHECKVAL(*values >= 0.0f);

            Source->RollOffFactor = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_REFERENCE_DISTANCE:
            CHECKVAL(*values >= 0.0f);

            Source->RefDistance = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_MIN_GAIN:
            CHECKVAL(*values >= 0.0f);

            Source->MinGain = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_MAX_GAIN:
            CHECKVAL(*values >= 0.0f);

            Source->MaxGain = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_CONE_OUTER_GAIN:
            CHECKVAL(*values >= 0.0f && *values <= 1.0f);

            Source->OuterGain = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_CONE_OUTER_GAINHF:
            CHECKVAL(*values >= 0.0f && *values <= 1.0f);

            Source->OuterGainHF = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_AIR_ABSORPTION_FACTOR:
            CHECKVAL(*values >= 0.0f && *values <= 10.0f);

            Source->AirAbsorptionFactor = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_ROOM_ROLLOFF_FACTOR:
            CHECKVAL(*values >= 0.0f && *values <= 10.0f);

            Source->RoomRolloffFactor = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_DOPPLER_FACTOR:
            CHECKVAL(*values >= 0.0f && *values <= 1.0f);

            Source->DopplerFactor = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            CHECKVAL(*values >= 0.0f);

            Source->OffsetType = prop;
            Source->Offset = *values;

            if((Source->state == AL_PLAYING || Source->state == AL_PAUSED) &&
               !ATOMIC_LOAD(&Context->DeferUpdates, almemory_order_acquire))
            {
                LockContext(Context);
                WriteLock(&Source->queue_lock);
                if(ApplyOffset(Source) == AL_FALSE)
                {
                    WriteUnlock(&Source->queue_lock);
                    UnlockContext(Context);
                    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);
                }
                WriteUnlock(&Source->queue_lock);
                UnlockContext(Context);
            }
            return AL_TRUE;

        case AL_SOURCE_RADIUS:
            CHECKVAL(*values >= 0.0f && isfinite(*values));

            Source->Radius = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_STEREO_ANGLES:
            CHECKVAL(isfinite(values[0]) && isfinite(values[1]));

            Source->StereoPan[0] = values[0];
            Source->StereoPan[1] = values[1];
            DO_UPDATEPROPS();
            return AL_TRUE;


        case AL_POSITION:
            CHECKVAL(isfinite(values[0]) && isfinite(values[1]) && isfinite(values[2]));

            Source->Position[0] = values[0];
            Source->Position[1] = values[1];
            Source->Position[2] = values[2];
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_VELOCITY:
            CHECKVAL(isfinite(values[0]) && isfinite(values[1]) && isfinite(values[2]));

            Source->Velocity[0] = values[0];
            Source->Velocity[1] = values[1];
            Source->Velocity[2] = values[2];
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_DIRECTION:
            CHECKVAL(isfinite(values[0]) && isfinite(values[1]) && isfinite(values[2]));

            Source->Direction[0] = values[0];
            Source->Direction[1] = values[1];
            Source->Direction[2] = values[2];
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_ORIENTATION:
            CHECKVAL(isfinite(values[0]) && isfinite(values[1]) && isfinite(values[2]) &&
                     isfinite(values[3]) && isfinite(values[4]) && isfinite(values[5]));

            Source->Orientation[0][0] = values[0];
            Source->Orientation[0][1] = values[1];
            Source->Orientation[0][2] = values[2];
            Source->Orientation[1][0] = values[3];
            Source->Orientation[1][1] = values[4];
            Source->Orientation[1][2] = values[5];
            DO_UPDATEPROPS();
            return AL_TRUE;


        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SOURCE_STATE:
        case AL_SOURCE_TYPE:
        case AL_DISTANCE_MODEL:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
            ival = (ALint)values[0];
            return SetSourceiv(Source, Context, prop, &ival);

        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
            ival = (ALint)((ALuint)values[0]);
            return SetSourceiv(Source, Context, prop, &ival);

        case AL_BUFFER:
        case AL_DIRECT_FILTER:
        case AL_AUXILIARY_SEND_FILTER:
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break;
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}

static ALboolean SetSourceiv(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALint *values)
{
    ALCdevice *device = Context->Device;
    ALbuffer  *buffer = NULL;
    ALfilter  *filter = NULL;
    ALeffectslot *slot = NULL;
    ALbufferlistitem *oldlist;
    ALbufferlistitem *newlist;
    ALfloat fvals[6];

    switch(prop)
    {
        case AL_SOURCE_STATE:
        case AL_SOURCE_TYPE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
            /* Query only */
            SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_OPERATION, AL_FALSE);

        case AL_SOURCE_RELATIVE:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            Source->HeadRelative = (ALboolean)*values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_LOOPING:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            WriteLock(&Source->queue_lock);
            ATOMIC_STORE(&Source->looping, *values);
            WriteUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_BUFFER:
            LockBuffersRead(device);
            if(!(*values == 0 || (buffer=LookupBuffer(device, *values)) != NULL))
            {
                UnlockBuffersRead(device);
                SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);
            }

            WriteLock(&Source->queue_lock);
            if(!(Source->state == AL_STOPPED || Source->state == AL_INITIAL))
            {
                WriteUnlock(&Source->queue_lock);
                UnlockBuffersRead(device);
                SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_OPERATION, AL_FALSE);
            }

            if(buffer != NULL)
            {
                /* Add the selected buffer to a one-item queue */
                newlist = malloc(sizeof(ALbufferlistitem));
                newlist->buffer = buffer;
                newlist->next = NULL;
                IncrementRef(&buffer->ref);

                /* Source is now Static */
                Source->SourceType = AL_STATIC;

                ReadLock(&buffer->lock);
                Source->NumChannels = ChannelsFromFmt(buffer->FmtChannels);
                Source->SampleSize  = BytesFromFmt(buffer->FmtType);
                ReadUnlock(&buffer->lock);
            }
            else
            {
                /* Source is now Undetermined */
                Source->SourceType = AL_UNDETERMINED;
                newlist = NULL;
            }
            oldlist = ATOMIC_EXCHANGE(ALbufferlistitem*, &Source->queue, newlist);
            ATOMIC_STORE(&Source->current_buffer, newlist);
            WriteUnlock(&Source->queue_lock);
            UnlockBuffersRead(device);

            /* Delete all elements in the previous queue */
            while(oldlist != NULL)
            {
                ALbufferlistitem *temp = oldlist;
                oldlist = temp->next;

                if(temp->buffer)
                    DecrementRef(&temp->buffer->ref);
                free(temp);
            }
            return AL_TRUE;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            CHECKVAL(*values >= 0);

            Source->OffsetType = prop;
            Source->Offset = *values;

            if((Source->state == AL_PLAYING || Source->state == AL_PAUSED) &&
                !ATOMIC_LOAD(&Context->DeferUpdates, almemory_order_acquire))
            {
                LockContext(Context);
                WriteLock(&Source->queue_lock);
                if(ApplyOffset(Source) == AL_FALSE)
                {
                    WriteUnlock(&Source->queue_lock);
                    UnlockContext(Context);
                    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);
                }
                WriteUnlock(&Source->queue_lock);
                UnlockContext(Context);
            }
            return AL_TRUE;

        case AL_DIRECT_FILTER:
            LockFiltersRead(device);
            if(!(*values == 0 || (filter=LookupFilter(device, *values)) != NULL))
            {
                UnlockFiltersRead(device);
                SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);
            }

            if(!filter)
            {
                Source->Direct.Gain = 1.0f;
                Source->Direct.GainHF = 1.0f;
                Source->Direct.HFReference = LOWPASSFREQREF;
                Source->Direct.GainLF = 1.0f;
                Source->Direct.LFReference = HIGHPASSFREQREF;
            }
            else
            {
                Source->Direct.Gain = filter->Gain;
                Source->Direct.GainHF = filter->GainHF;
                Source->Direct.HFReference = filter->HFReference;
                Source->Direct.GainLF = filter->GainLF;
                Source->Direct.LFReference = filter->LFReference;
            }
            UnlockFiltersRead(device);
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_DIRECT_FILTER_GAINHF_AUTO:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            Source->DryGainHFAuto = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            Source->WetGainAuto = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            Source->WetGainHFAuto = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_DIRECT_CHANNELS_SOFT:
            CHECKVAL(*values == AL_FALSE || *values == AL_TRUE);

            Source->DirectChannels = *values;
            DO_UPDATEPROPS();
            return AL_TRUE;

        case AL_DISTANCE_MODEL:
            CHECKVAL(*values == AL_NONE ||
                     *values == AL_INVERSE_DISTANCE ||
                     *values == AL_INVERSE_DISTANCE_CLAMPED ||
                     *values == AL_LINEAR_DISTANCE ||
                     *values == AL_LINEAR_DISTANCE_CLAMPED ||
                     *values == AL_EXPONENT_DISTANCE ||
                     *values == AL_EXPONENT_DISTANCE_CLAMPED);

            Source->DistanceModel = *values;
            if(Context->SourceDistanceModel)
                DO_UPDATEPROPS();
            return AL_TRUE;


        case AL_AUXILIARY_SEND_FILTER:
            LockEffectSlotsRead(Context);
            LockFiltersRead(device);
            if(!((ALuint)values[1] < device->NumAuxSends &&
                 (values[0] == 0 || (slot=LookupEffectSlot(Context, values[0])) != NULL) &&
                 (values[2] == 0 || (filter=LookupFilter(device, values[2])) != NULL)))
            {
                UnlockFiltersRead(device);
                UnlockEffectSlotsRead(Context);
                SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_VALUE, AL_FALSE);
            }

            if(!filter)
            {
                /* Disable filter */
                Source->Send[values[1]].Gain = 1.0f;
                Source->Send[values[1]].GainHF = 1.0f;
                Source->Send[values[1]].HFReference = LOWPASSFREQREF;
                Source->Send[values[1]].GainLF = 1.0f;
                Source->Send[values[1]].LFReference = HIGHPASSFREQREF;
            }
            else
            {
                Source->Send[values[1]].Gain = filter->Gain;
                Source->Send[values[1]].GainHF = filter->GainHF;
                Source->Send[values[1]].HFReference = filter->HFReference;
                Source->Send[values[1]].GainLF = filter->GainLF;
                Source->Send[values[1]].LFReference = filter->LFReference;
            }
            UnlockFiltersRead(device);

            if(slot != Source->Send[values[1]].Slot &&
               (Source->state == AL_PLAYING || Source->state == AL_PAUSED))
            {
                /* Add refcount on the new slot, and release the previous slot */
                if(slot) IncrementRef(&slot->ref);
                if(Source->Send[values[1]].Slot)
                    DecrementRef(&Source->Send[values[1]].Slot->ref);
                Source->Send[values[1]].Slot = slot;

                /* We must force an update if the auxiliary slot changed on a
                 * playing source, in case the slot is about to be deleted.
                 */
                UpdateSourceProps(Source, device->NumAuxSends);
            }
            else
            {
                if(slot) IncrementRef(&slot->ref);
                if(Source->Send[values[1]].Slot)
                    DecrementRef(&Source->Send[values[1]].Slot->ref);
                Source->Send[values[1]].Slot = slot;
                DO_UPDATEPROPS();
            }
            UnlockEffectSlotsRead(Context);

            return AL_TRUE;


        /* 1x float */
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_MAX_DISTANCE:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_SOURCE_RADIUS:
            fvals[0] = (ALfloat)*values;
            return SetSourcefv(Source, Context, (int)prop, fvals);

        /* 3x float */
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            fvals[0] = (ALfloat)values[0];
            fvals[1] = (ALfloat)values[1];
            fvals[2] = (ALfloat)values[2];
            return SetSourcefv(Source, Context, (int)prop, fvals);

        /* 6x float */
        case AL_ORIENTATION:
            fvals[0] = (ALfloat)values[0];
            fvals[1] = (ALfloat)values[1];
            fvals[2] = (ALfloat)values[2];
            fvals[3] = (ALfloat)values[3];
            fvals[4] = (ALfloat)values[4];
            fvals[5] = (ALfloat)values[5];
            return SetSourcefv(Source, Context, (int)prop, fvals);

        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
        case AL_SEC_OFFSET_LATENCY_SOFT:
        case AL_STEREO_ANGLES:
            break;
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}

static ALboolean SetSourcei64v(ALsource *Source, ALCcontext *Context, SourceProp prop, const ALint64SOFT *values)
{
    ALfloat fvals[6];
    ALint   ivals[3];

    switch(prop)
    {
        case AL_SOURCE_TYPE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_STATE:
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SEC_LENGTH_SOFT:
            /* Query only */
            SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_OPERATION, AL_FALSE);


        /* 1x int */
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
            CHECKVAL(*values <= INT_MAX && *values >= INT_MIN);

            ivals[0] = (ALint)*values;
            return SetSourceiv(Source, Context, (int)prop, ivals);

        /* 1x uint */
        case AL_BUFFER:
        case AL_DIRECT_FILTER:
            CHECKVAL(*values <= UINT_MAX && *values >= 0);

            ivals[0] = (ALuint)*values;
            return SetSourceiv(Source, Context, (int)prop, ivals);

        /* 3x uint */
        case AL_AUXILIARY_SEND_FILTER:
            CHECKVAL(values[0] <= UINT_MAX && values[0] >= 0 &&
                     values[1] <= UINT_MAX && values[1] >= 0 &&
                     values[2] <= UINT_MAX && values[2] >= 0);

            ivals[0] = (ALuint)values[0];
            ivals[1] = (ALuint)values[1];
            ivals[2] = (ALuint)values[2];
            return SetSourceiv(Source, Context, (int)prop, ivals);

        /* 1x float */
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_MAX_DISTANCE:
        case AL_DOPPLER_FACTOR:
        case AL_CONE_OUTER_GAINHF:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_SOURCE_RADIUS:
            fvals[0] = (ALfloat)*values;
            return SetSourcefv(Source, Context, (int)prop, fvals);

        /* 3x float */
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            fvals[0] = (ALfloat)values[0];
            fvals[1] = (ALfloat)values[1];
            fvals[2] = (ALfloat)values[2];
            return SetSourcefv(Source, Context, (int)prop, fvals);

        /* 6x float */
        case AL_ORIENTATION:
            fvals[0] = (ALfloat)values[0];
            fvals[1] = (ALfloat)values[1];
            fvals[2] = (ALfloat)values[2];
            fvals[3] = (ALfloat)values[3];
            fvals[4] = (ALfloat)values[4];
            fvals[5] = (ALfloat)values[5];
            return SetSourcefv(Source, Context, (int)prop, fvals);

        case AL_SEC_OFFSET_LATENCY_SOFT:
        case AL_STEREO_ANGLES:
            break;
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}

#undef CHECKVAL


static ALboolean GetSourcedv(ALsource *Source, ALCcontext *Context, SourceProp prop, ALdouble *values)
{
    ALCdevice *device = Context->Device;
    ALbufferlistitem *BufferList;
    ClockLatency clocktime;
    ALuint64 srcclock;
    ALint ivals[3];
    ALboolean err;

    switch(prop)
    {
        case AL_GAIN:
            *values = Source->Gain;
            return AL_TRUE;

        case AL_PITCH:
            *values = Source->Pitch;
            return AL_TRUE;

        case AL_MAX_DISTANCE:
            *values = Source->MaxDistance;
            return AL_TRUE;

        case AL_ROLLOFF_FACTOR:
            *values = Source->RollOffFactor;
            return AL_TRUE;

        case AL_REFERENCE_DISTANCE:
            *values = Source->RefDistance;
            return AL_TRUE;

        case AL_CONE_INNER_ANGLE:
            *values = Source->InnerAngle;
            return AL_TRUE;

        case AL_CONE_OUTER_ANGLE:
            *values = Source->OuterAngle;
            return AL_TRUE;

        case AL_MIN_GAIN:
            *values = Source->MinGain;
            return AL_TRUE;

        case AL_MAX_GAIN:
            *values = Source->MaxGain;
            return AL_TRUE;

        case AL_CONE_OUTER_GAIN:
            *values = Source->OuterGain;
            return AL_TRUE;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            *values = GetSourceOffset(Source, prop, device);
            return AL_TRUE;

        case AL_CONE_OUTER_GAINHF:
            *values = Source->OuterGainHF;
            return AL_TRUE;

        case AL_AIR_ABSORPTION_FACTOR:
            *values = Source->AirAbsorptionFactor;
            return AL_TRUE;

        case AL_ROOM_ROLLOFF_FACTOR:
            *values = Source->RoomRolloffFactor;
            return AL_TRUE;

        case AL_DOPPLER_FACTOR:
            *values = Source->DopplerFactor;
            return AL_TRUE;

        case AL_SEC_LENGTH_SOFT:
            ReadLock(&Source->queue_lock);
            if(!(BufferList=ATOMIC_LOAD(&Source->queue)))
                *values = 0;
            else
            {
                ALint length = 0;
                ALsizei freq = 1;
                do {
                    ALbuffer *buffer = BufferList->buffer;
                    if(buffer && buffer->SampleLen > 0)
                    {
                        freq = buffer->Frequency;
                        length += buffer->SampleLen;
                    }
                } while((BufferList=BufferList->next) != NULL);
                *values = (ALdouble)length / (ALdouble)freq;
            }
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_SOURCE_RADIUS:
            *values = Source->Radius;
            return AL_TRUE;

        case AL_STEREO_ANGLES:
            values[0] = Source->StereoPan[0];
            values[1] = Source->StereoPan[1];
            return AL_TRUE;

        case AL_SEC_OFFSET_LATENCY_SOFT:
            /* Get the source offset with the clock time first. Then get the
             * clock time with the device latency. Order is important.
             */
            values[0] = GetSourceSecOffset(Source, device, &srcclock);
            clocktime = V0(device->Backend,getClockLatency)();
            if(srcclock == (ALuint64)clocktime.ClockTime)
                values[1] = (ALdouble)clocktime.Latency / 1000000000.0;
            else
            {
                /* If the clock time incremented, reduce the latency by that
                 * much since it's that much closer to the source offset it got
                 * earlier.
                 */
                ALuint64 diff = clocktime.ClockTime - srcclock;
                values[1] = (ALdouble)(clocktime.Latency - minu64(clocktime.Latency, diff)) /
                            1000000000.0;
            }
            return AL_TRUE;

        case AL_POSITION:
            values[0] = Source->Position[0];
            values[1] = Source->Position[1];
            values[2] = Source->Position[2];
            return AL_TRUE;

        case AL_VELOCITY:
            values[0] = Source->Velocity[0];
            values[1] = Source->Velocity[1];
            values[2] = Source->Velocity[2];
            return AL_TRUE;

        case AL_DIRECTION:
            values[0] = Source->Direction[0];
            values[1] = Source->Direction[1];
            values[2] = Source->Direction[2];
            return AL_TRUE;

        case AL_ORIENTATION:
            values[0] = Source->Orientation[0][0];
            values[1] = Source->Orientation[0][1];
            values[2] = Source->Orientation[0][2];
            values[3] = Source->Orientation[1][0];
            values[4] = Source->Orientation[1][1];
            values[5] = Source->Orientation[1][2];
            return AL_TRUE;

        /* 1x int */
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_DISTANCE_MODEL:
            if((err=GetSourceiv(Source, Context, (int)prop, ivals)) != AL_FALSE)
                *values = (ALdouble)ivals[0];
            return err;

        case AL_BUFFER:
        case AL_DIRECT_FILTER:
        case AL_AUXILIARY_SEND_FILTER:
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break;
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}

static ALboolean GetSourceiv(ALsource *Source, ALCcontext *Context, SourceProp prop, ALint *values)
{
    ALbufferlistitem *BufferList;
    ALdouble dvals[6];
    ALboolean err;

    switch(prop)
    {
        case AL_SOURCE_RELATIVE:
            *values = Source->HeadRelative;
            return AL_TRUE;

        case AL_LOOPING:
            *values = ATOMIC_LOAD(&Source->looping);
            return AL_TRUE;

        case AL_BUFFER:
            ReadLock(&Source->queue_lock);
            BufferList = (Source->SourceType == AL_STATIC) ? ATOMIC_LOAD(&Source->queue) :
                                                             ATOMIC_LOAD(&Source->current_buffer);
            *values = (BufferList && BufferList->buffer) ? BufferList->buffer->id : 0;
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_SOURCE_STATE:
            *values = Source->state;
            return AL_TRUE;

        case AL_BYTE_LENGTH_SOFT:
            ReadLock(&Source->queue_lock);
            if(!(BufferList=ATOMIC_LOAD(&Source->queue)))
                *values = 0;
            else
            {
                ALint length = 0;
                do {
                    ALbuffer *buffer = BufferList->buffer;
                    if(buffer && buffer->SampleLen > 0)
                    {
                        ALuint byte_align, sample_align;
                        if(buffer->OriginalType == UserFmtIMA4)
                        {
                            ALsizei align = (buffer->OriginalAlign-1)/2 + 4;
                            byte_align = align * ChannelsFromFmt(buffer->FmtChannels);
                            sample_align = buffer->OriginalAlign;
                        }
                        else if(buffer->OriginalType == UserFmtMSADPCM)
                        {
                            ALsizei align = (buffer->OriginalAlign-2)/2 + 7;
                            byte_align = align * ChannelsFromFmt(buffer->FmtChannels);
                            sample_align = buffer->OriginalAlign;
                        }
                        else
                        {
                            ALsizei align = buffer->OriginalAlign;
                            byte_align = align * ChannelsFromFmt(buffer->FmtChannels);
                            sample_align = buffer->OriginalAlign;
                        }

                        length += buffer->SampleLen / sample_align * byte_align;
                    }
                } while((BufferList=BufferList->next) != NULL);
                *values = length;
            }
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_SAMPLE_LENGTH_SOFT:
            ReadLock(&Source->queue_lock);
            if(!(BufferList=ATOMIC_LOAD(&Source->queue)))
                *values = 0;
            else
            {
                ALint length = 0;
                do {
                    ALbuffer *buffer = BufferList->buffer;
                    if(buffer) length += buffer->SampleLen;
                } while((BufferList=BufferList->next) != NULL);
                *values = length;
            }
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_BUFFERS_QUEUED:
            ReadLock(&Source->queue_lock);
            if(!(BufferList=ATOMIC_LOAD(&Source->queue)))
                *values = 0;
            else
            {
                ALsizei count = 0;
                do {
                    ++count;
                } while((BufferList=BufferList->next) != NULL);
                *values = count;
            }
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_BUFFERS_PROCESSED:
            ReadLock(&Source->queue_lock);
            if(ATOMIC_LOAD(&Source->looping) || Source->SourceType != AL_STREAMING)
            {
                /* Buffers on a looping source are in a perpetual state of
                 * PENDING, so don't report any as PROCESSED */
                *values = 0;
            }
            else
            {
                const ALbufferlistitem *BufferList = ATOMIC_LOAD(&Source->queue);
                const ALbufferlistitem *Current = ATOMIC_LOAD(&Source->current_buffer);
                ALsizei played = 0;
                while(BufferList && BufferList != Current)
                {
                    played++;
                    BufferList = BufferList->next;
                }
                *values = played;
            }
            ReadUnlock(&Source->queue_lock);
            return AL_TRUE;

        case AL_SOURCE_TYPE:
            *values = Source->SourceType;
            return AL_TRUE;

        case AL_DIRECT_FILTER_GAINHF_AUTO:
            *values = Source->DryGainHFAuto;
            return AL_TRUE;

        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
            *values = Source->WetGainAuto;
            return AL_TRUE;

        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
            *values = Source->WetGainHFAuto;
            return AL_TRUE;

        case AL_DIRECT_CHANNELS_SOFT:
            *values = Source->DirectChannels;
            return AL_TRUE;

        case AL_DISTANCE_MODEL:
            *values = Source->DistanceModel;
            return AL_TRUE;

        /* 1x float/double */
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_MAX_DISTANCE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_DOPPLER_FACTOR:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAINHF:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
                *values = (ALint)dvals[0];
            return err;

        /* 3x float/double */
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
            {
                values[0] = (ALint)dvals[0];
                values[1] = (ALint)dvals[1];
                values[2] = (ALint)dvals[2];
            }
            return err;

        /* 6x float/double */
        case AL_ORIENTATION:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
            {
                values[0] = (ALint)dvals[0];
                values[1] = (ALint)dvals[1];
                values[2] = (ALint)dvals[2];
                values[3] = (ALint)dvals[3];
                values[4] = (ALint)dvals[4];
                values[5] = (ALint)dvals[5];
            }
            return err;

        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            break; /* i64 only */
        case AL_SEC_OFFSET_LATENCY_SOFT:
            break; /* Double only */
        case AL_STEREO_ANGLES:
            break; /* Float/double only */

        case AL_DIRECT_FILTER:
        case AL_AUXILIARY_SEND_FILTER:
            break; /* ??? */
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}

static ALboolean GetSourcei64v(ALsource *Source, ALCcontext *Context, SourceProp prop, ALint64 *values)
{
    ALCdevice *device = Context->Device;
    ClockLatency clocktime;
    ALuint64 srcclock;
    ALdouble dvals[6];
    ALint ivals[3];
    ALboolean err;

    switch(prop)
    {
        case AL_SAMPLE_OFFSET_LATENCY_SOFT:
            /* Get the source offset with the clock time first. Then get the
             * clock time with the device latency. Order is important.
             */
            values[0] = GetSourceSampleOffset(Source, device, &srcclock);
            clocktime = V0(device->Backend,getClockLatency)();
            if(srcclock == (ALuint64)clocktime.ClockTime)
                values[1] = clocktime.Latency;
            else
            {
                /* If the clock time incremented, reduce the latency by that
                 * much since it's that much closer to the source offset it got
                 * earlier.
                 */
                ALuint64 diff = clocktime.ClockTime - srcclock;
                values[1] = clocktime.Latency - minu64(clocktime.Latency, diff);
            }
            return AL_TRUE;

        /* 1x float/double */
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_PITCH:
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_MAX_DISTANCE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
        case AL_DOPPLER_FACTOR:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAINHF:
        case AL_SEC_LENGTH_SOFT:
        case AL_SOURCE_RADIUS:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
                *values = (ALint64)dvals[0];
            return err;

        /* 3x float/double */
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
            {
                values[0] = (ALint64)dvals[0];
                values[1] = (ALint64)dvals[1];
                values[2] = (ALint64)dvals[2];
            }
            return err;

        /* 6x float/double */
        case AL_ORIENTATION:
            if((err=GetSourcedv(Source, Context, prop, dvals)) != AL_FALSE)
            {
                values[0] = (ALint64)dvals[0];
                values[1] = (ALint64)dvals[1];
                values[2] = (ALint64)dvals[2];
                values[3] = (ALint64)dvals[3];
                values[4] = (ALint64)dvals[4];
                values[5] = (ALint64)dvals[5];
            }
            return err;

        /* 1x int */
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_SOURCE_STATE:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_BYTE_LENGTH_SOFT:
        case AL_SAMPLE_LENGTH_SOFT:
        case AL_SOURCE_TYPE:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
        case AL_DIRECT_CHANNELS_SOFT:
        case AL_DISTANCE_MODEL:
            if((err=GetSourceiv(Source, Context, prop, ivals)) != AL_FALSE)
                *values = ivals[0];
            return err;

        /* 1x uint */
        case AL_BUFFER:
        case AL_DIRECT_FILTER:
            if((err=GetSourceiv(Source, Context, prop, ivals)) != AL_FALSE)
                *values = (ALuint)ivals[0];
            return err;

        /* 3x uint */
        case AL_AUXILIARY_SEND_FILTER:
            if((err=GetSourceiv(Source, Context, prop, ivals)) != AL_FALSE)
            {
                values[0] = (ALuint)ivals[0];
                values[1] = (ALuint)ivals[1];
                values[2] = (ALuint)ivals[2];
            }
            return err;

        case AL_SEC_OFFSET_LATENCY_SOFT:
            break; /* Double only */
        case AL_STEREO_ANGLES:
            break; /* Float/double only */
    }

    ERR("Unexpected property: 0x%04x\n", prop);
    SET_ERROR_AND_RETURN_VALUE(Context, AL_INVALID_ENUM, AL_FALSE);
}


AL_API ALvoid AL_APIENTRY alGenSources(ALsizei n, ALuint *sources)
{
    ALCcontext *context;
    ALsizei cur = 0;
    ALenum err;

    context = GetContextRef();
    if(!context) return;

    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(cur = 0;cur < n;cur++)
    {
        ALsource *source = al_calloc(16, sizeof(ALsource));
        if(!source)
        {
            alDeleteSources(cur, sources);
            SET_ERROR_AND_GOTO(context, AL_OUT_OF_MEMORY, done);
        }
        InitSourceParams(source);

        err = NewThunkEntry(&source->id);
        if(err == AL_NO_ERROR)
            err = InsertUIntMapEntry(&context->SourceMap, source->id, source);
        if(err != AL_NO_ERROR)
        {
            FreeThunkEntry(source->id);
            memset(source, 0, sizeof(ALsource));
            al_free(source);

            alDeleteSources(cur, sources);
            SET_ERROR_AND_GOTO(context, err, done);
        }

        sources[cur] = source->id;
    }

done:
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alDeleteSources(ALsizei n, const ALuint *sources)
{
    ALCcontext *context;
    ALsource *Source;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockSourcesWrite(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    /* Check that all Sources are valid */
    for(i = 0;i < n;i++)
    {
        if(LookupSource(context, sources[i]) == NULL)
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    }
    for(i = 0;i < n;i++)
    {
        ALvoice *voice, *voice_end;

        if((Source=RemoveSource(context, sources[i])) == NULL)
            continue;
        FreeThunkEntry(Source->id);

        LockContext(context);
        voice = context->Voices;
        voice_end = voice + context->VoiceCount;
        while(voice != voice_end)
        {
            ALsource *old = Source;
            if(COMPARE_EXCHANGE(&voice->Source, &old, NULL))
                break;
            voice++;
        }
        UnlockContext(context);

        DeinitSource(Source);

        memset(Source, 0, sizeof(*Source));
        al_free(Source);
    }

done:
    UnlockSourcesWrite(context);
    ALCcontext_DecRef(context);
}


AL_API ALboolean AL_APIENTRY alIsSource(ALuint source)
{
    ALCcontext *context;
    ALboolean ret;

    context = GetContextRef();
    if(!context) return AL_FALSE;

    LockSourcesRead(context);
    ret = (LookupSource(context, source) ? AL_TRUE : AL_FALSE);
    UnlockSourcesRead(context);

    ALCcontext_DecRef(context);

    return ret;
}


AL_API ALvoid AL_APIENTRY alSourcef(ALuint source, ALenum param, ALfloat value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(FloatValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourcefv(Source, Context, param, &value);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(FloatValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALfloat fvals[3] = { value1, value2, value3 };
        SetSourcefv(Source, Context, param, fvals);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alSourcefv(ALuint source, ALenum param, const ALfloat *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(FloatValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourcefv(Source, Context, param, values);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alSourcedSOFT(ALuint source, ALenum param, ALdouble value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(DoubleValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALfloat fval = (ALfloat)value;
        SetSourcefv(Source, Context, param, &fval);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alSource3dSOFT(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(DoubleValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALfloat fvals[3] = { (ALfloat)value1, (ALfloat)value2, (ALfloat)value3 };
        SetSourcefv(Source, Context, param, fvals);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API ALvoid AL_APIENTRY alSourcedvSOFT(ALuint source, ALenum param, const ALdouble *values)
{
    ALCcontext *Context;
    ALsource   *Source;
    ALint      count;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!((count=DoubleValsByProp(param)) > 0 && count <= 6))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALfloat fvals[6];
        ALint i;

        for(i = 0;i < count;i++)
            fvals[i] = (ALfloat)values[i];
        SetSourcefv(Source, Context, param, fvals);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alSourcei(ALuint source, ALenum param, ALint value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(IntValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourceiv(Source, Context, param, &value);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(IntValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALint ivals[3] = { value1, value2, value3 };
        SetSourceiv(Source, Context, param, ivals);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alSourceiv(ALuint source, ALenum param, const ALint *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(IntValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourceiv(Source, Context, param, values);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(Int64ValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourcei64v(Source, Context, param, &value);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT value1, ALint64SOFT value2, ALint64SOFT value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(Int64ValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALint64SOFT i64vals[3] = { value1, value2, value3 };
        SetSourcei64v(Source, Context, param, i64vals);
    }
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alSourcei64vSOFT(ALuint source, ALenum param, const ALint64SOFT *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    WriteLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(Int64ValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        SetSourcei64v(Source, Context, param, values);
    UnlockSourcesRead(Context);
    WriteUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alGetSourcef(ALuint source, ALenum param, ALfloat *value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!value)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(FloatValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALdouble dval;
        if(GetSourcedv(Source, Context, param, &dval))
            *value = (ALfloat)dval;
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alGetSource3f(ALuint source, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(value1 && value2 && value3))
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(FloatValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALdouble dvals[3];
        if(GetSourcedv(Source, Context, param, dvals))
        {
            *value1 = (ALfloat)dvals[0];
            *value2 = (ALfloat)dvals[1];
            *value3 = (ALfloat)dvals[2];
        }
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alGetSourcefv(ALuint source, ALenum param, ALfloat *values)
{
    ALCcontext *Context;
    ALsource   *Source;
    ALint      count;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!((count=FloatValsByProp(param)) > 0 && count <= 6))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALdouble dvals[6];
        if(GetSourcedv(Source, Context, param, dvals))
        {
            ALint i;
            for(i = 0;i < count;i++)
                values[i] = (ALfloat)dvals[i];
        }
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API void AL_APIENTRY alGetSourcedSOFT(ALuint source, ALenum param, ALdouble *value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!value)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(DoubleValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourcedv(Source, Context, param, value);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alGetSource3dSOFT(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(value1 && value2 && value3))
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(DoubleValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALdouble dvals[3];
        if(GetSourcedv(Source, Context, param, dvals))
        {
            *value1 = dvals[0];
            *value2 = dvals[1];
            *value3 = dvals[2];
        }
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alGetSourcedvSOFT(ALuint source, ALenum param, ALdouble *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(DoubleValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourcedv(Source, Context, param, values);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alGetSourcei(ALuint source, ALenum param, ALint *value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!value)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(IntValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourceiv(Source, Context, param, value);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API void AL_APIENTRY alGetSource3i(ALuint source, ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(value1 && value2 && value3))
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(IntValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALint ivals[3];
        if(GetSourceiv(Source, Context, param, ivals))
        {
            *value1 = ivals[0];
            *value2 = ivals[1];
            *value3 = ivals[2];
        }
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API void AL_APIENTRY alGetSourceiv(ALuint source, ALenum param, ALint *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(IntValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourceiv(Source, Context, param, values);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API void AL_APIENTRY alGetSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT *value)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!value)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(Int64ValsByProp(param) == 1))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourcei64v(Source, Context, param, value);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alGetSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT *value1, ALint64SOFT *value2, ALint64SOFT *value3)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!(value1 && value2 && value3))
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(Int64ValsByProp(param) == 3))
        alSetError(Context, AL_INVALID_ENUM);
    else
    {
        ALint64 i64vals[3];
        if(GetSourcei64v(Source, Context, param, i64vals))
        {
            *value1 = i64vals[0];
            *value2 = i64vals[1];
            *value3 = i64vals[2];
        }
    }
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}

AL_API void AL_APIENTRY alGetSourcei64vSOFT(ALuint source, ALenum param, ALint64SOFT *values)
{
    ALCcontext *Context;
    ALsource   *Source;

    Context = GetContextRef();
    if(!Context) return;

    ReadLock(&Context->PropLock);
    LockSourcesRead(Context);
    if((Source=LookupSource(Context, source)) == NULL)
        alSetError(Context, AL_INVALID_NAME);
    else if(!values)
        alSetError(Context, AL_INVALID_VALUE);
    else if(!(Int64ValsByProp(param) > 0))
        alSetError(Context, AL_INVALID_ENUM);
    else
        GetSourcei64v(Source, Context, param, values);
    UnlockSourcesRead(Context);
    ReadUnlock(&Context->PropLock);

    ALCcontext_DecRef(Context);
}


AL_API ALvoid AL_APIENTRY alSourcePlay(ALuint source)
{
    alSourcePlayv(1, &source);
}
AL_API ALvoid AL_APIENTRY alSourcePlayv(ALsizei n, const ALuint *sources)
{
    ALCcontext *context;
    ALsource *source;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockSourcesRead(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(i = 0;i < n;i++)
    {
        if(!LookupSource(context, sources[i]))
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    }

    LockContext(context);
    while(n > context->MaxVoices-context->VoiceCount)
    {
        ALvoice *temp = NULL;
        ALsizei newcount;

        newcount = context->MaxVoices << 1;
        if(newcount > 0)
            temp = al_malloc(16, newcount * sizeof(context->Voices[0]));
        if(!temp)
        {
            UnlockContext(context);
            SET_ERROR_AND_GOTO(context, AL_OUT_OF_MEMORY, done);
        }
        memcpy(temp, context->Voices, context->MaxVoices * sizeof(temp[0]));
        memset(&temp[context->MaxVoices], 0, (newcount-context->MaxVoices) * sizeof(temp[0]));

        al_free(context->Voices);
        context->Voices = temp;
        context->MaxVoices = newcount;
    }

    if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire) == DeferAll)
    {
        for(i = 0;i < n;i++)
        {
            source = LookupSource(context, sources[i]);
            source->new_state = AL_PLAYING;
        }
    }
    else
    {
        for(i = 0;i < n;i++)
        {
            source = LookupSource(context, sources[i]);
            SetSourceState(source, context, AL_PLAYING);
        }
    }
    UnlockContext(context);

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSourcePause(ALuint source)
{
    alSourcePausev(1, &source);
}
AL_API ALvoid AL_APIENTRY alSourcePausev(ALsizei n, const ALuint *sources)
{
    ALCcontext *context;
    ALsource *source;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockSourcesRead(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(i = 0;i < n;i++)
    {
        if(!LookupSource(context, sources[i]))
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    }

    LockContext(context);
    if(ATOMIC_LOAD(&context->DeferUpdates, almemory_order_acquire))
    {
        for(i = 0;i < n;i++)
        {
            source = LookupSource(context, sources[i]);
            source->new_state = AL_PAUSED;
        }
    }
    else
    {
        for(i = 0;i < n;i++)
        {
            source = LookupSource(context, sources[i]);
            SetSourceState(source, context, AL_PAUSED);
        }
    }
    UnlockContext(context);

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSourceStop(ALuint source)
{
    alSourceStopv(1, &source);
}
AL_API ALvoid AL_APIENTRY alSourceStopv(ALsizei n, const ALuint *sources)
{
    ALCcontext *context;
    ALsource *source;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockSourcesRead(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(i = 0;i < n;i++)
    {
        if(!LookupSource(context, sources[i]))
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    }

    LockContext(context);
    for(i = 0;i < n;i++)
    {
        source = LookupSource(context, sources[i]);
        source->new_state = AL_NONE;
        SetSourceState(source, context, AL_STOPPED);
    }
    UnlockContext(context);

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSourceRewind(ALuint source)
{
    alSourceRewindv(1, &source);
}
AL_API ALvoid AL_APIENTRY alSourceRewindv(ALsizei n, const ALuint *sources)
{
    ALCcontext *context;
    ALsource *source;
    ALsizei i;

    context = GetContextRef();
    if(!context) return;

    LockSourcesRead(context);
    if(!(n >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    for(i = 0;i < n;i++)
    {
        if(!LookupSource(context, sources[i]))
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);
    }

    LockContext(context);
    for(i = 0;i < n;i++)
    {
        source = LookupSource(context, sources[i]);
        source->new_state = AL_NONE;
        SetSourceState(source, context, AL_INITIAL);
    }
    UnlockContext(context);

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}


AL_API ALvoid AL_APIENTRY alSourceQueueBuffers(ALuint src, ALsizei nb, const ALuint *buffers)
{
    ALCdevice *device;
    ALCcontext *context;
    ALsource *source;
    ALsizei i;
    ALbufferlistitem *BufferListStart;
    ALbufferlistitem *BufferList;
    ALbuffer *BufferFmt = NULL;

    if(nb == 0)
        return;

    context = GetContextRef();
    if(!context) return;

    device = context->Device;

    LockSourcesRead(context);
    if(!(nb >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    if((source=LookupSource(context, src)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    WriteLock(&source->queue_lock);
    if(source->SourceType == AL_STATIC)
    {
        WriteUnlock(&source->queue_lock);
        /* Can't queue on a Static Source */
        SET_ERROR_AND_GOTO(context, AL_INVALID_OPERATION, done);
    }

    /* Check for a valid Buffer, for its frequency and format */
    BufferList = ATOMIC_LOAD(&source->queue);
    while(BufferList)
    {
        if(BufferList->buffer)
        {
            BufferFmt = BufferList->buffer;
            break;
        }
        BufferList = BufferList->next;
    }

    LockBuffersRead(device);
    BufferListStart = NULL;
    BufferList = NULL;
    for(i = 0;i < nb;i++)
    {
        ALbuffer *buffer = NULL;
        if(buffers[i] && (buffer=LookupBuffer(device, buffers[i])) == NULL)
        {
            WriteUnlock(&source->queue_lock);
            SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, buffer_error);
        }

        if(!BufferListStart)
        {
            BufferListStart = malloc(sizeof(ALbufferlistitem));
            BufferList = BufferListStart;
        }
        else
        {
            BufferList->next = malloc(sizeof(ALbufferlistitem));
            BufferList = BufferList->next;
        }
        BufferList->buffer = buffer;
        BufferList->next = NULL;
        if(!buffer) continue;

        /* Hold a read lock on each buffer being queued while checking all
         * provided buffers. This is done so other threads don't see an extra
         * reference on some buffers if this operation ends up failing. */
        ReadLock(&buffer->lock);
        IncrementRef(&buffer->ref);

        if(BufferFmt == NULL)
        {
            BufferFmt = buffer;

            source->NumChannels = ChannelsFromFmt(buffer->FmtChannels);
            source->SampleSize  = BytesFromFmt(buffer->FmtType);
        }
        else if(BufferFmt->Frequency != buffer->Frequency ||
                BufferFmt->OriginalChannels != buffer->OriginalChannels ||
                BufferFmt->OriginalType != buffer->OriginalType)
        {
            WriteUnlock(&source->queue_lock);
            SET_ERROR_AND_GOTO(context, AL_INVALID_OPERATION, buffer_error);

        buffer_error:
            /* A buffer failed (invalid ID or format), so unlock and release
             * each buffer we had. */
            while(BufferListStart)
            {
                ALbufferlistitem *next = BufferListStart->next;
                if((buffer=BufferListStart->buffer) != NULL)
                {
                    DecrementRef(&buffer->ref);
                    ReadUnlock(&buffer->lock);
                }
                free(BufferListStart);
                BufferListStart = next;
            }
            UnlockBuffersRead(device);
            goto done;
        }
    }
    /* All buffers good, unlock them now. */
    BufferList = BufferListStart;
    while(BufferList != NULL)
    {
        ALbuffer *buffer = BufferList->buffer;
        if(buffer) ReadUnlock(&buffer->lock);
        BufferList = BufferList->next;
    }
    UnlockBuffersRead(device);

    /* Source is now streaming */
    source->SourceType = AL_STREAMING;

    BufferList = NULL;
    if(!ATOMIC_COMPARE_EXCHANGE_STRONG(ALbufferlistitem*, &source->queue, &BufferList, BufferListStart))
    {
        /* Queue head is not NULL, append to the end of the queue */
        while(BufferList->next != NULL)
            BufferList = BufferList->next;
        BufferList->next = BufferListStart;
    }
    /* If the current buffer was at the end (NULL), put it at the start of the newly queued
     * buffers.
     */
    BufferList = NULL;
    ATOMIC_COMPARE_EXCHANGE_STRONG(ALbufferlistitem*, &source->current_buffer, &BufferList, BufferListStart);
    WriteUnlock(&source->queue_lock);

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}

AL_API ALvoid AL_APIENTRY alSourceUnqueueBuffers(ALuint src, ALsizei nb, ALuint *buffers)
{
    ALCcontext *context;
    ALsource *source;
    ALbufferlistitem *OldHead;
    ALbufferlistitem *OldTail;
    ALbufferlistitem *Current;
    ALsizei i = 0;

    if(nb == 0)
        return;

    context = GetContextRef();
    if(!context) return;

    LockSourcesRead(context);
    if(!(nb >= 0))
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);

    if((source=LookupSource(context, src)) == NULL)
        SET_ERROR_AND_GOTO(context, AL_INVALID_NAME, done);

    WriteLock(&source->queue_lock);
    if(ATOMIC_LOAD(&source->looping) || source->SourceType != AL_STREAMING)
    {
        WriteUnlock(&source->queue_lock);
        /* Trying to unqueue buffers on a looping or non-streaming source. */
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }

    /* Find the new buffer queue head */
    OldTail = ATOMIC_LOAD(&source->queue);
    Current = ATOMIC_LOAD(&source->current_buffer);
    if(OldTail != Current)
    {
        for(i = 1;i < nb;i++)
        {
            ALbufferlistitem *next = OldTail->next;
            if(!next || next == Current) break;
            OldTail = next;
        }
    }
    if(i != nb)
    {
        WriteUnlock(&source->queue_lock);
        /* Trying to unqueue pending buffers. */
        SET_ERROR_AND_GOTO(context, AL_INVALID_VALUE, done);
    }

    /* Swap it, and cut the new head from the old. */
    OldHead = ATOMIC_EXCHANGE(ALbufferlistitem*, &source->queue, OldTail->next);
    if(OldTail->next)
    {
        ALCdevice *device = context->Device;
        uint count;

        /* Once the active mix (if any) is done, it's safe to cut the old tail
         * from the new head.
         */
        if(((count=ReadRef(&device->MixCount))&1) != 0)
        {
            while(count == ReadRef(&device->MixCount))
                althrd_yield();
        }
        OldTail->next = NULL;
    }
    WriteUnlock(&source->queue_lock);

    while(OldHead != NULL)
    {
        ALbufferlistitem *next = OldHead->next;
        ALbuffer *buffer = OldHead->buffer;

        if(!buffer)
            *(buffers++) = 0;
        else
        {
            *(buffers++) = buffer->id;
            DecrementRef(&buffer->ref);
        }

        free(OldHead);
        OldHead = next;
    }

done:
    UnlockSourcesRead(context);
    ALCcontext_DecRef(context);
}


static void InitSourceParams(ALsource *Source)
{
    ALuint i;

    RWLockInit(&Source->queue_lock);

    Source->InnerAngle = 360.0f;
    Source->OuterAngle = 360.0f;
    Source->Pitch = 1.0f;
    Source->Position[0] = 0.0f;
    Source->Position[1] = 0.0f;
    Source->Position[2] = 0.0f;
    Source->Velocity[0] = 0.0f;
    Source->Velocity[1] = 0.0f;
    Source->Velocity[2] = 0.0f;
    Source->Direction[0] = 0.0f;
    Source->Direction[1] = 0.0f;
    Source->Direction[2] = 0.0f;
    Source->Orientation[0][0] =  0.0f;
    Source->Orientation[0][1] =  0.0f;
    Source->Orientation[0][2] = -1.0f;
    Source->Orientation[1][0] =  0.0f;
    Source->Orientation[1][1] =  1.0f;
    Source->Orientation[1][2] =  0.0f;
    Source->RefDistance = 1.0f;
    Source->MaxDistance = FLT_MAX;
    Source->RollOffFactor = 1.0f;
    Source->Gain = 1.0f;
    Source->MinGain = 0.0f;
    Source->MaxGain = 1.0f;
    Source->OuterGain = 0.0f;
    Source->OuterGainHF = 1.0f;

    Source->DryGainHFAuto = AL_TRUE;
    Source->WetGainAuto = AL_TRUE;
    Source->WetGainHFAuto = AL_TRUE;
    Source->AirAbsorptionFactor = 0.0f;
    Source->RoomRolloffFactor = 0.0f;
    Source->DopplerFactor = 1.0f;
    Source->DirectChannels = AL_FALSE;

    Source->StereoPan[0] = DEG2RAD( 30.0f);
    Source->StereoPan[1] = DEG2RAD(-30.0f);

    Source->Radius = 0.0f;

    Source->DistanceModel = DefaultDistanceModel;

    Source->Direct.Gain = 1.0f;
    Source->Direct.GainHF = 1.0f;
    Source->Direct.HFReference = LOWPASSFREQREF;
    Source->Direct.GainLF = 1.0f;
    Source->Direct.LFReference = HIGHPASSFREQREF;
    for(i = 0;i < MAX_SENDS;i++)
    {
        Source->Send[i].Gain = 1.0f;
        Source->Send[i].GainHF = 1.0f;
        Source->Send[i].HFReference = LOWPASSFREQREF;
        Source->Send[i].GainLF = 1.0f;
        Source->Send[i].LFReference = HIGHPASSFREQREF;
    }

    Source->Offset = 0.0;
    Source->OffsetType = AL_NONE;
    Source->SourceType = AL_UNDETERMINED;
    Source->state = AL_INITIAL;
    Source->new_state = AL_NONE;

    ATOMIC_INIT(&Source->queue, NULL);
    ATOMIC_INIT(&Source->current_buffer, NULL);

    ATOMIC_INIT(&Source->position, 0);
    ATOMIC_INIT(&Source->position_fraction, 0);

    ATOMIC_INIT(&Source->looping, AL_FALSE);

    ATOMIC_INIT(&Source->Update, NULL);
    ATOMIC_INIT(&Source->FreeList, NULL);
}

static void DeinitSource(ALsource *source)
{
    ALbufferlistitem *BufferList;
    struct ALsourceProps *props;
    size_t count = 0;
    size_t i;

    props = ATOMIC_LOAD(&source->Update);
    if(props) al_free(props);

    props = ATOMIC_LOAD(&source->FreeList, almemory_order_relaxed);
    while(props)
    {
        struct ALsourceProps *next;
        next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        al_free(props);
        props = next;
        ++count;
    }
    /* This is excessively spammy if it traces every source destruction, so
     * just warn if it was unexpectedly large.
     */
    if(count > 3)
        WARN("Freed "SZFMT" Source property objects\n", count);

    BufferList = ATOMIC_EXCHANGE(ALbufferlistitem*, &source->queue, NULL);
    while(BufferList != NULL)
    {
        ALbufferlistitem *next = BufferList->next;
        if(BufferList->buffer != NULL)
            DecrementRef(&BufferList->buffer->ref);
        free(BufferList);
        BufferList = next;
    }

    for(i = 0;i < MAX_SENDS;++i)
    {
        if(source->Send[i].Slot)
            DecrementRef(&source->Send[i].Slot->ref);
        source->Send[i].Slot = NULL;
    }
}

static void UpdateSourceProps(ALsource *source, ALuint num_sends)
{
    struct ALsourceProps *props;
    size_t i;

    /* Get an unused property container, or allocate a new one as needed. */
    props = ATOMIC_LOAD(&source->FreeList, almemory_order_acquire);
    if(!props)
        props = al_calloc(16, sizeof(*props));
    else
    {
        struct ALsourceProps *next;
        do {
            next = ATOMIC_LOAD(&props->next, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALsourceProps*,
                &source->FreeList, &props, next, almemory_order_seq_cst,
                almemory_order_consume) == 0);
    }

    /* Copy in current property values. */
    ATOMIC_STORE(&props->Pitch, source->Pitch, almemory_order_relaxed);
    ATOMIC_STORE(&props->Gain, source->Gain, almemory_order_relaxed);
    ATOMIC_STORE(&props->OuterGain, source->OuterGain, almemory_order_relaxed);
    ATOMIC_STORE(&props->MinGain, source->MinGain, almemory_order_relaxed);
    ATOMIC_STORE(&props->MaxGain, source->MaxGain, almemory_order_relaxed);
    ATOMIC_STORE(&props->InnerAngle, source->InnerAngle, almemory_order_relaxed);
    ATOMIC_STORE(&props->OuterAngle, source->OuterAngle, almemory_order_relaxed);
    ATOMIC_STORE(&props->RefDistance, source->RefDistance, almemory_order_relaxed);
    ATOMIC_STORE(&props->MaxDistance, source->MaxDistance, almemory_order_relaxed);
    ATOMIC_STORE(&props->RollOffFactor, source->RollOffFactor, almemory_order_relaxed);
    for(i = 0;i < 3;i++)
        ATOMIC_STORE(&props->Position[i], source->Position[i], almemory_order_relaxed);
    for(i = 0;i < 3;i++)
        ATOMIC_STORE(&props->Velocity[i], source->Velocity[i], almemory_order_relaxed);
    for(i = 0;i < 3;i++)
        ATOMIC_STORE(&props->Direction[i], source->Direction[i], almemory_order_relaxed);
    for(i = 0;i < 2;i++)
    {
        size_t j;
        for(j = 0;j < 3;j++)
            ATOMIC_STORE(&props->Orientation[i][j], source->Orientation[i][j],
                         almemory_order_relaxed);
    }
    ATOMIC_STORE(&props->HeadRelative, source->HeadRelative, almemory_order_relaxed);
    ATOMIC_STORE(&props->DistanceModel, source->DistanceModel, almemory_order_relaxed);
    ATOMIC_STORE(&props->DirectChannels, source->DirectChannels, almemory_order_relaxed);

    ATOMIC_STORE(&props->DryGainHFAuto, source->DryGainHFAuto, almemory_order_relaxed);
    ATOMIC_STORE(&props->WetGainAuto, source->WetGainAuto, almemory_order_relaxed);
    ATOMIC_STORE(&props->WetGainHFAuto, source->WetGainHFAuto, almemory_order_relaxed);
    ATOMIC_STORE(&props->OuterGainHF, source->OuterGainHF, almemory_order_relaxed);

    ATOMIC_STORE(&props->AirAbsorptionFactor, source->AirAbsorptionFactor, almemory_order_relaxed);
    ATOMIC_STORE(&props->RoomRolloffFactor, source->RoomRolloffFactor, almemory_order_relaxed);
    ATOMIC_STORE(&props->DopplerFactor, source->DopplerFactor, almemory_order_relaxed);

    ATOMIC_STORE(&props->StereoPan[0], source->StereoPan[0], almemory_order_relaxed);
    ATOMIC_STORE(&props->StereoPan[1], source->StereoPan[1], almemory_order_relaxed);

    ATOMIC_STORE(&props->Radius, source->Radius, almemory_order_relaxed);

    ATOMIC_STORE(&props->Direct.Gain, source->Direct.Gain, almemory_order_relaxed);
    ATOMIC_STORE(&props->Direct.GainHF, source->Direct.GainHF, almemory_order_relaxed);
    ATOMIC_STORE(&props->Direct.HFReference, source->Direct.HFReference, almemory_order_relaxed);
    ATOMIC_STORE(&props->Direct.GainLF, source->Direct.GainLF, almemory_order_relaxed);
    ATOMIC_STORE(&props->Direct.LFReference, source->Direct.LFReference, almemory_order_relaxed);

    for(i = 0;i < num_sends;i++)
    {
        ATOMIC_STORE(&props->Send[i].Slot, source->Send[i].Slot, almemory_order_relaxed);
        ATOMIC_STORE(&props->Send[i].Gain, source->Send[i].Gain, almemory_order_relaxed);
        ATOMIC_STORE(&props->Send[i].GainHF, source->Send[i].GainHF, almemory_order_relaxed);
        ATOMIC_STORE(&props->Send[i].HFReference, source->Send[i].HFReference,
                     almemory_order_relaxed);
        ATOMIC_STORE(&props->Send[i].GainLF, source->Send[i].GainLF, almemory_order_relaxed);
        ATOMIC_STORE(&props->Send[i].LFReference, source->Send[i].LFReference,
                     almemory_order_relaxed);
    }

    /* Set the new container for updating internal parameters. */
    props = ATOMIC_EXCHANGE(struct ALsourceProps*, &source->Update, props, almemory_order_acq_rel);
    if(props)
    {
        /* If there was an unused update container, put it back in the
         * freelist.
         */
        struct ALsourceProps *first = ATOMIC_LOAD(&source->FreeList);
        do {
            ATOMIC_STORE(&props->next, first, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_WEAK(struct ALsourceProps*,
                &source->FreeList, &first, props) == 0);
    }
}

void UpdateAllSourceProps(ALCcontext *context)
{
    ALuint num_sends = context->Device->NumAuxSends;
    ALsizei pos;

    for(pos = 0;pos < context->VoiceCount;pos++)
    {
        ALvoice *voice = &context->Voices[pos];
        ALsource *source = voice->Source;
        if(source != NULL && (source->state == AL_PLAYING ||
                              source->state == AL_PAUSED))
            UpdateSourceProps(source, num_sends);
    }
}


/* SetSourceState
 *
 * Sets the source's new play state given its current state.
 */
ALvoid SetSourceState(ALsource *Source, ALCcontext *Context, ALenum state)
{
    WriteLock(&Source->queue_lock);
    if(state == AL_PLAYING)
    {
        ALCdevice *device = Context->Device;
        ALbufferlistitem *BufferList;
        ALboolean discontinuity;
        ALvoice *voice = NULL;
        ALsizei i;

        /* Check that there is a queue containing at least one valid, non zero
         * length Buffer. */
        BufferList = ATOMIC_LOAD(&Source->queue);
        while(BufferList)
        {
            ALbuffer *buffer;
            if((buffer=BufferList->buffer) != NULL && buffer->SampleLen > 0)
                break;
            BufferList = BufferList->next;
        }

        if(Source->state != AL_PAUSED)
        {
            Source->state = AL_PLAYING;
            ATOMIC_STORE(&Source->current_buffer, BufferList, almemory_order_relaxed);
            ATOMIC_STORE(&Source->position, 0, almemory_order_relaxed);
            ATOMIC_STORE(&Source->position_fraction, 0);
            discontinuity = AL_TRUE;
        }
        else
        {
            Source->state = AL_PLAYING;
            discontinuity = AL_FALSE;
        }

        // Check if an Offset has been set
        if(Source->OffsetType != AL_NONE)
        {
            ApplyOffset(Source);
            /* discontinuity = AL_TRUE;??? */
        }

        /* If there's nothing to play, or device is disconnected, go right to
         * stopped */
        if(!BufferList || !device->Connected)
            goto do_stop;

        /* Make sure this source isn't already active, while looking for an
         * unused active source slot to put it in. */
        for(i = 0;i < Context->VoiceCount;i++)
        {
            ALsource *old = Source;
            if(COMPARE_EXCHANGE(&Context->Voices[i].Source, &old, NULL))
            {
                if(voice == NULL)
                {
                    voice = &Context->Voices[i];
                    voice->Source = Source;
                }
                break;
            }
            old = NULL;
            if(voice == NULL && COMPARE_EXCHANGE(&Context->Voices[i].Source, &old, Source))
                voice = &Context->Voices[i];
        }
        if(voice == NULL)
        {
            voice = &Context->Voices[Context->VoiceCount++];
            voice->Source = Source;
        }

        if(discontinuity)
        {
            /* Clear previous samples if playback is discontinuous. */
            memset(voice->PrevSamples, 0, sizeof(voice->PrevSamples));

            /* Clear the stepping value so the mixer knows not to mix this
             * until the update gets applied.
             */
            voice->Step = 0;
        }

        voice->Moving = AL_FALSE;
        for(i = 0;i < MAX_INPUT_CHANNELS;i++)
        {
            ALsizei j;
            for(j = 0;j < HRTF_HISTORY_LENGTH;j++)
                voice->Chan[i].Direct.Hrtf.State.History[j] = 0.0f;
            for(j = 0;j < HRIR_LENGTH;j++)
            {
                voice->Chan[i].Direct.Hrtf.State.Values[j][0] = 0.0f;
                voice->Chan[i].Direct.Hrtf.State.Values[j][1] = 0.0f;
            }
        }

        UpdateSourceProps(Source, device->NumAuxSends);
    }
    else if(state == AL_PAUSED)
    {
        if(Source->state == AL_PLAYING)
            Source->state = AL_PAUSED;
    }
    else if(state == AL_STOPPED)
    {
    do_stop:
        if(Source->state != AL_INITIAL)
        {
            Source->state = AL_STOPPED;
            ATOMIC_STORE(&Source->current_buffer, NULL);
        }
        Source->OffsetType = AL_NONE;
        Source->Offset = 0.0;
    }
    else if(state == AL_INITIAL)
    {
        if(Source->state != AL_INITIAL)
        {
            Source->state = AL_INITIAL;
            ATOMIC_STORE(&Source->current_buffer, ATOMIC_LOAD(&Source->queue),
                         almemory_order_relaxed);
            ATOMIC_STORE(&Source->position, 0, almemory_order_relaxed);
            ATOMIC_STORE(&Source->position_fraction, 0);
        }
        Source->OffsetType = AL_NONE;
        Source->Offset = 0.0;
    }
    WriteUnlock(&Source->queue_lock);
}

/* GetSourceSampleOffset
 *
 * Gets the current read offset for the given Source, in 32.32 fixed-point
 * samples. The offset is relative to the start of the queue (not the start of
 * the current buffer).
 */
static ALint64 GetSourceSampleOffset(ALsource *Source, ALCdevice *device, ALuint64 *clocktime)
{
    const ALbufferlistitem *BufferList;
    const ALbufferlistitem *Current;
    ALuint64 readPos;
    ALuint refcount;

    ReadLock(&Source->queue_lock);
    if(Source->state != AL_PLAYING && Source->state != AL_PAUSED)
    {
        ReadUnlock(&Source->queue_lock);
        do {
            while(((refcount=ReadRef(&device->MixCount))&1))
                althrd_yield();
            *clocktime = GetDeviceClockTime(device);
        } while(refcount != ReadRef(&device->MixCount));
        return 0;
    }

    do {
        while(((refcount=ReadRef(&device->MixCount))&1))
            althrd_yield();
        *clocktime = GetDeviceClockTime(device);

        BufferList = ATOMIC_LOAD(&Source->queue, almemory_order_relaxed);
        Current = ATOMIC_LOAD(&Source->current_buffer, almemory_order_relaxed);

        readPos  = (ALuint64)ATOMIC_LOAD(&Source->position, almemory_order_relaxed) << 32;
        readPos |= (ALuint64)ATOMIC_LOAD(&Source->position_fraction, almemory_order_relaxed) <<
                   (32-FRACTIONBITS);
    } while(refcount != ReadRef(&device->MixCount));
    while(BufferList && BufferList != Current)
    {
        if(BufferList->buffer)
            readPos += (ALuint64)BufferList->buffer->SampleLen << 32;
        BufferList = BufferList->next;
    }

    ReadUnlock(&Source->queue_lock);
    return (ALint64)minu64(readPos, U64(0x7fffffffffffffff));
}

/* GetSourceSecOffset
 *
 * Gets the current read offset for the given Source, in seconds. The offset is
 * relative to the start of the queue (not the start of the current buffer).
 */
static ALdouble GetSourceSecOffset(ALsource *Source, ALCdevice *device, ALuint64 *clocktime)
{
    const ALbufferlistitem *BufferList;
    const ALbufferlistitem *Current;
    const ALbuffer *Buffer = NULL;
    ALuint64 readPos;
    ALuint refcount;

    ReadLock(&Source->queue_lock);
    if(Source->state != AL_PLAYING && Source->state != AL_PAUSED)
    {
        ReadUnlock(&Source->queue_lock);
        do {
            while(((refcount=ReadRef(&device->MixCount))&1))
                althrd_yield();
            *clocktime = GetDeviceClockTime(device);
        } while(refcount != ReadRef(&device->MixCount));
        return 0.0;
    }

    do {
        while(((refcount=ReadRef(&device->MixCount))&1))
            althrd_yield();
        *clocktime = GetDeviceClockTime(device);

        BufferList = ATOMIC_LOAD(&Source->queue, almemory_order_relaxed);
        Current = ATOMIC_LOAD(&Source->current_buffer, almemory_order_relaxed);

        readPos  = (ALuint64)ATOMIC_LOAD(&Source->position, almemory_order_relaxed)<<FRACTIONBITS;
        readPos |= (ALuint64)ATOMIC_LOAD(&Source->position_fraction, almemory_order_relaxed);
    } while(refcount != ReadRef(&device->MixCount));
    while(BufferList && BufferList != Current)
    {
        const ALbuffer *buffer = BufferList->buffer;
        if(buffer != NULL)
        {
            if(!Buffer) Buffer = buffer;
            readPos += (ALuint64)buffer->SampleLen << FRACTIONBITS;
        }
        BufferList = BufferList->next;
    }

    while(BufferList && !Buffer)
    {
        Buffer = BufferList->buffer;
        BufferList = BufferList->next;
    }
    assert(Buffer != NULL);

    ReadUnlock(&Source->queue_lock);
    return (ALdouble)readPos / (ALdouble)FRACTIONONE / (ALdouble)Buffer->Frequency;
}

/* GetSourceOffset
 *
 * Gets the current read offset for the given Source, in the appropriate format
 * (Bytes, Samples or Seconds). The offset is relative to the start of the
 * queue (not the start of the current buffer).
 */
static ALdouble GetSourceOffset(ALsource *Source, ALenum name, ALCdevice *device)
{
    const ALbufferlistitem *BufferList;
    const ALbufferlistitem *Current;
    const ALbuffer *Buffer = NULL;
    ALboolean readFin = AL_FALSE;
    ALuint readPos, readPosFrac;
    ALuint totalBufferLen;
    ALdouble offset = 0.0;
    ALboolean looping;
    ALuint refcount;

    ReadLock(&Source->queue_lock);
    if(Source->state != AL_PLAYING && Source->state != AL_PAUSED)
    {
        ReadUnlock(&Source->queue_lock);
        return 0.0;
    }

    totalBufferLen = 0;
    do {
        while(((refcount=ReadRef(&device->MixCount))&1))
            althrd_yield();
        BufferList = ATOMIC_LOAD(&Source->queue, almemory_order_relaxed);
        Current = ATOMIC_LOAD(&Source->current_buffer, almemory_order_relaxed);

        readPos = ATOMIC_LOAD(&Source->position, almemory_order_relaxed);
        readPosFrac = ATOMIC_LOAD(&Source->position_fraction, almemory_order_relaxed);

        looping = ATOMIC_LOAD(&Source->looping, almemory_order_relaxed);
    } while(refcount != ReadRef(&device->MixCount));

    while(BufferList != NULL)
    {
        const ALbuffer *buffer;
        readFin = readFin || (BufferList == Current);
        if((buffer=BufferList->buffer) != NULL)
        {
            if(!Buffer) Buffer = buffer;
            totalBufferLen += buffer->SampleLen;
            if(!readFin) readPos += buffer->SampleLen;
        }
        BufferList = BufferList->next;
    }
    assert(Buffer != NULL);

    if(looping)
        readPos %= totalBufferLen;
    else
    {
        /* Wrap back to 0 */
        if(readPos >= totalBufferLen)
            readPos = readPosFrac = 0;
    }

    switch(name)
    {
        case AL_SEC_OFFSET:
            offset = (readPos + (ALdouble)readPosFrac/FRACTIONONE)/Buffer->Frequency;
            break;

        case AL_SAMPLE_OFFSET:
            offset = readPos + (ALdouble)readPosFrac/FRACTIONONE;
            break;

        case AL_BYTE_OFFSET:
            if(Buffer->OriginalType == UserFmtIMA4)
            {
                ALsizei align = (Buffer->OriginalAlign-1)/2 + 4;
                ALuint BlockSize = align * ChannelsFromFmt(Buffer->FmtChannels);
                ALuint FrameBlockSize = Buffer->OriginalAlign;

                /* Round down to nearest ADPCM block */
                offset = (ALdouble)(readPos / FrameBlockSize * BlockSize);
            }
            else if(Buffer->OriginalType == UserFmtMSADPCM)
            {
                ALsizei align = (Buffer->OriginalAlign-2)/2 + 7;
                ALuint BlockSize = align * ChannelsFromFmt(Buffer->FmtChannels);
                ALuint FrameBlockSize = Buffer->OriginalAlign;

                /* Round down to nearest ADPCM block */
                offset = (ALdouble)(readPos / FrameBlockSize * BlockSize);
            }
            else
            {
                ALuint FrameSize = FrameSizeFromUserFmt(Buffer->OriginalChannels, Buffer->OriginalType);
                offset = (ALdouble)(readPos * FrameSize);
            }
            break;
    }

    ReadUnlock(&Source->queue_lock);
    return offset;
}


/* ApplyOffset
 *
 * Apply the stored playback offset to the Source. This function will update
 * the number of buffers "played" given the stored offset.
 */
ALboolean ApplyOffset(ALsource *Source)
{
    ALbufferlistitem *BufferList;
    const ALbuffer *Buffer;
    ALuint bufferLen, totalBufferLen;
    ALuint offset=0, frac=0;

    /* Get sample frame offset */
    if(!GetSampleOffset(Source, &offset, &frac))
        return AL_FALSE;

    totalBufferLen = 0;
    BufferList = ATOMIC_LOAD(&Source->queue);
    while(BufferList && totalBufferLen <= offset)
    {
        Buffer = BufferList->buffer;
        bufferLen = Buffer ? Buffer->SampleLen : 0;

        if(bufferLen > offset-totalBufferLen)
        {
            /* Offset is in this buffer */
            ATOMIC_STORE(&Source->current_buffer, BufferList, almemory_order_relaxed);

            ATOMIC_STORE(&Source->position, offset - totalBufferLen, almemory_order_relaxed);
            ATOMIC_STORE(&Source->position_fraction, frac);
            return AL_TRUE;
        }

        totalBufferLen += bufferLen;

        BufferList = BufferList->next;
    }

    /* Offset is out of range of the queue */
    return AL_FALSE;
}


/* GetSampleOffset
 *
 * Retrieves the sample offset into the Source's queue (from the Sample, Byte
 * or Second offset supplied by the application). This takes into account the
 * fact that the buffer format may have been modifed since.
 */
static ALboolean GetSampleOffset(ALsource *Source, ALuint *offset, ALuint *frac)
{
    const ALbuffer *Buffer = NULL;
    const ALbufferlistitem *BufferList;
    ALdouble dbloff, dblfrac;

    /* Find the first valid Buffer in the Queue */
    BufferList = ATOMIC_LOAD(&Source->queue);
    while(BufferList)
    {
        if(BufferList->buffer)
        {
            Buffer = BufferList->buffer;
            break;
        }
        BufferList = BufferList->next;
    }
    if(!Buffer)
    {
        Source->OffsetType = AL_NONE;
        Source->Offset = 0.0;
        return AL_FALSE;
    }

    switch(Source->OffsetType)
    {
    case AL_BYTE_OFFSET:
        /* Determine the ByteOffset (and ensure it is block aligned) */
        *offset = (ALuint)Source->Offset;
        if(Buffer->OriginalType == UserFmtIMA4)
        {
            ALsizei align = (Buffer->OriginalAlign-1)/2 + 4;
            *offset /= align * ChannelsFromUserFmt(Buffer->OriginalChannels);
            *offset *= Buffer->OriginalAlign;
        }
        else if(Buffer->OriginalType == UserFmtMSADPCM)
        {
            ALsizei align = (Buffer->OriginalAlign-2)/2 + 7;
            *offset /= align * ChannelsFromUserFmt(Buffer->OriginalChannels);
            *offset *= Buffer->OriginalAlign;
        }
        else
            *offset /= FrameSizeFromUserFmt(Buffer->OriginalChannels, Buffer->OriginalType);
        *frac = 0;
        break;

    case AL_SAMPLE_OFFSET:
        dblfrac = modf(Source->Offset, &dbloff);
        *offset = (ALuint)mind(dbloff, UINT_MAX);
        *frac = (ALuint)mind(dblfrac*FRACTIONONE, FRACTIONONE-1.0);
        break;

    case AL_SEC_OFFSET:
        dblfrac = modf(Source->Offset*Buffer->Frequency, &dbloff);
        *offset = (ALuint)mind(dbloff, UINT_MAX);
        *frac = (ALuint)mind(dblfrac*FRACTIONONE, FRACTIONONE-1.0);
        break;
    }
    Source->OffsetType = AL_NONE;
    Source->Offset = 0.0;

    return AL_TRUE;
}


/* ReleaseALSources
 *
 * Destroys all sources in the source map.
 */
ALvoid ReleaseALSources(ALCcontext *Context)
{
    ALsizei pos;
    for(pos = 0;pos < Context->SourceMap.size;pos++)
    {
        ALsource *temp = Context->SourceMap.values[pos];
        Context->SourceMap.values[pos] = NULL;

        DeinitSource(temp);

        FreeThunkEntry(temp->id);
        memset(temp, 0, sizeof(*temp));
        al_free(temp);
    }
}
