#ifndef _AL_SOURCE_H_
#define _AL_SOURCE_H_

#include "bool.h"
#include "alMain.h"
#include "alu.h"
#include "hrtf.h"
#include "atomic.h"

#define MAX_SENDS      16
#define DEFAULT_SENDS  2

#ifdef __cplusplus
extern "C" {
#endif

struct ALbuffer;
struct ALsource;


typedef struct ALbufferlistitem {
    ATOMIC(struct ALbufferlistitem*) next;
    ALsizei max_samples;
    ALsizei num_buffers;
    struct ALbuffer *buffers[];
} ALbufferlistitem;


typedef struct ALsource {
    /** Source properties. */
    ALfloat   Pitch;
    ALfloat   Gain;
    ALfloat   OuterGain;
    ALfloat   MinGain;
    ALfloat   MaxGain;
    ALfloat   InnerAngle;
    ALfloat   OuterAngle;
    ALfloat   RefDistance;
    ALfloat   MaxDistance;
    ALfloat   RolloffFactor;
    ALfloat   Position[3];
    ALfloat   Velocity[3];
    ALfloat   Direction[3];
    ALfloat   Orientation[2][3];
    ALboolean HeadRelative;
    ALboolean Looping;
    enum DistanceModel DistanceModel;
    enum Resampler Resampler;
    ALboolean DirectChannels;
    enum SpatializeMode Spatialize;

    ALboolean DryGainHFAuto;
    ALboolean WetGainAuto;
    ALboolean WetGainHFAuto;
    ALfloat   OuterGainHF;

    ALfloat AirAbsorptionFactor;
    ALfloat RoomRolloffFactor;
    ALfloat DopplerFactor;

    /* NOTE: Stereo pan angles are specified in radians, counter-clockwise
     * rather than clockwise.
     */
    ALfloat StereoPan[2];

    ALfloat Radius;

    /** Direct filter and auxiliary send info. */
    struct {
        ALfloat Gain;
        ALfloat GainHF;
        ALfloat HFReference;
        ALfloat GainLF;
        ALfloat LFReference;
    } Direct;
    struct {
        struct ALeffectslot *Slot;
        ALfloat Gain;
        ALfloat GainHF;
        ALfloat HFReference;
        ALfloat GainLF;
        ALfloat LFReference;
    } *Send;

    /**
     * Last user-specified offset, and the offset type (bytes, samples, or
     * seconds).
     */
    ALdouble Offset;
    ALenum   OffsetType;

    /** Source type (static, streaming, or undetermined) */
    ALint SourceType;

    /** Source state (initial, playing, paused, or stopped) */
    ALenum state;

    /** Source Buffer Queue head. */
    ALbufferlistitem *queue;

    ATOMIC_FLAG PropsClean;

    /* Index into the context's Voices array. Lazily updated, only checked and
     * reset when looking up the voice.
     */
    ALint VoiceIdx;

    /** Self ID */
    ALuint id;
} ALsource;

void UpdateAllSourceProps(ALCcontext *context);

ALvoid ReleaseALSources(ALCcontext *Context);

#ifdef __cplusplus
}
#endif

#endif
