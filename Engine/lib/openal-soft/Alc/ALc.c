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

#include "version.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <signal.h>

#include "alMain.h"
#include "alSource.h"
#include "alListener.h"
#include "alSource.h"
#include "alBuffer.h"
#include "alFilter.h"
#include "alEffect.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "mastering.h"
#include "bformatdec.h"
#include "alu.h"
#include "alconfig.h"
#include "ringbuffer.h"

#include "fpu_modes.h"
#include "cpu_caps.h"
#include "compat.h"
#include "threads.h"
#include "alstring.h"
#include "almalloc.h"

#include "backends/base.h"


/************************************************
 * Backends
 ************************************************/
struct BackendInfo {
    const char *name;
    ALCbackendFactory* (*getFactory)(void);
};

static struct BackendInfo BackendList[] = {
#ifdef HAVE_JACK
    { "jack", ALCjackBackendFactory_getFactory },
#endif
#ifdef HAVE_PULSEAUDIO
    { "pulse", ALCpulseBackendFactory_getFactory },
#endif
#ifdef HAVE_ALSA
    { "alsa", ALCalsaBackendFactory_getFactory },
#endif
#ifdef HAVE_COREAUDIO
    { "core", ALCcoreAudioBackendFactory_getFactory },
#endif
#ifdef HAVE_OSS
    { "oss", ALCossBackendFactory_getFactory },
#endif
#ifdef HAVE_SOLARIS
    { "solaris", ALCsolarisBackendFactory_getFactory },
#endif
#ifdef HAVE_SNDIO
    { "sndio", ALCsndioBackendFactory_getFactory },
#endif
#ifdef HAVE_QSA
    { "qsa", ALCqsaBackendFactory_getFactory },
#endif
#ifdef HAVE_WASAPI
    { "wasapi", ALCwasapiBackendFactory_getFactory },
#endif
#ifdef HAVE_DSOUND
    { "dsound", ALCdsoundBackendFactory_getFactory },
#endif
#ifdef HAVE_WINMM
    { "winmm", ALCwinmmBackendFactory_getFactory },
#endif
#ifdef HAVE_PORTAUDIO
    { "port", ALCportBackendFactory_getFactory },
#endif
#ifdef HAVE_OPENSL
    { "opensl", ALCopenslBackendFactory_getFactory },
#endif
#ifdef HAVE_SDL2
    { "sdl2", ALCsdl2BackendFactory_getFactory },
#endif

    { "null", ALCnullBackendFactory_getFactory },
#ifdef HAVE_WAVE
    { "wave", ALCwaveBackendFactory_getFactory },
#endif
};
static ALsizei BackendListSize = COUNTOF(BackendList);
#undef EmptyFuncs

static struct BackendInfo PlaybackBackend;
static struct BackendInfo CaptureBackend;


/************************************************
 * Functions, enums, and errors
 ************************************************/
#define DECL(x) { #x, (ALCvoid*)(x) }
static const struct {
    const ALCchar *funcName;
    ALCvoid *address;
} alcFunctions[] = {
    DECL(alcCreateContext),
    DECL(alcMakeContextCurrent),
    DECL(alcProcessContext),
    DECL(alcSuspendContext),
    DECL(alcDestroyContext),
    DECL(alcGetCurrentContext),
    DECL(alcGetContextsDevice),
    DECL(alcOpenDevice),
    DECL(alcCloseDevice),
    DECL(alcGetError),
    DECL(alcIsExtensionPresent),
    DECL(alcGetProcAddress),
    DECL(alcGetEnumValue),
    DECL(alcGetString),
    DECL(alcGetIntegerv),
    DECL(alcCaptureOpenDevice),
    DECL(alcCaptureCloseDevice),
    DECL(alcCaptureStart),
    DECL(alcCaptureStop),
    DECL(alcCaptureSamples),

    DECL(alcSetThreadContext),
    DECL(alcGetThreadContext),

    DECL(alcLoopbackOpenDeviceSOFT),
    DECL(alcIsRenderFormatSupportedSOFT),
    DECL(alcRenderSamplesSOFT),

    DECL(alcDevicePauseSOFT),
    DECL(alcDeviceResumeSOFT),

    DECL(alcGetStringiSOFT),
    DECL(alcResetDeviceSOFT),

    DECL(alcGetInteger64vSOFT),

    DECL(alEnable),
    DECL(alDisable),
    DECL(alIsEnabled),
    DECL(alGetString),
    DECL(alGetBooleanv),
    DECL(alGetIntegerv),
    DECL(alGetFloatv),
    DECL(alGetDoublev),
    DECL(alGetBoolean),
    DECL(alGetInteger),
    DECL(alGetFloat),
    DECL(alGetDouble),
    DECL(alGetError),
    DECL(alIsExtensionPresent),
    DECL(alGetProcAddress),
    DECL(alGetEnumValue),
    DECL(alListenerf),
    DECL(alListener3f),
    DECL(alListenerfv),
    DECL(alListeneri),
    DECL(alListener3i),
    DECL(alListeneriv),
    DECL(alGetListenerf),
    DECL(alGetListener3f),
    DECL(alGetListenerfv),
    DECL(alGetListeneri),
    DECL(alGetListener3i),
    DECL(alGetListeneriv),
    DECL(alGenSources),
    DECL(alDeleteSources),
    DECL(alIsSource),
    DECL(alSourcef),
    DECL(alSource3f),
    DECL(alSourcefv),
    DECL(alSourcei),
    DECL(alSource3i),
    DECL(alSourceiv),
    DECL(alGetSourcef),
    DECL(alGetSource3f),
    DECL(alGetSourcefv),
    DECL(alGetSourcei),
    DECL(alGetSource3i),
    DECL(alGetSourceiv),
    DECL(alSourcePlayv),
    DECL(alSourceStopv),
    DECL(alSourceRewindv),
    DECL(alSourcePausev),
    DECL(alSourcePlay),
    DECL(alSourceStop),
    DECL(alSourceRewind),
    DECL(alSourcePause),
    DECL(alSourceQueueBuffers),
    DECL(alSourceUnqueueBuffers),
    DECL(alGenBuffers),
    DECL(alDeleteBuffers),
    DECL(alIsBuffer),
    DECL(alBufferData),
    DECL(alBufferf),
    DECL(alBuffer3f),
    DECL(alBufferfv),
    DECL(alBufferi),
    DECL(alBuffer3i),
    DECL(alBufferiv),
    DECL(alGetBufferf),
    DECL(alGetBuffer3f),
    DECL(alGetBufferfv),
    DECL(alGetBufferi),
    DECL(alGetBuffer3i),
    DECL(alGetBufferiv),
    DECL(alDopplerFactor),
    DECL(alDopplerVelocity),
    DECL(alSpeedOfSound),
    DECL(alDistanceModel),

    DECL(alGenFilters),
    DECL(alDeleteFilters),
    DECL(alIsFilter),
    DECL(alFilteri),
    DECL(alFilteriv),
    DECL(alFilterf),
    DECL(alFilterfv),
    DECL(alGetFilteri),
    DECL(alGetFilteriv),
    DECL(alGetFilterf),
    DECL(alGetFilterfv),
    DECL(alGenEffects),
    DECL(alDeleteEffects),
    DECL(alIsEffect),
    DECL(alEffecti),
    DECL(alEffectiv),
    DECL(alEffectf),
    DECL(alEffectfv),
    DECL(alGetEffecti),
    DECL(alGetEffectiv),
    DECL(alGetEffectf),
    DECL(alGetEffectfv),
    DECL(alGenAuxiliaryEffectSlots),
    DECL(alDeleteAuxiliaryEffectSlots),
    DECL(alIsAuxiliaryEffectSlot),
    DECL(alAuxiliaryEffectSloti),
    DECL(alAuxiliaryEffectSlotiv),
    DECL(alAuxiliaryEffectSlotf),
    DECL(alAuxiliaryEffectSlotfv),
    DECL(alGetAuxiliaryEffectSloti),
    DECL(alGetAuxiliaryEffectSlotiv),
    DECL(alGetAuxiliaryEffectSlotf),
    DECL(alGetAuxiliaryEffectSlotfv),

    DECL(alDeferUpdatesSOFT),
    DECL(alProcessUpdatesSOFT),

    DECL(alSourcedSOFT),
    DECL(alSource3dSOFT),
    DECL(alSourcedvSOFT),
    DECL(alGetSourcedSOFT),
    DECL(alGetSource3dSOFT),
    DECL(alGetSourcedvSOFT),
    DECL(alSourcei64SOFT),
    DECL(alSource3i64SOFT),
    DECL(alSourcei64vSOFT),
    DECL(alGetSourcei64SOFT),
    DECL(alGetSource3i64SOFT),
    DECL(alGetSourcei64vSOFT),

    DECL(alGetStringiSOFT),

    DECL(alBufferStorageSOFT),
    DECL(alMapBufferSOFT),
    DECL(alUnmapBufferSOFT),
    DECL(alFlushMappedBufferSOFT),

    DECL(alEventControlSOFT),
    DECL(alEventCallbackSOFT),
    DECL(alGetPointerSOFT),
    DECL(alGetPointervSOFT),
};
#undef DECL

#define DECL(x) { #x, (x) }
static const struct {
    const ALCchar *enumName;
    ALCenum value;
} alcEnumerations[] = {
    DECL(ALC_INVALID),
    DECL(ALC_FALSE),
    DECL(ALC_TRUE),

    DECL(ALC_MAJOR_VERSION),
    DECL(ALC_MINOR_VERSION),
    DECL(ALC_ATTRIBUTES_SIZE),
    DECL(ALC_ALL_ATTRIBUTES),
    DECL(ALC_DEFAULT_DEVICE_SPECIFIER),
    DECL(ALC_DEVICE_SPECIFIER),
    DECL(ALC_ALL_DEVICES_SPECIFIER),
    DECL(ALC_DEFAULT_ALL_DEVICES_SPECIFIER),
    DECL(ALC_EXTENSIONS),
    DECL(ALC_FREQUENCY),
    DECL(ALC_REFRESH),
    DECL(ALC_SYNC),
    DECL(ALC_MONO_SOURCES),
    DECL(ALC_STEREO_SOURCES),
    DECL(ALC_CAPTURE_DEVICE_SPECIFIER),
    DECL(ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER),
    DECL(ALC_CAPTURE_SAMPLES),
    DECL(ALC_CONNECTED),

    DECL(ALC_EFX_MAJOR_VERSION),
    DECL(ALC_EFX_MINOR_VERSION),
    DECL(ALC_MAX_AUXILIARY_SENDS),

    DECL(ALC_FORMAT_CHANNELS_SOFT),
    DECL(ALC_FORMAT_TYPE_SOFT),

    DECL(ALC_MONO_SOFT),
    DECL(ALC_STEREO_SOFT),
    DECL(ALC_QUAD_SOFT),
    DECL(ALC_5POINT1_SOFT),
    DECL(ALC_6POINT1_SOFT),
    DECL(ALC_7POINT1_SOFT),
    DECL(ALC_BFORMAT3D_SOFT),

    DECL(ALC_BYTE_SOFT),
    DECL(ALC_UNSIGNED_BYTE_SOFT),
    DECL(ALC_SHORT_SOFT),
    DECL(ALC_UNSIGNED_SHORT_SOFT),
    DECL(ALC_INT_SOFT),
    DECL(ALC_UNSIGNED_INT_SOFT),
    DECL(ALC_FLOAT_SOFT),

    DECL(ALC_HRTF_SOFT),
    DECL(ALC_DONT_CARE_SOFT),
    DECL(ALC_HRTF_STATUS_SOFT),
    DECL(ALC_HRTF_DISABLED_SOFT),
    DECL(ALC_HRTF_ENABLED_SOFT),
    DECL(ALC_HRTF_DENIED_SOFT),
    DECL(ALC_HRTF_REQUIRED_SOFT),
    DECL(ALC_HRTF_HEADPHONES_DETECTED_SOFT),
    DECL(ALC_HRTF_UNSUPPORTED_FORMAT_SOFT),
    DECL(ALC_NUM_HRTF_SPECIFIERS_SOFT),
    DECL(ALC_HRTF_SPECIFIER_SOFT),
    DECL(ALC_HRTF_ID_SOFT),

    DECL(ALC_AMBISONIC_LAYOUT_SOFT),
    DECL(ALC_AMBISONIC_SCALING_SOFT),
    DECL(ALC_AMBISONIC_ORDER_SOFT),
    DECL(ALC_ACN_SOFT),
    DECL(ALC_FUMA_SOFT),
    DECL(ALC_N3D_SOFT),
    DECL(ALC_SN3D_SOFT),

    DECL(ALC_OUTPUT_LIMITER_SOFT),

    DECL(ALC_NO_ERROR),
    DECL(ALC_INVALID_DEVICE),
    DECL(ALC_INVALID_CONTEXT),
    DECL(ALC_INVALID_ENUM),
    DECL(ALC_INVALID_VALUE),
    DECL(ALC_OUT_OF_MEMORY),


    DECL(AL_INVALID),
    DECL(AL_NONE),
    DECL(AL_FALSE),
    DECL(AL_TRUE),

    DECL(AL_SOURCE_RELATIVE),
    DECL(AL_CONE_INNER_ANGLE),
    DECL(AL_CONE_OUTER_ANGLE),
    DECL(AL_PITCH),
    DECL(AL_POSITION),
    DECL(AL_DIRECTION),
    DECL(AL_VELOCITY),
    DECL(AL_LOOPING),
    DECL(AL_BUFFER),
    DECL(AL_GAIN),
    DECL(AL_MIN_GAIN),
    DECL(AL_MAX_GAIN),
    DECL(AL_ORIENTATION),
    DECL(AL_REFERENCE_DISTANCE),
    DECL(AL_ROLLOFF_FACTOR),
    DECL(AL_CONE_OUTER_GAIN),
    DECL(AL_MAX_DISTANCE),
    DECL(AL_SEC_OFFSET),
    DECL(AL_SAMPLE_OFFSET),
    DECL(AL_BYTE_OFFSET),
    DECL(AL_SOURCE_TYPE),
    DECL(AL_STATIC),
    DECL(AL_STREAMING),
    DECL(AL_UNDETERMINED),
    DECL(AL_METERS_PER_UNIT),
    DECL(AL_LOOP_POINTS_SOFT),
    DECL(AL_DIRECT_CHANNELS_SOFT),

    DECL(AL_DIRECT_FILTER),
    DECL(AL_AUXILIARY_SEND_FILTER),
    DECL(AL_AIR_ABSORPTION_FACTOR),
    DECL(AL_ROOM_ROLLOFF_FACTOR),
    DECL(AL_CONE_OUTER_GAINHF),
    DECL(AL_DIRECT_FILTER_GAINHF_AUTO),
    DECL(AL_AUXILIARY_SEND_FILTER_GAIN_AUTO),
    DECL(AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO),

    DECL(AL_SOURCE_STATE),
    DECL(AL_INITIAL),
    DECL(AL_PLAYING),
    DECL(AL_PAUSED),
    DECL(AL_STOPPED),

    DECL(AL_BUFFERS_QUEUED),
    DECL(AL_BUFFERS_PROCESSED),

    DECL(AL_FORMAT_MONO8),
    DECL(AL_FORMAT_MONO16),
    DECL(AL_FORMAT_MONO_FLOAT32),
    DECL(AL_FORMAT_MONO_DOUBLE_EXT),
    DECL(AL_FORMAT_STEREO8),
    DECL(AL_FORMAT_STEREO16),
    DECL(AL_FORMAT_STEREO_FLOAT32),
    DECL(AL_FORMAT_STEREO_DOUBLE_EXT),
    DECL(AL_FORMAT_MONO_IMA4),
    DECL(AL_FORMAT_STEREO_IMA4),
    DECL(AL_FORMAT_MONO_MSADPCM_SOFT),
    DECL(AL_FORMAT_STEREO_MSADPCM_SOFT),
    DECL(AL_FORMAT_QUAD8_LOKI),
    DECL(AL_FORMAT_QUAD16_LOKI),
    DECL(AL_FORMAT_QUAD8),
    DECL(AL_FORMAT_QUAD16),
    DECL(AL_FORMAT_QUAD32),
    DECL(AL_FORMAT_51CHN8),
    DECL(AL_FORMAT_51CHN16),
    DECL(AL_FORMAT_51CHN32),
    DECL(AL_FORMAT_61CHN8),
    DECL(AL_FORMAT_61CHN16),
    DECL(AL_FORMAT_61CHN32),
    DECL(AL_FORMAT_71CHN8),
    DECL(AL_FORMAT_71CHN16),
    DECL(AL_FORMAT_71CHN32),
    DECL(AL_FORMAT_REAR8),
    DECL(AL_FORMAT_REAR16),
    DECL(AL_FORMAT_REAR32),
    DECL(AL_FORMAT_MONO_MULAW),
    DECL(AL_FORMAT_MONO_MULAW_EXT),
    DECL(AL_FORMAT_STEREO_MULAW),
    DECL(AL_FORMAT_STEREO_MULAW_EXT),
    DECL(AL_FORMAT_QUAD_MULAW),
    DECL(AL_FORMAT_51CHN_MULAW),
    DECL(AL_FORMAT_61CHN_MULAW),
    DECL(AL_FORMAT_71CHN_MULAW),
    DECL(AL_FORMAT_REAR_MULAW),
    DECL(AL_FORMAT_MONO_ALAW_EXT),
    DECL(AL_FORMAT_STEREO_ALAW_EXT),

    DECL(AL_FORMAT_BFORMAT2D_8),
    DECL(AL_FORMAT_BFORMAT2D_16),
    DECL(AL_FORMAT_BFORMAT2D_FLOAT32),
    DECL(AL_FORMAT_BFORMAT2D_MULAW),
    DECL(AL_FORMAT_BFORMAT3D_8),
    DECL(AL_FORMAT_BFORMAT3D_16),
    DECL(AL_FORMAT_BFORMAT3D_FLOAT32),
    DECL(AL_FORMAT_BFORMAT3D_MULAW),

    DECL(AL_FREQUENCY),
    DECL(AL_BITS),
    DECL(AL_CHANNELS),
    DECL(AL_SIZE),
    DECL(AL_UNPACK_BLOCK_ALIGNMENT_SOFT),
    DECL(AL_PACK_BLOCK_ALIGNMENT_SOFT),

    DECL(AL_SOURCE_RADIUS),

    DECL(AL_STEREO_ANGLES),

    DECL(AL_UNUSED),
    DECL(AL_PENDING),
    DECL(AL_PROCESSED),

    DECL(AL_NO_ERROR),
    DECL(AL_INVALID_NAME),
    DECL(AL_INVALID_ENUM),
    DECL(AL_INVALID_VALUE),
    DECL(AL_INVALID_OPERATION),
    DECL(AL_OUT_OF_MEMORY),

    DECL(AL_VENDOR),
    DECL(AL_VERSION),
    DECL(AL_RENDERER),
    DECL(AL_EXTENSIONS),

    DECL(AL_DOPPLER_FACTOR),
    DECL(AL_DOPPLER_VELOCITY),
    DECL(AL_DISTANCE_MODEL),
    DECL(AL_SPEED_OF_SOUND),
    DECL(AL_SOURCE_DISTANCE_MODEL),
    DECL(AL_DEFERRED_UPDATES_SOFT),
    DECL(AL_GAIN_LIMIT_SOFT),

    DECL(AL_INVERSE_DISTANCE),
    DECL(AL_INVERSE_DISTANCE_CLAMPED),
    DECL(AL_LINEAR_DISTANCE),
    DECL(AL_LINEAR_DISTANCE_CLAMPED),
    DECL(AL_EXPONENT_DISTANCE),
    DECL(AL_EXPONENT_DISTANCE_CLAMPED),

    DECL(AL_FILTER_TYPE),
    DECL(AL_FILTER_NULL),
    DECL(AL_FILTER_LOWPASS),
    DECL(AL_FILTER_HIGHPASS),
    DECL(AL_FILTER_BANDPASS),

    DECL(AL_LOWPASS_GAIN),
    DECL(AL_LOWPASS_GAINHF),

    DECL(AL_HIGHPASS_GAIN),
    DECL(AL_HIGHPASS_GAINLF),

    DECL(AL_BANDPASS_GAIN),
    DECL(AL_BANDPASS_GAINHF),
    DECL(AL_BANDPASS_GAINLF),

    DECL(AL_EFFECT_TYPE),
    DECL(AL_EFFECT_NULL),
    DECL(AL_EFFECT_REVERB),
    DECL(AL_EFFECT_EAXREVERB),
    DECL(AL_EFFECT_CHORUS),
    DECL(AL_EFFECT_DISTORTION),
    DECL(AL_EFFECT_ECHO),
    DECL(AL_EFFECT_FLANGER),
    DECL(AL_EFFECT_PITCH_SHIFTER),
#if 0
    DECL(AL_EFFECT_FREQUENCY_SHIFTER),
    DECL(AL_EFFECT_VOCAL_MORPHER),
#endif
    DECL(AL_EFFECT_RING_MODULATOR),
#if 0
    DECL(AL_EFFECT_AUTOWAH),
#endif
    DECL(AL_EFFECT_COMPRESSOR),
    DECL(AL_EFFECT_EQUALIZER),
    DECL(AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT),
    DECL(AL_EFFECT_DEDICATED_DIALOGUE),

    DECL(AL_EFFECTSLOT_EFFECT),
    DECL(AL_EFFECTSLOT_GAIN),
    DECL(AL_EFFECTSLOT_AUXILIARY_SEND_AUTO),
    DECL(AL_EFFECTSLOT_NULL),

    DECL(AL_EAXREVERB_DENSITY),
    DECL(AL_EAXREVERB_DIFFUSION),
    DECL(AL_EAXREVERB_GAIN),
    DECL(AL_EAXREVERB_GAINHF),
    DECL(AL_EAXREVERB_GAINLF),
    DECL(AL_EAXREVERB_DECAY_TIME),
    DECL(AL_EAXREVERB_DECAY_HFRATIO),
    DECL(AL_EAXREVERB_DECAY_LFRATIO),
    DECL(AL_EAXREVERB_REFLECTIONS_GAIN),
    DECL(AL_EAXREVERB_REFLECTIONS_DELAY),
    DECL(AL_EAXREVERB_REFLECTIONS_PAN),
    DECL(AL_EAXREVERB_LATE_REVERB_GAIN),
    DECL(AL_EAXREVERB_LATE_REVERB_DELAY),
    DECL(AL_EAXREVERB_LATE_REVERB_PAN),
    DECL(AL_EAXREVERB_ECHO_TIME),
    DECL(AL_EAXREVERB_ECHO_DEPTH),
    DECL(AL_EAXREVERB_MODULATION_TIME),
    DECL(AL_EAXREVERB_MODULATION_DEPTH),
    DECL(AL_EAXREVERB_AIR_ABSORPTION_GAINHF),
    DECL(AL_EAXREVERB_HFREFERENCE),
    DECL(AL_EAXREVERB_LFREFERENCE),
    DECL(AL_EAXREVERB_ROOM_ROLLOFF_FACTOR),
    DECL(AL_EAXREVERB_DECAY_HFLIMIT),

    DECL(AL_REVERB_DENSITY),
    DECL(AL_REVERB_DIFFUSION),
    DECL(AL_REVERB_GAIN),
    DECL(AL_REVERB_GAINHF),
    DECL(AL_REVERB_DECAY_TIME),
    DECL(AL_REVERB_DECAY_HFRATIO),
    DECL(AL_REVERB_REFLECTIONS_GAIN),
    DECL(AL_REVERB_REFLECTIONS_DELAY),
    DECL(AL_REVERB_LATE_REVERB_GAIN),
    DECL(AL_REVERB_LATE_REVERB_DELAY),
    DECL(AL_REVERB_AIR_ABSORPTION_GAINHF),
    DECL(AL_REVERB_ROOM_ROLLOFF_FACTOR),
    DECL(AL_REVERB_DECAY_HFLIMIT),

    DECL(AL_CHORUS_WAVEFORM),
    DECL(AL_CHORUS_PHASE),
    DECL(AL_CHORUS_RATE),
    DECL(AL_CHORUS_DEPTH),
    DECL(AL_CHORUS_FEEDBACK),
    DECL(AL_CHORUS_DELAY),

    DECL(AL_DISTORTION_EDGE),
    DECL(AL_DISTORTION_GAIN),
    DECL(AL_DISTORTION_LOWPASS_CUTOFF),
    DECL(AL_DISTORTION_EQCENTER),
    DECL(AL_DISTORTION_EQBANDWIDTH),

    DECL(AL_ECHO_DELAY),
    DECL(AL_ECHO_LRDELAY),
    DECL(AL_ECHO_DAMPING),
    DECL(AL_ECHO_FEEDBACK),
    DECL(AL_ECHO_SPREAD),

    DECL(AL_FLANGER_WAVEFORM),
    DECL(AL_FLANGER_PHASE),
    DECL(AL_FLANGER_RATE),
    DECL(AL_FLANGER_DEPTH),
    DECL(AL_FLANGER_FEEDBACK),
    DECL(AL_FLANGER_DELAY),

    DECL(AL_RING_MODULATOR_FREQUENCY),
    DECL(AL_RING_MODULATOR_HIGHPASS_CUTOFF),
    DECL(AL_RING_MODULATOR_WAVEFORM),

    DECL(AL_PITCH_SHIFTER_COARSE_TUNE),
    DECL(AL_PITCH_SHIFTER_FINE_TUNE),

    DECL(AL_COMPRESSOR_ONOFF),

    DECL(AL_EQUALIZER_LOW_GAIN),
    DECL(AL_EQUALIZER_LOW_CUTOFF),
    DECL(AL_EQUALIZER_MID1_GAIN),
    DECL(AL_EQUALIZER_MID1_CENTER),
    DECL(AL_EQUALIZER_MID1_WIDTH),
    DECL(AL_EQUALIZER_MID2_GAIN),
    DECL(AL_EQUALIZER_MID2_CENTER),
    DECL(AL_EQUALIZER_MID2_WIDTH),
    DECL(AL_EQUALIZER_HIGH_GAIN),
    DECL(AL_EQUALIZER_HIGH_CUTOFF),

    DECL(AL_DEDICATED_GAIN),

    DECL(AL_NUM_RESAMPLERS_SOFT),
    DECL(AL_DEFAULT_RESAMPLER_SOFT),
    DECL(AL_SOURCE_RESAMPLER_SOFT),
    DECL(AL_RESAMPLER_NAME_SOFT),

    DECL(AL_SOURCE_SPATIALIZE_SOFT),
    DECL(AL_AUTO_SOFT),

    DECL(AL_MAP_READ_BIT_SOFT),
    DECL(AL_MAP_WRITE_BIT_SOFT),
    DECL(AL_MAP_PERSISTENT_BIT_SOFT),
    DECL(AL_PRESERVE_DATA_BIT_SOFT),

    DECL(AL_EVENT_CALLBACK_FUNCTION_SOFT),
    DECL(AL_EVENT_CALLBACK_USER_PARAM_SOFT),
    DECL(AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT),
    DECL(AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT),
    DECL(AL_EVENT_TYPE_ERROR_SOFT),
    DECL(AL_EVENT_TYPE_PERFORMANCE_SOFT),
    DECL(AL_EVENT_TYPE_DEPRECATED_SOFT),
};
#undef DECL

static const ALCchar alcNoError[] = "No Error";
static const ALCchar alcErrInvalidDevice[] = "Invalid Device";
static const ALCchar alcErrInvalidContext[] = "Invalid Context";
static const ALCchar alcErrInvalidEnum[] = "Invalid Enum";
static const ALCchar alcErrInvalidValue[] = "Invalid Value";
static const ALCchar alcErrOutOfMemory[] = "Out of Memory";


/************************************************
 * Global variables
 ************************************************/

/* Enumerated device names */
static const ALCchar alcDefaultName[] = "OpenAL Soft\0";

static al_string alcAllDevicesList;
static al_string alcCaptureDeviceList;

/* Default is always the first in the list */
static ALCchar *alcDefaultAllDevicesSpecifier;
static ALCchar *alcCaptureDefaultDeviceSpecifier;

/* Default context extensions */
static const ALchar alExtList[] =
    "AL_EXT_ALAW "
    "AL_EXT_BFORMAT "
    "AL_EXT_DOUBLE "
    "AL_EXT_EXPONENT_DISTANCE "
    "AL_EXT_FLOAT32 "
    "AL_EXT_IMA4 "
    "AL_EXT_LINEAR_DISTANCE "
    "AL_EXT_MCFORMATS "
    "AL_EXT_MULAW "
    "AL_EXT_MULAW_BFORMAT "
    "AL_EXT_MULAW_MCFORMATS "
    "AL_EXT_OFFSET "
    "AL_EXT_source_distance_model "
    "AL_EXT_SOURCE_RADIUS "
    "AL_EXT_STEREO_ANGLES "
    "AL_LOKI_quadriphonic "
    "AL_SOFT_block_alignment "
    "AL_SOFT_deferred_updates "
    "AL_SOFT_direct_channels "
    "AL_SOFTX_events "
    "AL_SOFT_gain_clamp_ex "
    "AL_SOFT_loop_points "
    "AL_SOFTX_map_buffer "
    "AL_SOFT_MSADPCM "
    "AL_SOFT_source_latency "
    "AL_SOFT_source_length "
    "AL_SOFT_source_resampler "
    "AL_SOFT_source_spatialize";

static ATOMIC(ALCenum) LastNullDeviceError = ATOMIC_INIT_STATIC(ALC_NO_ERROR);

/* Thread-local current context */
static altss_t LocalContext;
/* Process-wide current context */
static ATOMIC(ALCcontext*) GlobalContext = ATOMIC_INIT_STATIC(NULL);

/* Mixing thread piority level */
ALint RTPrioLevel;

FILE *LogFile;
#ifdef _DEBUG
enum LogLevel LogLevel = LogWarning;
#else
enum LogLevel LogLevel = LogError;
#endif

/* Flag to trap ALC device errors */
static ALCboolean TrapALCError = ALC_FALSE;

/* One-time configuration init control */
static alonce_flag alc_config_once = AL_ONCE_FLAG_INIT;

/* Default effect that applies to sources that don't have an effect on send 0 */
static ALeffect DefaultEffect;

/* Flag to specify if alcSuspendContext/alcProcessContext should defer/process
 * updates.
 */
static ALCboolean SuspendDefers = ALC_TRUE;


/************************************************
 * ALC information
 ************************************************/
static const ALCchar alcNoDeviceExtList[] =
    "ALC_ENUMERATE_ALL_EXT ALC_ENUMERATION_EXT ALC_EXT_CAPTURE "
    "ALC_EXT_thread_local_context ALC_SOFT_loopback";
static const ALCchar alcExtensionList[] =
    "ALC_ENUMERATE_ALL_EXT ALC_ENUMERATION_EXT ALC_EXT_CAPTURE "
    "ALC_EXT_DEDICATED ALC_EXT_disconnect ALC_EXT_EFX "
    "ALC_EXT_thread_local_context ALC_SOFT_device_clock ALC_SOFT_HRTF "
    "ALC_SOFT_loopback ALC_SOFT_output_limiter ALC_SOFT_pause_device";
static const ALCint alcMajorVersion = 1;
static const ALCint alcMinorVersion = 1;

static const ALCint alcEFXMajorVersion = 1;
static const ALCint alcEFXMinorVersion = 0;


/************************************************
 * Device lists
 ************************************************/
static ATOMIC(ALCdevice*) DeviceList = ATOMIC_INIT_STATIC(NULL);

static almtx_t ListLock;
static inline void LockLists(void)
{
    int ret = almtx_lock(&ListLock);
    assert(ret == althrd_success);
}
static inline void UnlockLists(void)
{
    int ret = almtx_unlock(&ListLock);
    assert(ret == althrd_success);
}

/************************************************
 * Library initialization
 ************************************************/
#if defined(_WIN32)
static void alc_init(void);
static void alc_deinit(void);
static void alc_deinit_safe(void);

#ifndef AL_LIBTYPE_STATIC
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID lpReserved)
{
    switch(reason)
    {
        case DLL_PROCESS_ATTACH:
            /* Pin the DLL so we won't get unloaded until the process terminates */
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                               (WCHAR*)hModule, &hModule);
            alc_init();
            break;

        case DLL_THREAD_DETACH:
            althrd_thread_detach();
            break;

        case DLL_PROCESS_DETACH:
            if(!lpReserved)
                alc_deinit();
            else
                alc_deinit_safe();
            break;
    }
    return TRUE;
}
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
static void alc_constructor(void);
static void alc_destructor(void);
__declspec(allocate(".CRT$XCU")) void (__cdecl* alc_constructor_)(void) = alc_constructor;

static void alc_constructor(void)
{
    atexit(alc_destructor);
    alc_init();
}

static void alc_destructor(void)
{
    alc_deinit();
}
#elif defined(HAVE_GCC_DESTRUCTOR)
static void alc_init(void) __attribute__((constructor));
static void alc_deinit(void) __attribute__((destructor));
#else
#error "No static initialization available on this platform!"
#endif

#elif defined(HAVE_GCC_DESTRUCTOR)

static void alc_init(void) __attribute__((constructor));
static void alc_deinit(void) __attribute__((destructor));

#else
#error "No global initialization available on this platform!"
#endif

static void ReleaseThreadCtx(void *ptr);
static void alc_init(void)
{
    const char *str;
    int ret;

    LogFile = stderr;

    AL_STRING_INIT(alcAllDevicesList);
    AL_STRING_INIT(alcCaptureDeviceList);

    str = getenv("__ALSOFT_HALF_ANGLE_CONES");
    if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
        ConeScale *= 0.5f;

    str = getenv("__ALSOFT_REVERSE_Z");
    if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
        ZScale *= -1.0f;

    str = getenv("__ALSOFT_REVERB_IGNORES_SOUND_SPEED");
    if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
        OverrideReverbSpeedOfSound = AL_TRUE;

    ret = altss_create(&LocalContext, ReleaseThreadCtx);
    assert(ret == althrd_success);

    ret = almtx_init(&ListLock, almtx_recursive);
    assert(ret == althrd_success);
}

static void alc_initconfig(void)
{
    const char *devs, *str;
    int capfilter;
    float valf;
    int i, n;

    str = getenv("ALSOFT_LOGLEVEL");
    if(str)
    {
        long lvl = strtol(str, NULL, 0);
        if(lvl >= NoLog && lvl <= LogRef)
            LogLevel = lvl;
    }

    str = getenv("ALSOFT_LOGFILE");
    if(str && str[0])
    {
        FILE *logfile = al_fopen(str, "wt");
        if(logfile) LogFile = logfile;
        else ERR("Failed to open log file '%s'\n", str);
    }

    TRACE("Initializing library v%s-%s %s\n", ALSOFT_VERSION,
          ALSOFT_GIT_COMMIT_HASH, ALSOFT_GIT_BRANCH);
    {
        char buf[1024] = "";
        int len = 0;

        if(BackendListSize > 0)
            len += snprintf(buf, sizeof(buf), "%s", BackendList[0].name);
        for(i = 1;i < BackendListSize;i++)
            len += snprintf(buf+len, sizeof(buf)-len, ", %s", BackendList[i].name);
        TRACE("Supported backends: %s\n", buf);
    }
    ReadALConfig();

    str = getenv("__ALSOFT_SUSPEND_CONTEXT");
    if(str && *str)
    {
        if(strcasecmp(str, "ignore") == 0)
        {
            SuspendDefers = ALC_FALSE;
            TRACE("Selected context suspend behavior, \"ignore\"\n");
        }
        else
            ERR("Unhandled context suspend behavior setting: \"%s\"\n", str);
    }

    capfilter = 0;
#if defined(HAVE_SSE4_1)
    capfilter |= CPU_CAP_SSE | CPU_CAP_SSE2 | CPU_CAP_SSE3 | CPU_CAP_SSE4_1;
#elif defined(HAVE_SSE3)
    capfilter |= CPU_CAP_SSE | CPU_CAP_SSE2 | CPU_CAP_SSE3;
#elif defined(HAVE_SSE2)
    capfilter |= CPU_CAP_SSE | CPU_CAP_SSE2;
#elif defined(HAVE_SSE)
    capfilter |= CPU_CAP_SSE;
#endif
#ifdef HAVE_NEON
    capfilter |= CPU_CAP_NEON;
#endif
    if(ConfigValueStr(NULL, NULL, "disable-cpu-exts", &str))
    {
        if(strcasecmp(str, "all") == 0)
            capfilter = 0;
        else
        {
            size_t len;
            const char *next = str;

            do {
                str = next;
                while(isspace(str[0]))
                    str++;
                next = strchr(str, ',');

                if(!str[0] || str[0] == ',')
                    continue;

                len = (next ? ((size_t)(next-str)) : strlen(str));
                while(len > 0 && isspace(str[len-1]))
                    len--;
                if(len == 3 && strncasecmp(str, "sse", len) == 0)
                    capfilter &= ~CPU_CAP_SSE;
                else if(len == 4 && strncasecmp(str, "sse2", len) == 0)
                    capfilter &= ~CPU_CAP_SSE2;
                else if(len == 4 && strncasecmp(str, "sse3", len) == 0)
                    capfilter &= ~CPU_CAP_SSE3;
                else if(len == 6 && strncasecmp(str, "sse4.1", len) == 0)
                    capfilter &= ~CPU_CAP_SSE4_1;
                else if(len == 4 && strncasecmp(str, "neon", len) == 0)
                    capfilter &= ~CPU_CAP_NEON;
                else
                    WARN("Invalid CPU extension \"%s\"\n", str);
            } while(next++);
        }
    }
    FillCPUCaps(capfilter);

#ifdef _WIN32
    RTPrioLevel = 1;
#else
    RTPrioLevel = 0;
#endif
    ConfigValueInt(NULL, NULL, "rt-prio", &RTPrioLevel);

    aluInit();
    aluInitMixer();

    str = getenv("ALSOFT_TRAP_ERROR");
    if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
    {
        TrapALError  = AL_TRUE;
        TrapALCError = AL_TRUE;
    }
    else
    {
        str = getenv("ALSOFT_TRAP_AL_ERROR");
        if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
            TrapALError = AL_TRUE;
        TrapALError = GetConfigValueBool(NULL, NULL, "trap-al-error", TrapALError);

        str = getenv("ALSOFT_TRAP_ALC_ERROR");
        if(str && (strcasecmp(str, "true") == 0 || strtol(str, NULL, 0) == 1))
            TrapALCError = ALC_TRUE;
        TrapALCError = GetConfigValueBool(NULL, NULL, "trap-alc-error", TrapALCError);
    }

    if(ConfigValueFloat(NULL, "reverb", "boost", &valf))
        ReverbBoost *= powf(10.0f, valf / 20.0f);

    if(((devs=getenv("ALSOFT_DRIVERS")) && devs[0]) ||
       ConfigValueStr(NULL, NULL, "drivers", &devs))
    {
        int n;
        size_t len;
        const char *next = devs;
        int endlist, delitem;

        i = 0;
        do {
            devs = next;
            while(isspace(devs[0]))
                devs++;
            next = strchr(devs, ',');

            delitem = (devs[0] == '-');
            if(devs[0] == '-') devs++;

            if(!devs[0] || devs[0] == ',')
            {
                endlist = 0;
                continue;
            }
            endlist = 1;

            len = (next ? ((size_t)(next-devs)) : strlen(devs));
            while(len > 0 && isspace(devs[len-1]))
                len--;
#ifdef HAVE_WASAPI
            /* HACK: For backwards compatibility, convert backend references of
             * mmdevapi to wasapi. This should eventually be removed.
             */
            if(len == 8 && strncmp(devs, "mmdevapi", len) == 0)
            {
                devs = "wasapi";
                len = 6;
            }
#endif
            for(n = i;n < BackendListSize;n++)
            {
                if(len == strlen(BackendList[n].name) &&
                   strncmp(BackendList[n].name, devs, len) == 0)
                {
                    if(delitem)
                    {
                        for(;n+1 < BackendListSize;n++)
                            BackendList[n] = BackendList[n+1];
                        BackendListSize--;
                    }
                    else
                    {
                        struct BackendInfo Bkp = BackendList[n];
                        for(;n > i;n--)
                            BackendList[n] = BackendList[n-1];
                        BackendList[n] = Bkp;

                        i++;
                    }
                    break;
                }
            }
        } while(next++);

        if(endlist)
            BackendListSize = i;
    }

    for(n = i = 0;i < BackendListSize && (!PlaybackBackend.name || !CaptureBackend.name);i++)
    {
        ALCbackendFactory *factory;
        BackendList[n] = BackendList[i];

        factory = BackendList[n].getFactory();
        if(!V0(factory,init)())
        {
            WARN("Failed to initialize backend \"%s\"\n", BackendList[n].name);
            continue;
        }

        TRACE("Initialized backend \"%s\"\n", BackendList[n].name);
        if(!PlaybackBackend.name && V(factory,querySupport)(ALCbackend_Playback))
        {
            PlaybackBackend = BackendList[n];
            TRACE("Added \"%s\" for playback\n", PlaybackBackend.name);
        }
        if(!CaptureBackend.name && V(factory,querySupport)(ALCbackend_Capture))
        {
            CaptureBackend = BackendList[n];
            TRACE("Added \"%s\" for capture\n", CaptureBackend.name);
        }
        n++;
    }
    BackendListSize = n;

    {
        ALCbackendFactory *factory = ALCloopbackFactory_getFactory();
        V0(factory,init)();
    }

    if(!PlaybackBackend.name)
        WARN("No playback backend available!\n");
    if(!CaptureBackend.name)
        WARN("No capture backend available!\n");

    if(ConfigValueStr(NULL, NULL, "excludefx", &str))
    {
        size_t len;
        const char *next = str;

        do {
            str = next;
            next = strchr(str, ',');

            if(!str[0] || next == str)
                continue;

            len = (next ? ((size_t)(next-str)) : strlen(str));
            for(n = 0;n < EFFECTLIST_SIZE;n++)
            {
                if(len == strlen(EffectList[n].name) &&
                   strncmp(EffectList[n].name, str, len) == 0)
                    DisabledEffects[EffectList[n].type] = AL_TRUE;
            }
        } while(next++);
    }

    InitEffect(&DefaultEffect);
    str = getenv("ALSOFT_DEFAULT_REVERB");
    if((str && str[0]) || ConfigValueStr(NULL, NULL, "default-reverb", &str))
        LoadReverbPreset(str, &DefaultEffect);
}
#define DO_INITCONFIG() alcall_once(&alc_config_once, alc_initconfig)

#ifdef __ANDROID__
#include <jni.h>

static JavaVM *gJavaVM;
static pthread_key_t gJVMThreadKey;

static void CleanupJNIEnv(void* UNUSED(ptr))
{
    JCALL0(gJavaVM,DetachCurrentThread)();
}

void *Android_GetJNIEnv(void)
{
    if(!gJavaVM)
    {
        WARN("gJavaVM is NULL!\n");
        return NULL;
    }

    /* http://developer.android.com/guide/practices/jni.html
     *
     * All threads are Linux threads, scheduled by the kernel. They're usually
     * started from managed code (using Thread.start), but they can also be
     * created elsewhere and then attached to the JavaVM. For example, a thread
     * started with pthread_create can be attached with the JNI
     * AttachCurrentThread or AttachCurrentThreadAsDaemon functions. Until a
     * thread is attached, it has no JNIEnv, and cannot make JNI calls.
     * Attaching a natively-created thread causes a java.lang.Thread object to
     * be constructed and added to the "main" ThreadGroup, making it visible to
     * the debugger. Calling AttachCurrentThread on an already-attached thread
     * is a no-op.
     */
    JNIEnv *env = pthread_getspecific(gJVMThreadKey);
    if(!env)
    {
        int status = JCALL(gJavaVM,AttachCurrentThread)(&env, NULL);
        if(status < 0)
        {
            ERR("Failed to attach current thread\n");
            return NULL;
        }
        pthread_setspecific(gJVMThreadKey, env);
    }
    return env;
}

/* Automatically called by JNI. */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void* UNUSED(reserved))
{
    void *env;
    int err;

    gJavaVM = jvm;
    if(JCALL(gJavaVM,GetEnv)(&env, JNI_VERSION_1_4) != JNI_OK)
    {
        ERR("Failed to get JNIEnv with JNI_VERSION_1_4\n");
        return JNI_ERR;
    }

    /* Create gJVMThreadKey so we can keep track of the JNIEnv assigned to each
     * thread. The JNIEnv *must* be detached before the thread is destroyed.
     */
    if((err=pthread_key_create(&gJVMThreadKey, CleanupJNIEnv)) != 0)
        ERR("pthread_key_create failed: %d\n", err);
    pthread_setspecific(gJVMThreadKey, env);
    return JNI_VERSION_1_4;
}
#endif


/************************************************
 * Library deinitialization
 ************************************************/
static void alc_cleanup(void)
{
    ALCdevice *dev;

    AL_STRING_DEINIT(alcAllDevicesList);
    AL_STRING_DEINIT(alcCaptureDeviceList);

    free(alcDefaultAllDevicesSpecifier);
    alcDefaultAllDevicesSpecifier = NULL;
    free(alcCaptureDefaultDeviceSpecifier);
    alcCaptureDefaultDeviceSpecifier = NULL;

    if((dev=ATOMIC_EXCHANGE_PTR_SEQ(&DeviceList, NULL)) != NULL)
    {
        ALCuint num = 0;
        do {
            num++;
            dev = ATOMIC_LOAD(&dev->next, almemory_order_relaxed);
        } while(dev != NULL);
        ERR("%u device%s not closed\n", num, (num>1)?"s":"");
    }
}

static void alc_deinit_safe(void)
{
    alc_cleanup();

    FreeHrtfs();
    FreeALConfig();

    almtx_destroy(&ListLock);
    altss_delete(LocalContext);

    if(LogFile != stderr)
        fclose(LogFile);
    LogFile = NULL;

    althrd_deinit();
}

static void alc_deinit(void)
{
    int i;

    alc_cleanup();

    memset(&PlaybackBackend, 0, sizeof(PlaybackBackend));
    memset(&CaptureBackend, 0, sizeof(CaptureBackend));

    for(i = 0;i < BackendListSize;i++)
    {
        ALCbackendFactory *factory = BackendList[i].getFactory();
        V0(factory,deinit)();
    }
    {
        ALCbackendFactory *factory = ALCloopbackFactory_getFactory();
        V0(factory,deinit)();
    }

    alc_deinit_safe();
}


/************************************************
 * Device enumeration
 ************************************************/
static void ProbeDevices(al_string *list, struct BackendInfo *backendinfo, enum DevProbe type)
{
    DO_INITCONFIG();

    LockLists();
    alstr_clear(list);

    if(backendinfo->getFactory)
    {
        ALCbackendFactory *factory = backendinfo->getFactory();
        V(factory,probe)(type);
    }

    UnlockLists();
}
static void ProbeAllDevicesList(void)
{ ProbeDevices(&alcAllDevicesList, &PlaybackBackend, ALL_DEVICE_PROBE); }
static void ProbeCaptureDeviceList(void)
{ ProbeDevices(&alcCaptureDeviceList, &CaptureBackend, CAPTURE_DEVICE_PROBE); }

static void AppendDevice(const ALCchar *name, al_string *devnames)
{
    size_t len = strlen(name);
    if(len > 0)
        alstr_append_range(devnames, name, name+len+1);
}
void AppendAllDevicesList(const ALCchar *name)
{ AppendDevice(name, &alcAllDevicesList); }
void AppendCaptureDeviceList(const ALCchar *name)
{ AppendDevice(name, &alcCaptureDeviceList); }


/************************************************
 * Device format information
 ************************************************/
const ALCchar *DevFmtTypeString(enum DevFmtType type)
{
    switch(type)
    {
    case DevFmtByte: return "Signed Byte";
    case DevFmtUByte: return "Unsigned Byte";
    case DevFmtShort: return "Signed Short";
    case DevFmtUShort: return "Unsigned Short";
    case DevFmtInt: return "Signed Int";
    case DevFmtUInt: return "Unsigned Int";
    case DevFmtFloat: return "Float";
    }
    return "(unknown type)";
}
const ALCchar *DevFmtChannelsString(enum DevFmtChannels chans)
{
    switch(chans)
    {
    case DevFmtMono: return "Mono";
    case DevFmtStereo: return "Stereo";
    case DevFmtQuad: return "Quadraphonic";
    case DevFmtX51: return "5.1 Surround";
    case DevFmtX51Rear: return "5.1 Surround (Rear)";
    case DevFmtX61: return "6.1 Surround";
    case DevFmtX71: return "7.1 Surround";
    case DevFmtAmbi3D: return "Ambisonic 3D";
    }
    return "(unknown channels)";
}

extern inline ALsizei FrameSizeFromDevFmt(enum DevFmtChannels chans, enum DevFmtType type, ALsizei ambiorder);
ALsizei BytesFromDevFmt(enum DevFmtType type)
{
    switch(type)
    {
    case DevFmtByte: return sizeof(ALbyte);
    case DevFmtUByte: return sizeof(ALubyte);
    case DevFmtShort: return sizeof(ALshort);
    case DevFmtUShort: return sizeof(ALushort);
    case DevFmtInt: return sizeof(ALint);
    case DevFmtUInt: return sizeof(ALuint);
    case DevFmtFloat: return sizeof(ALfloat);
    }
    return 0;
}
ALsizei ChannelsFromDevFmt(enum DevFmtChannels chans, ALsizei ambiorder)
{
    switch(chans)
    {
    case DevFmtMono: return 1;
    case DevFmtStereo: return 2;
    case DevFmtQuad: return 4;
    case DevFmtX51: return 6;
    case DevFmtX51Rear: return 6;
    case DevFmtX61: return 7;
    case DevFmtX71: return 8;
    case DevFmtAmbi3D: return (ambiorder >= 3) ? 16 :
                              (ambiorder == 2) ? 9 :
                              (ambiorder == 1) ? 4 : 1;
    }
    return 0;
}

static ALboolean DecomposeDevFormat(ALenum format, enum DevFmtChannels *chans,
                                    enum DevFmtType *type)
{
    static const struct {
        ALenum format;
        enum DevFmtChannels channels;
        enum DevFmtType type;
    } list[] = {
        { AL_FORMAT_MONO8,        DevFmtMono, DevFmtUByte },
        { AL_FORMAT_MONO16,       DevFmtMono, DevFmtShort },
        { AL_FORMAT_MONO_FLOAT32, DevFmtMono, DevFmtFloat },

        { AL_FORMAT_STEREO8,        DevFmtStereo, DevFmtUByte },
        { AL_FORMAT_STEREO16,       DevFmtStereo, DevFmtShort },
        { AL_FORMAT_STEREO_FLOAT32, DevFmtStereo, DevFmtFloat },

        { AL_FORMAT_QUAD8,  DevFmtQuad, DevFmtUByte },
        { AL_FORMAT_QUAD16, DevFmtQuad, DevFmtShort },
        { AL_FORMAT_QUAD32, DevFmtQuad, DevFmtFloat },

        { AL_FORMAT_51CHN8,  DevFmtX51, DevFmtUByte },
        { AL_FORMAT_51CHN16, DevFmtX51, DevFmtShort },
        { AL_FORMAT_51CHN32, DevFmtX51, DevFmtFloat },

        { AL_FORMAT_61CHN8,  DevFmtX61, DevFmtUByte },
        { AL_FORMAT_61CHN16, DevFmtX61, DevFmtShort },
        { AL_FORMAT_61CHN32, DevFmtX61, DevFmtFloat },

        { AL_FORMAT_71CHN8,  DevFmtX71, DevFmtUByte },
        { AL_FORMAT_71CHN16, DevFmtX71, DevFmtShort },
        { AL_FORMAT_71CHN32, DevFmtX71, DevFmtFloat },
    };
    ALuint i;

    for(i = 0;i < COUNTOF(list);i++)
    {
        if(list[i].format == format)
        {
            *chans = list[i].channels;
            *type  = list[i].type;
            return AL_TRUE;
        }
    }

    return AL_FALSE;
}

static ALCboolean IsValidALCType(ALCenum type)
{
    switch(type)
    {
        case ALC_BYTE_SOFT:
        case ALC_UNSIGNED_BYTE_SOFT:
        case ALC_SHORT_SOFT:
        case ALC_UNSIGNED_SHORT_SOFT:
        case ALC_INT_SOFT:
        case ALC_UNSIGNED_INT_SOFT:
        case ALC_FLOAT_SOFT:
            return ALC_TRUE;
    }
    return ALC_FALSE;
}

static ALCboolean IsValidALCChannels(ALCenum channels)
{
    switch(channels)
    {
        case ALC_MONO_SOFT:
        case ALC_STEREO_SOFT:
        case ALC_QUAD_SOFT:
        case ALC_5POINT1_SOFT:
        case ALC_6POINT1_SOFT:
        case ALC_7POINT1_SOFT:
        case ALC_BFORMAT3D_SOFT:
            return ALC_TRUE;
    }
    return ALC_FALSE;
}

static ALCboolean IsValidAmbiLayout(ALCenum layout)
{
    switch(layout)
    {
        case ALC_ACN_SOFT:
        case ALC_FUMA_SOFT:
            return ALC_TRUE;
    }
    return ALC_FALSE;
}

static ALCboolean IsValidAmbiScaling(ALCenum scaling)
{
    switch(scaling)
    {
        case ALC_N3D_SOFT:
        case ALC_SN3D_SOFT:
        case ALC_FUMA_SOFT:
            return ALC_TRUE;
    }
    return ALC_FALSE;
}

/************************************************
 * Miscellaneous ALC helpers
 ************************************************/

/* SetDefaultWFXChannelOrder
 *
 * Sets the default channel order used by WaveFormatEx.
 */
void SetDefaultWFXChannelOrder(ALCdevice *device)
{
    ALsizei i;

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        device->RealOut.ChannelName[i] = InvalidChannel;

    switch(device->FmtChans)
    {
    case DevFmtMono:
        device->RealOut.ChannelName[0] = FrontCenter;
        break;
    case DevFmtStereo:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        break;
    case DevFmtQuad:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = BackLeft;
        device->RealOut.ChannelName[3] = BackRight;
        break;
    case DevFmtX51:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = FrontCenter;
        device->RealOut.ChannelName[3] = LFE;
        device->RealOut.ChannelName[4] = SideLeft;
        device->RealOut.ChannelName[5] = SideRight;
        break;
    case DevFmtX51Rear:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = FrontCenter;
        device->RealOut.ChannelName[3] = LFE;
        device->RealOut.ChannelName[4] = BackLeft;
        device->RealOut.ChannelName[5] = BackRight;
        break;
    case DevFmtX61:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = FrontCenter;
        device->RealOut.ChannelName[3] = LFE;
        device->RealOut.ChannelName[4] = BackCenter;
        device->RealOut.ChannelName[5] = SideLeft;
        device->RealOut.ChannelName[6] = SideRight;
        break;
    case DevFmtX71:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = FrontCenter;
        device->RealOut.ChannelName[3] = LFE;
        device->RealOut.ChannelName[4] = BackLeft;
        device->RealOut.ChannelName[5] = BackRight;
        device->RealOut.ChannelName[6] = SideLeft;
        device->RealOut.ChannelName[7] = SideRight;
        break;
    case DevFmtAmbi3D:
        device->RealOut.ChannelName[0] = Aux0;
        if(device->AmbiOrder > 0)
        {
            device->RealOut.ChannelName[1] = Aux1;
            device->RealOut.ChannelName[2] = Aux2;
            device->RealOut.ChannelName[3] = Aux3;
        }
        if(device->AmbiOrder > 1)
        {
            device->RealOut.ChannelName[4] = Aux4;
            device->RealOut.ChannelName[5] = Aux5;
            device->RealOut.ChannelName[6] = Aux6;
            device->RealOut.ChannelName[7] = Aux7;
            device->RealOut.ChannelName[8] = Aux8;
        }
        if(device->AmbiOrder > 2)
        {
            device->RealOut.ChannelName[9]  = Aux9;
            device->RealOut.ChannelName[10] = Aux10;
            device->RealOut.ChannelName[11] = Aux11;
            device->RealOut.ChannelName[12] = Aux12;
            device->RealOut.ChannelName[13] = Aux13;
            device->RealOut.ChannelName[14] = Aux14;
            device->RealOut.ChannelName[15] = Aux15;
        }
        break;
    }
}

/* SetDefaultChannelOrder
 *
 * Sets the default channel order used by most non-WaveFormatEx-based APIs.
 */
void SetDefaultChannelOrder(ALCdevice *device)
{
    ALsizei i;

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
        device->RealOut.ChannelName[i] = InvalidChannel;

    switch(device->FmtChans)
    {
    case DevFmtX51Rear:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = BackLeft;
        device->RealOut.ChannelName[3] = BackRight;
        device->RealOut.ChannelName[4] = FrontCenter;
        device->RealOut.ChannelName[5] = LFE;
        return;
    case DevFmtX71:
        device->RealOut.ChannelName[0] = FrontLeft;
        device->RealOut.ChannelName[1] = FrontRight;
        device->RealOut.ChannelName[2] = BackLeft;
        device->RealOut.ChannelName[3] = BackRight;
        device->RealOut.ChannelName[4] = FrontCenter;
        device->RealOut.ChannelName[5] = LFE;
        device->RealOut.ChannelName[6] = SideLeft;
        device->RealOut.ChannelName[7] = SideRight;
        return;

    /* Same as WFX order */
    case DevFmtMono:
    case DevFmtStereo:
    case DevFmtQuad:
    case DevFmtX51:
    case DevFmtX61:
    case DevFmtAmbi3D:
        SetDefaultWFXChannelOrder(device);
        break;
    }
}

extern inline ALint GetChannelIndex(const enum Channel names[MAX_OUTPUT_CHANNELS], enum Channel chan);
extern inline ALint GetChannelIdxByName(const RealMixParams *real, enum Channel chan);


/* ALCcontext_DeferUpdates
 *
 * Defers/suspends updates for the given context's listener and sources. This
 * does *NOT* stop mixing, but rather prevents certain property changes from
 * taking effect.
 */
void ALCcontext_DeferUpdates(ALCcontext *context)
{
    ATOMIC_STORE_SEQ(&context->DeferUpdates, AL_TRUE);
}

/* ALCcontext_ProcessUpdates
 *
 * Resumes update processing after being deferred.
 */
void ALCcontext_ProcessUpdates(ALCcontext *context)
{
    almtx_lock(&context->PropLock);
    if(ATOMIC_EXCHANGE_SEQ(&context->DeferUpdates, AL_FALSE))
    {
        /* Tell the mixer to stop applying updates, then wait for any active
         * updating to finish, before providing updates.
         */
        ATOMIC_STORE_SEQ(&context->HoldUpdates, AL_TRUE);
        while((ATOMIC_LOAD(&context->UpdateCount, almemory_order_acquire)&1) != 0)
            althrd_yield();

        if(!ATOMIC_FLAG_TEST_AND_SET(&context->PropsClean, almemory_order_acq_rel))
            UpdateContextProps(context);
        if(!ATOMIC_FLAG_TEST_AND_SET(&context->Listener->PropsClean, almemory_order_acq_rel))
            UpdateListenerProps(context);
        UpdateAllEffectSlotProps(context);
        UpdateAllSourceProps(context);

        /* Now with all updates declared, let the mixer continue applying them
         * so they all happen at once.
         */
        ATOMIC_STORE_SEQ(&context->HoldUpdates, AL_FALSE);
    }
    almtx_unlock(&context->PropLock);
}


/* alcSetError
 *
 * Stores the latest ALC device error
 */
static void alcSetError(ALCdevice *device, ALCenum errorCode)
{
    WARN("Error generated on device %p, code 0x%04x\n", device, errorCode);
    if(TrapALCError)
    {
#ifdef _WIN32
        /* DebugBreak() will cause an exception if there is no debugger */
        if(IsDebuggerPresent())
            DebugBreak();
#elif defined(SIGTRAP)
        raise(SIGTRAP);
#endif
    }

    if(device)
        ATOMIC_STORE_SEQ(&device->LastError, errorCode);
    else
        ATOMIC_STORE_SEQ(&LastNullDeviceError, errorCode);
}


struct Compressor *CreateDeviceLimiter(const ALCdevice *device)
{
    return CompressorInit(0.0f, 0.0f, AL_FALSE, AL_TRUE, 0.0f, 0.0f, 0.5f, 2.0f,
                          0.0f, -3.0f, 3.0f, device->Frequency);
}

/* UpdateClockBase
 *
 * Updates the device's base clock time with however many samples have been
 * done. This is used so frequency changes on the device don't cause the time
 * to jump forward or back. Must not be called while the device is running/
 * mixing.
 */
static inline void UpdateClockBase(ALCdevice *device)
{
    IncrementRef(&device->MixCount);
    device->ClockBase += device->SamplesDone * DEVICE_CLOCK_RES / device->Frequency;
    device->SamplesDone = 0;
    IncrementRef(&device->MixCount);
}

/* UpdateDeviceParams
 *
 * Updates device parameters according to the attribute list (caller is
 * responsible for holding the list lock).
 */
static ALCenum UpdateDeviceParams(ALCdevice *device, const ALCint *attrList)
{
    enum HrtfRequestMode hrtf_userreq = Hrtf_Default;
    enum HrtfRequestMode hrtf_appreq = Hrtf_Default;
    ALCenum gainLimiter = device->Limiter ? ALC_TRUE : ALC_FALSE;
    const ALsizei old_sends = device->NumAuxSends;
    ALsizei new_sends = device->NumAuxSends;
    enum DevFmtChannels oldChans;
    enum DevFmtType oldType;
    ALboolean update_failed;
    ALCsizei hrtf_id = -1;
    ALCcontext *context;
    ALCuint oldFreq;
    size_t size;
    ALCsizei i;
    int val;

    // Check for attributes
    if(device->Type == Loopback)
    {
        ALCsizei numMono, numStereo, numSends;
        ALCenum alayout = AL_NONE;
        ALCenum ascale = AL_NONE;
        ALCenum schans = AL_NONE;
        ALCenum stype = AL_NONE;
        ALCsizei attrIdx = 0;
        ALCsizei aorder = 0;
        ALCuint freq = 0;

        if(!attrList)
        {
            WARN("Missing attributes for loopback device\n");
            return ALC_INVALID_VALUE;
        }

        numMono = device->NumMonoSources;
        numStereo = device->NumStereoSources;
        numSends = old_sends;

#define TRACE_ATTR(a, v) TRACE("Loopback %s = %d\n", #a, v)
        while(attrList[attrIdx])
        {
            switch(attrList[attrIdx])
            {
                case ALC_FORMAT_CHANNELS_SOFT:
                    schans = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_FORMAT_CHANNELS_SOFT, schans);
                    if(!IsValidALCChannels(schans))
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_FORMAT_TYPE_SOFT:
                    stype = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_FORMAT_TYPE_SOFT, stype);
                    if(!IsValidALCType(stype))
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_FREQUENCY:
                    freq = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_FREQUENCY, freq);
                    if(freq < MIN_OUTPUT_RATE)
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_AMBISONIC_LAYOUT_SOFT:
                    alayout = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_AMBISONIC_LAYOUT_SOFT, alayout);
                    if(!IsValidAmbiLayout(alayout))
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_AMBISONIC_SCALING_SOFT:
                    ascale = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_AMBISONIC_SCALING_SOFT, ascale);
                    if(!IsValidAmbiScaling(ascale))
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_AMBISONIC_ORDER_SOFT:
                    aorder = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_AMBISONIC_ORDER_SOFT, aorder);
                    if(aorder < 1 || aorder > MAX_AMBI_ORDER)
                        return ALC_INVALID_VALUE;
                    break;

                case ALC_MONO_SOURCES:
                    numMono = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_MONO_SOURCES, numMono);
                    numMono = maxi(numMono, 0);
                    break;

                case ALC_STEREO_SOURCES:
                    numStereo = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_STEREO_SOURCES, numStereo);
                    numStereo = maxi(numStereo, 0);
                    break;

                case ALC_MAX_AUXILIARY_SENDS:
                    numSends = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_MAX_AUXILIARY_SENDS, numSends);
                    numSends = clampi(numSends, 0, MAX_SENDS);
                    break;

                case ALC_HRTF_SOFT:
                    TRACE_ATTR(ALC_HRTF_SOFT, attrList[attrIdx + 1]);
                    if(attrList[attrIdx + 1] == ALC_FALSE)
                        hrtf_appreq = Hrtf_Disable;
                    else if(attrList[attrIdx + 1] == ALC_TRUE)
                        hrtf_appreq = Hrtf_Enable;
                    else
                        hrtf_appreq = Hrtf_Default;
                    break;

                case ALC_HRTF_ID_SOFT:
                    hrtf_id = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_HRTF_ID_SOFT, hrtf_id);
                    break;

                case ALC_OUTPUT_LIMITER_SOFT:
                    gainLimiter = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_OUTPUT_LIMITER_SOFT, gainLimiter);
                    break;

                default:
                    TRACE("Loopback 0x%04X = %d (0x%x)\n", attrList[attrIdx],
                          attrList[attrIdx + 1], attrList[attrIdx + 1]);
                    break;
            }

            attrIdx += 2;
        }
#undef TRACE_ATTR

        if(!schans || !stype || !freq)
        {
            WARN("Missing format for loopback device\n");
            return ALC_INVALID_VALUE;
        }
        if(schans == ALC_BFORMAT3D_SOFT && (!alayout || !ascale || !aorder))
        {
            WARN("Missing ambisonic info for loopback device\n");
            return ALC_INVALID_VALUE;
        }

        if((device->Flags&DEVICE_RUNNING))
            V0(device->Backend,stop)();
        device->Flags &= ~DEVICE_RUNNING;

        UpdateClockBase(device);

        device->Frequency = freq;
        device->FmtChans = schans;
        device->FmtType = stype;
        if(schans == ALC_BFORMAT3D_SOFT)
        {
            device->AmbiOrder = aorder;
            device->AmbiLayout = alayout;
            device->AmbiScale = ascale;
        }

        if(numMono > INT_MAX-numStereo)
            numMono = INT_MAX-numStereo;
        numMono += numStereo;
        if(ConfigValueInt(NULL, NULL, "sources", &numMono))
        {
            if(numMono <= 0)
                numMono = 256;
        }
        else
            numMono = maxi(numMono, 256);
        numStereo = mini(numStereo, numMono);
        numMono -= numStereo;
        device->SourcesMax = numMono + numStereo;

        device->NumMonoSources = numMono;
        device->NumStereoSources = numStereo;

        if(ConfigValueInt(NULL, NULL, "sends", &new_sends))
            new_sends = mini(numSends, clampi(new_sends, 0, MAX_SENDS));
        else
            new_sends = numSends;
    }
    else if(attrList && attrList[0])
    {
        ALCsizei numMono, numStereo, numSends;
        ALCsizei attrIdx = 0;
        ALCuint freq;

        /* If a context is already running on the device, stop playback so the
         * device attributes can be updated. */
        if((device->Flags&DEVICE_RUNNING))
            V0(device->Backend,stop)();
        device->Flags &= ~DEVICE_RUNNING;

        UpdateClockBase(device);

        freq = device->Frequency;
        numMono = device->NumMonoSources;
        numStereo = device->NumStereoSources;
        numSends = old_sends;

#define TRACE_ATTR(a, v) TRACE("%s = %d\n", #a, v)
        while(attrList[attrIdx])
        {
            switch(attrList[attrIdx])
            {
                case ALC_FREQUENCY:
                    freq = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_FREQUENCY, freq);
                    device->Flags |= DEVICE_FREQUENCY_REQUEST;
                    break;

                case ALC_MONO_SOURCES:
                    numMono = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_MONO_SOURCES, numMono);
                    numMono = maxi(numMono, 0);
                    break;

                case ALC_STEREO_SOURCES:
                    numStereo = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_STEREO_SOURCES, numStereo);
                    numStereo = maxi(numStereo, 0);
                    break;

                case ALC_MAX_AUXILIARY_SENDS:
                    numSends = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_MAX_AUXILIARY_SENDS, numSends);
                    numSends = clampi(numSends, 0, MAX_SENDS);
                    break;

                case ALC_HRTF_SOFT:
                    TRACE_ATTR(ALC_HRTF_SOFT, attrList[attrIdx + 1]);
                    if(attrList[attrIdx + 1] == ALC_FALSE)
                        hrtf_appreq = Hrtf_Disable;
                    else if(attrList[attrIdx + 1] == ALC_TRUE)
                        hrtf_appreq = Hrtf_Enable;
                    else
                        hrtf_appreq = Hrtf_Default;
                    break;

                case ALC_HRTF_ID_SOFT:
                    hrtf_id = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_HRTF_ID_SOFT, hrtf_id);
                    break;

                case ALC_OUTPUT_LIMITER_SOFT:
                    gainLimiter = attrList[attrIdx + 1];
                    TRACE_ATTR(ALC_OUTPUT_LIMITER_SOFT, gainLimiter);
                    break;

                default:
                    TRACE("0x%04X = %d (0x%x)\n", attrList[attrIdx],
                          attrList[attrIdx + 1], attrList[attrIdx + 1]);
                    break;
            }

            attrIdx += 2;
        }
#undef TRACE_ATTR

        ConfigValueUInt(alstr_get_cstr(device->DeviceName), NULL, "frequency", &freq);
        freq = maxu(freq, MIN_OUTPUT_RATE);

        device->UpdateSize = (ALuint64)device->UpdateSize * freq /
                             device->Frequency;
        /* SSE and Neon do best with the update size being a multiple of 4 */
        if((CPUCapFlags&(CPU_CAP_SSE|CPU_CAP_NEON)) != 0)
            device->UpdateSize = (device->UpdateSize+3)&~3;

        device->Frequency = freq;

        if(numMono > INT_MAX-numStereo)
            numMono = INT_MAX-numStereo;
        numMono += numStereo;
        if(ConfigValueInt(alstr_get_cstr(device->DeviceName), NULL, "sources", &numMono))
        {
            if(numMono <= 0)
                numMono = 256;
        }
        else
            numMono = maxi(numMono, 256);
        numStereo = mini(numStereo, numMono);
        numMono -= numStereo;
        device->SourcesMax = numMono + numStereo;

        device->NumMonoSources = numMono;
        device->NumStereoSources = numStereo;

        if(ConfigValueInt(alstr_get_cstr(device->DeviceName), NULL, "sends", &new_sends))
            new_sends = mini(numSends, clampi(new_sends, 0, MAX_SENDS));
        else
            new_sends = numSends;
    }

    if((device->Flags&DEVICE_RUNNING))
        return ALC_NO_ERROR;

    al_free(device->Uhj_Encoder);
    device->Uhj_Encoder = NULL;

    al_free(device->Bs2b);
    device->Bs2b = NULL;

    al_free(device->ChannelDelay[0].Buffer);
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        device->ChannelDelay[i].Length = 0;
        device->ChannelDelay[i].Buffer = NULL;
    }

    al_free(device->Dry.Buffer);
    device->Dry.Buffer = NULL;
    device->Dry.NumChannels = 0;
    device->FOAOut.Buffer = NULL;
    device->FOAOut.NumChannels = 0;
    device->RealOut.Buffer = NULL;
    device->RealOut.NumChannels = 0;

    UpdateClockBase(device);

    device->DitherSeed = DITHER_RNG_SEED;

    /*************************************************************************
     * Update device format request if HRTF is requested
     */
    device->HrtfStatus = ALC_HRTF_DISABLED_SOFT;
    if(device->Type != Loopback)
    {
        const char *hrtf;
        if(ConfigValueStr(alstr_get_cstr(device->DeviceName), NULL, "hrtf", &hrtf))
        {
            if(strcasecmp(hrtf, "true") == 0)
                hrtf_userreq = Hrtf_Enable;
            else if(strcasecmp(hrtf, "false") == 0)
                hrtf_userreq = Hrtf_Disable;
            else if(strcasecmp(hrtf, "auto") != 0)
                ERR("Unexpected hrtf value: %s\n", hrtf);
        }

        if(hrtf_userreq == Hrtf_Enable || (hrtf_userreq != Hrtf_Disable && hrtf_appreq == Hrtf_Enable))
        {
            struct Hrtf *hrtf = NULL;
            if(VECTOR_SIZE(device->HrtfList) == 0)
            {
                VECTOR_DEINIT(device->HrtfList);
                device->HrtfList = EnumerateHrtf(device->DeviceName);
            }
            if(VECTOR_SIZE(device->HrtfList) > 0)
            {
                if(hrtf_id >= 0 && (size_t)hrtf_id < VECTOR_SIZE(device->HrtfList))
                    hrtf = GetLoadedHrtf(VECTOR_ELEM(device->HrtfList, hrtf_id).hrtf);
                else
                    hrtf = GetLoadedHrtf(VECTOR_ELEM(device->HrtfList, 0).hrtf);
            }

            if(hrtf)
            {
                device->FmtChans = DevFmtStereo;
                device->Frequency = hrtf->sampleRate;
                device->Flags |= DEVICE_CHANNELS_REQUEST | DEVICE_FREQUENCY_REQUEST;
                if(device->HrtfHandle)
                    Hrtf_DecRef(device->HrtfHandle);
                device->HrtfHandle = hrtf;
            }
            else
            {
                hrtf_userreq = Hrtf_Default;
                hrtf_appreq = Hrtf_Disable;
                device->HrtfStatus = ALC_HRTF_UNSUPPORTED_FORMAT_SOFT;
            }
        }
    }

    oldFreq  = device->Frequency;
    oldChans = device->FmtChans;
    oldType  = device->FmtType;

    TRACE("Pre-reset: %s%s, %s%s, %s%uhz, %u update size x%d\n",
        (device->Flags&DEVICE_CHANNELS_REQUEST)?"*":"", DevFmtChannelsString(device->FmtChans),
        (device->Flags&DEVICE_SAMPLE_TYPE_REQUEST)?"*":"", DevFmtTypeString(device->FmtType),
        (device->Flags&DEVICE_FREQUENCY_REQUEST)?"*":"", device->Frequency,
        device->UpdateSize, device->NumUpdates
    );

    if(V0(device->Backend,reset)() == ALC_FALSE)
        return ALC_INVALID_DEVICE;

    if(device->FmtChans != oldChans && (device->Flags&DEVICE_CHANNELS_REQUEST))
    {
        ERR("Failed to set %s, got %s instead\n", DevFmtChannelsString(oldChans),
            DevFmtChannelsString(device->FmtChans));
        device->Flags &= ~DEVICE_CHANNELS_REQUEST;
    }
    if(device->FmtType != oldType && (device->Flags&DEVICE_SAMPLE_TYPE_REQUEST))
    {
        ERR("Failed to set %s, got %s instead\n", DevFmtTypeString(oldType),
            DevFmtTypeString(device->FmtType));
        device->Flags &= ~DEVICE_SAMPLE_TYPE_REQUEST;
    }
    if(device->Frequency != oldFreq && (device->Flags&DEVICE_FREQUENCY_REQUEST))
    {
        ERR("Failed to set %uhz, got %uhz instead\n", oldFreq, device->Frequency);
        device->Flags &= ~DEVICE_FREQUENCY_REQUEST;
    }

    if((device->UpdateSize&3) != 0)
    {
        if((CPUCapFlags&CPU_CAP_SSE))
            WARN("SSE performs best with multiple of 4 update sizes (%u)\n", device->UpdateSize);
        if((CPUCapFlags&CPU_CAP_NEON))
            WARN("NEON performs best with multiple of 4 update sizes (%u)\n", device->UpdateSize);
    }

    TRACE("Post-reset: %s, %s, %uhz, %u update size x%d\n",
        DevFmtChannelsString(device->FmtChans), DevFmtTypeString(device->FmtType),
        device->Frequency, device->UpdateSize, device->NumUpdates
    );

    aluInitRenderer(device, hrtf_id, hrtf_appreq, hrtf_userreq);
    TRACE("Channel config, Dry: %d, FOA: %d, Real: %d\n", device->Dry.NumChannels,
          device->FOAOut.NumChannels, device->RealOut.NumChannels);

    /* Allocate extra channels for any post-filter output. */
    size = (device->Dry.NumChannels + device->FOAOut.NumChannels +
            device->RealOut.NumChannels)*sizeof(device->Dry.Buffer[0]);

    TRACE("Allocating "SZFMT" channels, "SZFMT" bytes\n", size/sizeof(device->Dry.Buffer[0]), size);
    device->Dry.Buffer = al_calloc(16, size);
    if(!device->Dry.Buffer)
    {
        ERR("Failed to allocate "SZFMT" bytes for mix buffer\n", size);
        return ALC_INVALID_DEVICE;
    }

    if(device->RealOut.NumChannels != 0)
        device->RealOut.Buffer = device->Dry.Buffer + device->Dry.NumChannels +
                                 device->FOAOut.NumChannels;
    else
    {
        device->RealOut.Buffer = device->Dry.Buffer;
        device->RealOut.NumChannels = device->Dry.NumChannels;
    }

    if(device->FOAOut.NumChannels != 0)
        device->FOAOut.Buffer = device->Dry.Buffer + device->Dry.NumChannels;
    else
    {
        device->FOAOut.Buffer = device->Dry.Buffer;
        device->FOAOut.NumChannels = device->Dry.NumChannels;
    }

    device->NumAuxSends = new_sends;
    TRACE("Max sources: %d (%d + %d), effect slots: %d, sends: %d\n",
          device->SourcesMax, device->NumMonoSources, device->NumStereoSources,
          device->AuxiliaryEffectSlotMax, device->NumAuxSends);

    device->DitherDepth = 0.0f;
    if(GetConfigValueBool(alstr_get_cstr(device->DeviceName), NULL, "dither", 1))
    {
        ALint depth = 0;
        ConfigValueInt(alstr_get_cstr(device->DeviceName), NULL, "dither-depth", &depth);
        if(depth <= 0)
        {
            switch(device->FmtType)
            {
                case DevFmtByte:
                case DevFmtUByte:
                    depth = 8;
                    break;
                case DevFmtShort:
                case DevFmtUShort:
                    depth = 16;
                    break;
                case DevFmtInt:
                case DevFmtUInt:
                case DevFmtFloat:
                    break;
            }
        }
        else if(depth > 24)
            depth = 24;
        device->DitherDepth = (depth > 0) ? powf(2.0f, (ALfloat)(depth-1)) : 0.0f;
    }
    if(!(device->DitherDepth > 0.0f))
        TRACE("Dithering disabled\n");
    else
        TRACE("Dithering enabled (%g-bit, %g)\n", log2f(device->DitherDepth)+1.0f,
              device->DitherDepth);

    if(ConfigValueBool(alstr_get_cstr(device->DeviceName), NULL, "output-limiter", &val))
        gainLimiter = val ? ALC_TRUE : ALC_FALSE;
    /* Valid values for gainLimiter are ALC_DONT_CARE_SOFT, ALC_TRUE, and
     * ALC_FALSE. We default to on, so ALC_DONT_CARE_SOFT is the same as
     * ALC_TRUE.
     */
    if(gainLimiter != ALC_FALSE)
    {
        if(!device->Limiter || device->Frequency != GetCompressorSampleRate(device->Limiter))
        {
            al_free(device->Limiter);
            device->Limiter = CreateDeviceLimiter(device);
        }
    }
    else
    {
        al_free(device->Limiter);
        device->Limiter = NULL;
    }
    TRACE("Output limiter %s\n", device->Limiter ? "enabled" : "disabled");

    aluSelectPostProcess(device);

    /* Need to delay returning failure until replacement Send arrays have been
     * allocated with the appropriate size.
     */
    update_failed = AL_FALSE;
    START_MIXER_MODE();
    context = ATOMIC_LOAD_SEQ(&device->ContextList);
    while(context)
    {
        SourceSubList *sublist, *subend;
        struct ALvoiceProps *vprops;
        ALsizei pos;

        if(context->DefaultSlot)
        {
            ALeffectslot *slot = context->DefaultSlot;
            ALeffectState *state = slot->Effect.State;

            state->OutBuffer = device->Dry.Buffer;
            state->OutChannels = device->Dry.NumChannels;
            if(V(state,deviceUpdate)(device) == AL_FALSE)
                update_failed = AL_TRUE;
            else
                UpdateEffectSlotProps(slot, context);
        }

        almtx_lock(&context->PropLock);
        almtx_lock(&context->EffectSlotLock);
        for(pos = 0;pos < (ALsizei)VECTOR_SIZE(context->EffectSlotList);pos++)
        {
            ALeffectslot *slot = VECTOR_ELEM(context->EffectSlotList, pos);
            ALeffectState *state = slot->Effect.State;

            state->OutBuffer = device->Dry.Buffer;
            state->OutChannels = device->Dry.NumChannels;
            if(V(state,deviceUpdate)(device) == AL_FALSE)
                update_failed = AL_TRUE;
            else
                UpdateEffectSlotProps(slot, context);
        }
        almtx_unlock(&context->EffectSlotLock);

        almtx_lock(&context->SourceLock);
        sublist = VECTOR_BEGIN(context->SourceList);
        subend = VECTOR_END(context->SourceList);
        for(;sublist != subend;++sublist)
        {
            ALuint64 usemask = ~sublist->FreeMask;
            while(usemask)
            {
                ALsizei idx = CTZ64(usemask);
                ALsource *source = sublist->Sources + idx;

                usemask &= ~(U64(1) << idx);

                if(old_sends != device->NumAuxSends)
                {
                    ALvoid *sends = al_calloc(16, device->NumAuxSends*sizeof(source->Send[0]));
                    ALsizei s;

                    memcpy(sends, source->Send,
                        mini(device->NumAuxSends, old_sends)*sizeof(source->Send[0])
                    );
                    for(s = device->NumAuxSends;s < old_sends;s++)
                    {
                        if(source->Send[s].Slot)
                            DecrementRef(&source->Send[s].Slot->ref);
                        source->Send[s].Slot = NULL;
                    }
                    al_free(source->Send);
                    source->Send = sends;
                    for(s = old_sends;s < device->NumAuxSends;s++)
                    {
                        source->Send[s].Slot = NULL;
                        source->Send[s].Gain = 1.0f;
                        source->Send[s].GainHF = 1.0f;
                        source->Send[s].HFReference = LOWPASSFREQREF;
                        source->Send[s].GainLF = 1.0f;
                        source->Send[s].LFReference = HIGHPASSFREQREF;
                    }
                }

                ATOMIC_FLAG_CLEAR(&source->PropsClean, almemory_order_release);
            }
        }

        /* Clear any pre-existing voice property structs, in case the number of
         * auxiliary sends is changing. Active sources will have updates
         * respecified in UpdateAllSourceProps.
         */
        vprops = ATOMIC_EXCHANGE_PTR(&context->FreeVoiceProps, NULL, almemory_order_acq_rel);
        while(vprops)
        {
            struct ALvoiceProps *next = ATOMIC_LOAD(&vprops->next, almemory_order_relaxed);
            al_free(vprops);
            vprops = next;
        }

        AllocateVoices(context, context->MaxVoices, old_sends);
        for(pos = 0;pos < context->VoiceCount;pos++)
        {
            ALvoice *voice = context->Voices[pos];

            al_free(ATOMIC_EXCHANGE_PTR(&voice->Update, NULL, almemory_order_acq_rel));

            if(ATOMIC_LOAD(&voice->Source, almemory_order_acquire) == NULL)
                continue;

            if(device->AvgSpeakerDist > 0.0f)
            {
                /* Reinitialize the NFC filters for new parameters. */
                ALfloat w1 = SPEEDOFSOUNDMETRESPERSEC /
                             (device->AvgSpeakerDist * device->Frequency);
                for(i = 0;i < voice->NumChannels;i++)
                    NfcFilterCreate(&voice->Direct.Params[i].NFCtrlFilter, 0.0f, w1);
            }
        }
        almtx_unlock(&context->SourceLock);

        ATOMIC_FLAG_TEST_AND_SET(&context->PropsClean, almemory_order_release);
        UpdateContextProps(context);
        ATOMIC_FLAG_TEST_AND_SET(&context->Listener->PropsClean, almemory_order_release);
        UpdateListenerProps(context);
        UpdateAllSourceProps(context);
        almtx_unlock(&context->PropLock);

        context = ATOMIC_LOAD(&context->next, almemory_order_relaxed);
    }
    END_MIXER_MODE();
    if(update_failed)
        return ALC_INVALID_DEVICE;

    if(!(device->Flags&DEVICE_PAUSED))
    {
        if(V0(device->Backend,start)() == ALC_FALSE)
            return ALC_INVALID_DEVICE;
        device->Flags |= DEVICE_RUNNING;
    }

    return ALC_NO_ERROR;
}


static void InitDevice(ALCdevice *device, enum DeviceType type)
{
    ALsizei i;

    InitRef(&device->ref, 1);
    ATOMIC_INIT(&device->Connected, ALC_TRUE);
    device->Type = type;
    ATOMIC_INIT(&device->LastError, ALC_NO_ERROR);

    device->Flags = 0;
    device->Render_Mode = NormalRender;
    device->AvgSpeakerDist = 0.0f;

    ATOMIC_INIT(&device->ContextList, NULL);

    device->ClockBase = 0;
    device->SamplesDone = 0;

    device->SourcesMax = 0;
    device->AuxiliaryEffectSlotMax = 0;
    device->NumAuxSends = 0;

    device->Dry.Buffer = NULL;
    device->Dry.NumChannels = 0;
    device->FOAOut.Buffer = NULL;
    device->FOAOut.NumChannels = 0;
    device->RealOut.Buffer = NULL;
    device->RealOut.NumChannels = 0;

    AL_STRING_INIT(device->DeviceName);

    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        device->ChannelDelay[i].Gain   = 1.0f;
        device->ChannelDelay[i].Length = 0;
        device->ChannelDelay[i].Buffer = NULL;
    }

    AL_STRING_INIT(device->HrtfName);
    VECTOR_INIT(device->HrtfList);
    device->HrtfHandle = NULL;
    device->Hrtf = NULL;
    device->Bs2b = NULL;
    device->Uhj_Encoder = NULL;
    device->AmbiDecoder = NULL;
    device->AmbiUp = NULL;
    device->Stablizer = NULL;
    device->Limiter = NULL;

    VECTOR_INIT(device->BufferList);
    almtx_init(&device->BufferLock, almtx_plain);

    VECTOR_INIT(device->EffectList);
    almtx_init(&device->EffectLock, almtx_plain);

    VECTOR_INIT(device->FilterList);
    almtx_init(&device->FilterLock, almtx_plain);

    almtx_init(&device->BackendLock, almtx_plain);
    device->Backend = NULL;

    ATOMIC_INIT(&device->next, NULL);
}

/* FreeDevice
 *
 * Frees the device structure, and destroys any objects the app failed to
 * delete. Called once there's no more references on the device.
 */
static ALCvoid FreeDevice(ALCdevice *device)
{
    ALsizei i;

    TRACE("%p\n", device);

    if(device->Backend)
        DELETE_OBJ(device->Backend);
    device->Backend = NULL;

    almtx_destroy(&device->BackendLock);

    ReleaseALBuffers(device);
#define FREE_BUFFERSUBLIST(x) al_free((x)->Buffers)
    VECTOR_FOR_EACH(BufferSubList, device->BufferList, FREE_BUFFERSUBLIST);
#undef FREE_BUFFERSUBLIST
    VECTOR_DEINIT(device->BufferList);
    almtx_destroy(&device->BufferLock);

    ReleaseALEffects(device);
#define FREE_EFFECTSUBLIST(x) al_free((x)->Effects)
    VECTOR_FOR_EACH(EffectSubList, device->EffectList, FREE_EFFECTSUBLIST);
#undef FREE_EFFECTSUBLIST
    VECTOR_DEINIT(device->EffectList);
    almtx_destroy(&device->EffectLock);

    ReleaseALFilters(device);
#define FREE_FILTERSUBLIST(x) al_free((x)->Filters)
    VECTOR_FOR_EACH(FilterSubList, device->FilterList, FREE_FILTERSUBLIST);
#undef FREE_FILTERSUBLIST
    VECTOR_DEINIT(device->FilterList);
    almtx_destroy(&device->FilterLock);

    AL_STRING_DEINIT(device->HrtfName);
    FreeHrtfList(&device->HrtfList);
    if(device->HrtfHandle)
        Hrtf_DecRef(device->HrtfHandle);
    device->HrtfHandle = NULL;
    al_free(device->Hrtf);
    device->Hrtf = NULL;

    al_free(device->Bs2b);
    device->Bs2b = NULL;

    al_free(device->Uhj_Encoder);
    device->Uhj_Encoder = NULL;

    bformatdec_free(&device->AmbiDecoder);
    ambiup_free(&device->AmbiUp);

    al_free(device->Stablizer);
    device->Stablizer = NULL;

    al_free(device->Limiter);
    device->Limiter = NULL;

    al_free(device->ChannelDelay[0].Buffer);
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        device->ChannelDelay[i].Gain   = 1.0f;
        device->ChannelDelay[i].Length = 0;
        device->ChannelDelay[i].Buffer = NULL;
    }

    AL_STRING_DEINIT(device->DeviceName);

    al_free(device->Dry.Buffer);
    device->Dry.Buffer = NULL;
    device->Dry.NumChannels = 0;
    device->FOAOut.Buffer = NULL;
    device->FOAOut.NumChannels = 0;
    device->RealOut.Buffer = NULL;
    device->RealOut.NumChannels = 0;

    al_free(device);
}


void ALCdevice_IncRef(ALCdevice *device)
{
    uint ref;
    ref = IncrementRef(&device->ref);
    TRACEREF("%p increasing refcount to %u\n", device, ref);
}

void ALCdevice_DecRef(ALCdevice *device)
{
    uint ref;
    ref = DecrementRef(&device->ref);
    TRACEREF("%p decreasing refcount to %u\n", device, ref);
    if(ref == 0) FreeDevice(device);
}

/* VerifyDevice
 *
 * Checks if the device handle is valid, and increments its ref count if so.
 */
static ALCboolean VerifyDevice(ALCdevice **device)
{
    ALCdevice *tmpDevice;

    LockLists();
    tmpDevice = ATOMIC_LOAD_SEQ(&DeviceList);
    while(tmpDevice)
    {
        if(tmpDevice == *device)
        {
            ALCdevice_IncRef(tmpDevice);
            UnlockLists();
            return ALC_TRUE;
        }
        tmpDevice = ATOMIC_LOAD(&tmpDevice->next, almemory_order_relaxed);
    }
    UnlockLists();

    *device = NULL;
    return ALC_FALSE;
}


/* InitContext
 *
 * Initializes context fields
 */
static ALvoid InitContext(ALCcontext *Context)
{
    ALlistener *listener = Context->Listener;
    struct ALeffectslotArray *auxslots;

    //Initialise listener
    listener->Gain = 1.0f;
    listener->Position[0] = 0.0f;
    listener->Position[1] = 0.0f;
    listener->Position[2] = 0.0f;
    listener->Velocity[0] = 0.0f;
    listener->Velocity[1] = 0.0f;
    listener->Velocity[2] = 0.0f;
    listener->Forward[0] = 0.0f;
    listener->Forward[1] = 0.0f;
    listener->Forward[2] = -1.0f;
    listener->Up[0] = 0.0f;
    listener->Up[1] = 1.0f;
    listener->Up[2] = 0.0f;
    ATOMIC_FLAG_TEST_AND_SET(&listener->PropsClean, almemory_order_relaxed);

    ATOMIC_INIT(&listener->Update, NULL);

    //Validate Context
    InitRef(&Context->UpdateCount, 0);
    ATOMIC_INIT(&Context->HoldUpdates, AL_FALSE);
    Context->GainBoost = 1.0f;
    almtx_init(&Context->PropLock, almtx_plain);
    ATOMIC_INIT(&Context->LastError, AL_NO_ERROR);
    VECTOR_INIT(Context->SourceList);
    Context->NumSources = 0;
    almtx_init(&Context->SourceLock, almtx_plain);
    VECTOR_INIT(Context->EffectSlotList);
    almtx_init(&Context->EffectSlotLock, almtx_plain);

    if(Context->DefaultSlot)
    {
        auxslots = al_calloc(DEF_ALIGN, FAM_SIZE(struct ALeffectslotArray, slot, 1));
        auxslots->count = 1;
        auxslots->slot[0] = Context->DefaultSlot;
    }
    else
    {
        auxslots = al_calloc(DEF_ALIGN, sizeof(struct ALeffectslotArray));
        auxslots->count = 0;
    }
    ATOMIC_INIT(&Context->ActiveAuxSlots, auxslots);

    //Set globals
    Context->DistanceModel = DefaultDistanceModel;
    Context->SourceDistanceModel = AL_FALSE;
    Context->DopplerFactor = 1.0f;
    Context->DopplerVelocity = 1.0f;
    Context->SpeedOfSound = SPEEDOFSOUNDMETRESPERSEC;
    Context->MetersPerUnit = AL_DEFAULT_METERS_PER_UNIT;
    ATOMIC_FLAG_TEST_AND_SET(&Context->PropsClean, almemory_order_relaxed);
    ATOMIC_INIT(&Context->DeferUpdates, AL_FALSE);
    almtx_init(&Context->EventThrdLock, almtx_plain);
    alsem_init(&Context->EventSem, 0);
    Context->AsyncEvents = NULL;
    ATOMIC_INIT(&Context->EnabledEvts, 0);
    almtx_init(&Context->EventCbLock, almtx_plain);
    Context->EventCb = NULL;
    Context->EventParam = NULL;

    ATOMIC_INIT(&Context->Update, NULL);
    ATOMIC_INIT(&Context->FreeContextProps, NULL);
    ATOMIC_INIT(&Context->FreeListenerProps, NULL);
    ATOMIC_INIT(&Context->FreeVoiceProps, NULL);
    ATOMIC_INIT(&Context->FreeEffectslotProps, NULL);

    Context->ExtensionList = alExtList;


    listener->Params.Matrix = IdentityMatrixf;
    aluVectorSet(&listener->Params.Velocity, 0.0f, 0.0f, 0.0f, 0.0f);
    listener->Params.Gain = listener->Gain;
    listener->Params.MetersPerUnit = Context->MetersPerUnit;
    listener->Params.DopplerFactor = Context->DopplerFactor;
    listener->Params.SpeedOfSound = Context->SpeedOfSound * Context->DopplerVelocity;
    listener->Params.ReverbSpeedOfSound = listener->Params.SpeedOfSound *
                                          listener->Params.MetersPerUnit;
    listener->Params.SourceDistanceModel = Context->SourceDistanceModel;
    listener->Params.DistanceModel = Context->DistanceModel;
}


/* FreeContext
 *
 * Cleans up the context, and destroys any remaining objects the app failed to
 * delete. Called once there's no more references on the context.
 */
static void FreeContext(ALCcontext *context)
{
    ALlistener *listener = context->Listener;
    struct ALeffectslotArray *auxslots;
    struct ALeffectslotProps *eprops;
    struct ALlistenerProps *lprops;
    struct ALcontextProps *cprops;
    struct ALvoiceProps *vprops;
    size_t count;
    ALsizei i;

    TRACE("%p\n", context);

    if((cprops=ATOMIC_LOAD(&context->Update, almemory_order_acquire)) != NULL)
    {
        TRACE("Freed unapplied context update %p\n", cprops);
        al_free(cprops);
    }

    count = 0;
    cprops = ATOMIC_LOAD(&context->FreeContextProps, almemory_order_acquire);
    while(cprops)
    {
        struct ALcontextProps *next = ATOMIC_LOAD(&cprops->next, almemory_order_acquire);
        al_free(cprops);
        cprops = next;
        ++count;
    }
    TRACE("Freed "SZFMT" context property object%s\n", count, (count==1)?"":"s");

    if(context->DefaultSlot)
    {
        DeinitEffectSlot(context->DefaultSlot);
        context->DefaultSlot = NULL;
    }

    auxslots = ATOMIC_EXCHANGE_PTR(&context->ActiveAuxSlots, NULL, almemory_order_relaxed);
    al_free(auxslots);

    ReleaseALSources(context);
#define FREE_SOURCESUBLIST(x) al_free((x)->Sources)
    VECTOR_FOR_EACH(SourceSubList, context->SourceList, FREE_SOURCESUBLIST);
#undef FREE_SOURCESUBLIST
    VECTOR_DEINIT(context->SourceList);
    context->NumSources = 0;
    almtx_destroy(&context->SourceLock);

    count = 0;
    eprops = ATOMIC_LOAD(&context->FreeEffectslotProps, almemory_order_relaxed);
    while(eprops)
    {
        struct ALeffectslotProps *next = ATOMIC_LOAD(&eprops->next, almemory_order_relaxed);
        if(eprops->State) ALeffectState_DecRef(eprops->State);
        al_free(eprops);
        eprops = next;
        ++count;
    }
    TRACE("Freed "SZFMT" AuxiliaryEffectSlot property object%s\n", count, (count==1)?"":"s");

    ReleaseALAuxiliaryEffectSlots(context);
#define FREE_EFFECTSLOTPTR(x) al_free(*(x))
    VECTOR_FOR_EACH(ALeffectslotPtr, context->EffectSlotList, FREE_EFFECTSLOTPTR);
#undef FREE_EFFECTSLOTPTR
    VECTOR_DEINIT(context->EffectSlotList);
    almtx_destroy(&context->EffectSlotLock);

    count = 0;
    vprops = ATOMIC_LOAD(&context->FreeVoiceProps, almemory_order_relaxed);
    while(vprops)
    {
        struct ALvoiceProps *next = ATOMIC_LOAD(&vprops->next, almemory_order_relaxed);
        al_free(vprops);
        vprops = next;
        ++count;
    }
    TRACE("Freed "SZFMT" voice property object%s\n", count, (count==1)?"":"s");

    for(i = 0;i < context->VoiceCount;i++)
        DeinitVoice(context->Voices[i]);
    al_free(context->Voices);
    context->Voices = NULL;
    context->VoiceCount = 0;
    context->MaxVoices = 0;

    if((lprops=ATOMIC_LOAD(&listener->Update, almemory_order_acquire)) != NULL)
    {
        TRACE("Freed unapplied listener update %p\n", lprops);
        al_free(lprops);
    }
    count = 0;
    lprops = ATOMIC_LOAD(&context->FreeListenerProps, almemory_order_acquire);
    while(lprops)
    {
        struct ALlistenerProps *next = ATOMIC_LOAD(&lprops->next, almemory_order_acquire);
        al_free(lprops);
        lprops = next;
        ++count;
    }
    TRACE("Freed "SZFMT" listener property object%s\n", count, (count==1)?"":"s");

    if(ATOMIC_EXCHANGE(&context->EnabledEvts, 0, almemory_order_acq_rel))
    {
        static const AsyncEvent kill_evt = { 0 };
        while(ll_ringbuffer_write(context->AsyncEvents, (const char*)&kill_evt, 1) == 0)
            althrd_yield();
        alsem_post(&context->EventSem);
        althrd_join(context->EventThread, NULL);
    }

    almtx_destroy(&context->EventCbLock);
    almtx_destroy(&context->EventThrdLock);
    alsem_destroy(&context->EventSem);

    ll_ringbuffer_free(context->AsyncEvents);
    context->AsyncEvents = NULL;

    almtx_destroy(&context->PropLock);

    ALCdevice_DecRef(context->Device);
    context->Device = NULL;

    //Invalidate context
    memset(context, 0, sizeof(ALCcontext));
    al_free(context);
}

/* ReleaseContext
 *
 * Removes the context reference from the given device and removes it from
 * being current on the running thread or globally. Returns true if other
 * contexts still exist on the device.
 */
static bool ReleaseContext(ALCcontext *context, ALCdevice *device)
{
    ALCcontext *origctx, *newhead;
    bool ret = true;

    if(altss_get(LocalContext) == context)
    {
        WARN("%p released while current on thread\n", context);
        altss_set(LocalContext, NULL);
        ALCcontext_DecRef(context);
    }

    origctx = context;
    if(ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&GlobalContext, &origctx, NULL))
        ALCcontext_DecRef(context);

    V0(device->Backend,lock)();
    origctx = context;
    newhead = ATOMIC_LOAD(&context->next, almemory_order_relaxed);
    if(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&device->ContextList, &origctx, newhead))
    {
        ALCcontext *list;
        do {
            /* origctx is what the desired context failed to match. Try
             * swapping out the next one in the list.
             */
            list = origctx;
            origctx = context;
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&list->next, &origctx, newhead));
    }
    else
        ret = !!newhead;
    V0(device->Backend,unlock)();

    ALCcontext_DecRef(context);
    return ret;
}

static void ALCcontext_IncRef(ALCcontext *context)
{
    uint ref = IncrementRef(&context->ref);
    TRACEREF("%p increasing refcount to %u\n", context, ref);
}

void ALCcontext_DecRef(ALCcontext *context)
{
    uint ref = DecrementRef(&context->ref);
    TRACEREF("%p decreasing refcount to %u\n", context, ref);
    if(ref == 0) FreeContext(context);
}

static void ReleaseThreadCtx(void *ptr)
{
    ALCcontext *context = ptr;
    uint ref = DecrementRef(&context->ref);
    TRACEREF("%p decreasing refcount to %u\n", context, ref);
    ERR("Context %p current for thread being destroyed, possible leak!\n", context);
}

/* VerifyContext
 *
 * Checks that the given context is valid, and increments its reference count.
 */
static ALCboolean VerifyContext(ALCcontext **context)
{
    ALCdevice *dev;

    LockLists();
    dev = ATOMIC_LOAD_SEQ(&DeviceList);
    while(dev)
    {
        ALCcontext *ctx = ATOMIC_LOAD(&dev->ContextList, almemory_order_acquire);
        while(ctx)
        {
            if(ctx == *context)
            {
                ALCcontext_IncRef(ctx);
                UnlockLists();
                return ALC_TRUE;
            }
            ctx = ATOMIC_LOAD(&ctx->next, almemory_order_relaxed);
        }
        dev = ATOMIC_LOAD(&dev->next, almemory_order_relaxed);
    }
    UnlockLists();

    *context = NULL;
    return ALC_FALSE;
}


/* GetContextRef
 *
 * Returns the currently active context for this thread, and adds a reference
 * without locking it.
 */
ALCcontext *GetContextRef(void)
{
    ALCcontext *context;

    context = altss_get(LocalContext);
    if(context)
        ALCcontext_IncRef(context);
    else
    {
        LockLists();
        context = ATOMIC_LOAD_SEQ(&GlobalContext);
        if(context)
            ALCcontext_IncRef(context);
        UnlockLists();
    }

    return context;
}


void AllocateVoices(ALCcontext *context, ALsizei num_voices, ALsizei old_sends)
{
    ALCdevice *device = context->Device;
    ALsizei num_sends = device->NumAuxSends;
    struct ALvoiceProps *props;
    size_t sizeof_props;
    size_t sizeof_voice;
    ALvoice **voices;
    ALvoice *voice;
    ALsizei v = 0;
    size_t size;

    if(num_voices == context->MaxVoices && num_sends == old_sends)
        return;

    /* Allocate the voice pointers, voices, and the voices' stored source
     * property set (including the dynamically-sized Send[] array) in one
     * chunk.
     */
    sizeof_voice = RoundUp(FAM_SIZE(ALvoice, Send, num_sends), 16);
    sizeof_props = RoundUp(FAM_SIZE(struct ALvoiceProps, Send, num_sends), 16);
    size = sizeof(ALvoice*) + sizeof_voice + sizeof_props;

    voices = al_calloc(16, RoundUp(size*num_voices, 16));
    /* The voice and property objects are stored interleaved since they're
     * paired together.
     */
    voice = (ALvoice*)((char*)voices + RoundUp(num_voices*sizeof(ALvoice*), 16));
    props = (struct ALvoiceProps*)((char*)voice + sizeof_voice);

    if(context->Voices)
    {
        const ALsizei v_count = mini(context->VoiceCount, num_voices);
        const ALsizei s_count = mini(old_sends, num_sends);

        for(;v < v_count;v++)
        {
            ALvoice *old_voice = context->Voices[v];
            ALsizei i;

            /* Copy the old voice data and source property set to the new
             * storage.
             */
            *voice = *old_voice;
            for(i = 0;i < s_count;i++)
                voice->Send[i] = old_voice->Send[i];
            *props = *(old_voice->Props);
            for(i = 0;i < s_count;i++)
                props->Send[i] = old_voice->Props->Send[i];

            /* Set this voice's property set pointer and voice reference. */
            voice->Props = props;
            voices[v] = voice;

            /* Increment pointers to the next storage space. */
            voice = (ALvoice*)((char*)props + sizeof_props);
            props = (struct ALvoiceProps*)((char*)voice + sizeof_voice);
        }
        /* Deinit any left over voices that weren't copied over to the new
         * array. NOTE: If this does anything, v equals num_voices and
         * num_voices is less than VoiceCount, so the following loop won't do
         * anything.
         */
        for(;v < context->VoiceCount;v++)
            DeinitVoice(context->Voices[v]);
    }
    /* Finish setting the voices' property set pointers and references. */
    for(;v < num_voices;v++)
    {
        ATOMIC_INIT(&voice->Update, NULL);

        voice->Props = props;
        voices[v] = voice;

        voice = (ALvoice*)((char*)props + sizeof_props);
        props = (struct ALvoiceProps*)((char*)voice + sizeof_voice);
    }

    al_free(context->Voices);
    context->Voices = voices;
    context->MaxVoices = num_voices;
    context->VoiceCount = mini(context->VoiceCount, num_voices);
}


/************************************************
 * Standard ALC functions
 ************************************************/

/* alcGetError
 *
 * Return last ALC generated error code for the given device
*/
ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice *device)
{
    ALCenum errorCode;

    if(VerifyDevice(&device))
    {
        errorCode = ATOMIC_EXCHANGE_SEQ(&device->LastError, ALC_NO_ERROR);
        ALCdevice_DecRef(device);
    }
    else
        errorCode = ATOMIC_EXCHANGE_SEQ(&LastNullDeviceError, ALC_NO_ERROR);

    return errorCode;
}


/* alcSuspendContext
 *
 * Suspends updates for the given context
 */
ALC_API ALCvoid ALC_APIENTRY alcSuspendContext(ALCcontext *context)
{
    if(!SuspendDefers)
        return;

    if(!VerifyContext(&context))
        alcSetError(NULL, ALC_INVALID_CONTEXT);
    else
    {
        ALCcontext_DeferUpdates(context);
        ALCcontext_DecRef(context);
    }
}

/* alcProcessContext
 *
 * Resumes processing updates for the given context
 */
ALC_API ALCvoid ALC_APIENTRY alcProcessContext(ALCcontext *context)
{
    if(!SuspendDefers)
        return;

    if(!VerifyContext(&context))
        alcSetError(NULL, ALC_INVALID_CONTEXT);
    else
    {
        ALCcontext_ProcessUpdates(context);
        ALCcontext_DecRef(context);
    }
}


/* alcGetString
 *
 * Returns information about the device, and error strings
 */
ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice *Device, ALCenum param)
{
    const ALCchar *value = NULL;

    switch(param)
    {
    case ALC_NO_ERROR:
        value = alcNoError;
        break;

    case ALC_INVALID_ENUM:
        value = alcErrInvalidEnum;
        break;

    case ALC_INVALID_VALUE:
        value = alcErrInvalidValue;
        break;

    case ALC_INVALID_DEVICE:
        value = alcErrInvalidDevice;
        break;

    case ALC_INVALID_CONTEXT:
        value = alcErrInvalidContext;
        break;

    case ALC_OUT_OF_MEMORY:
        value = alcErrOutOfMemory;
        break;

    case ALC_DEVICE_SPECIFIER:
        value = alcDefaultName;
        break;

    case ALC_ALL_DEVICES_SPECIFIER:
        if(VerifyDevice(&Device))
        {
            value = alstr_get_cstr(Device->DeviceName);
            ALCdevice_DecRef(Device);
        }
        else
        {
            ProbeAllDevicesList();
            value = alstr_get_cstr(alcAllDevicesList);
        }
        break;

    case ALC_CAPTURE_DEVICE_SPECIFIER:
        if(VerifyDevice(&Device))
        {
            value = alstr_get_cstr(Device->DeviceName);
            ALCdevice_DecRef(Device);
        }
        else
        {
            ProbeCaptureDeviceList();
            value = alstr_get_cstr(alcCaptureDeviceList);
        }
        break;

    /* Default devices are always first in the list */
    case ALC_DEFAULT_DEVICE_SPECIFIER:
        value = alcDefaultName;
        break;

    case ALC_DEFAULT_ALL_DEVICES_SPECIFIER:
        if(alstr_empty(alcAllDevicesList))
            ProbeAllDevicesList();

        VerifyDevice(&Device);

        free(alcDefaultAllDevicesSpecifier);
        alcDefaultAllDevicesSpecifier = strdup(alstr_get_cstr(alcAllDevicesList));
        if(!alcDefaultAllDevicesSpecifier)
            alcSetError(Device, ALC_OUT_OF_MEMORY);

        value = alcDefaultAllDevicesSpecifier;
        if(Device) ALCdevice_DecRef(Device);
        break;

    case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
        if(alstr_empty(alcCaptureDeviceList))
            ProbeCaptureDeviceList();

        VerifyDevice(&Device);

        free(alcCaptureDefaultDeviceSpecifier);
        alcCaptureDefaultDeviceSpecifier = strdup(alstr_get_cstr(alcCaptureDeviceList));
        if(!alcCaptureDefaultDeviceSpecifier)
            alcSetError(Device, ALC_OUT_OF_MEMORY);

        value = alcCaptureDefaultDeviceSpecifier;
        if(Device) ALCdevice_DecRef(Device);
        break;

    case ALC_EXTENSIONS:
        if(!VerifyDevice(&Device))
            value = alcNoDeviceExtList;
        else
        {
            value = alcExtensionList;
            ALCdevice_DecRef(Device);
        }
        break;

    case ALC_HRTF_SPECIFIER_SOFT:
        if(!VerifyDevice(&Device))
            alcSetError(NULL, ALC_INVALID_DEVICE);
        else
        {
            almtx_lock(&Device->BackendLock);
            value = (Device->HrtfHandle ? alstr_get_cstr(Device->HrtfName) : "");
            almtx_unlock(&Device->BackendLock);
            ALCdevice_DecRef(Device);
        }
        break;

    default:
        VerifyDevice(&Device);
        alcSetError(Device, ALC_INVALID_ENUM);
        if(Device) ALCdevice_DecRef(Device);
        break;
    }

    return value;
}


static inline ALCsizei NumAttrsForDevice(ALCdevice *device)
{
    if(device->Type == Capture) return 9;
    if(device->Type != Loopback) return 29;
    if(device->FmtChans == DevFmtAmbi3D)
        return 35;
    return 29;
}

static ALCsizei GetIntegerv(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values)
{
    ALCsizei i;

    if(size <= 0 || values == NULL)
    {
        alcSetError(device, ALC_INVALID_VALUE);
        return 0;
    }

    if(!device)
    {
        switch(param)
        {
            case ALC_MAJOR_VERSION:
                values[0] = alcMajorVersion;
                return 1;
            case ALC_MINOR_VERSION:
                values[0] = alcMinorVersion;
                return 1;

            case ALC_ATTRIBUTES_SIZE:
            case ALC_ALL_ATTRIBUTES:
            case ALC_FREQUENCY:
            case ALC_REFRESH:
            case ALC_SYNC:
            case ALC_MONO_SOURCES:
            case ALC_STEREO_SOURCES:
            case ALC_CAPTURE_SAMPLES:
            case ALC_FORMAT_CHANNELS_SOFT:
            case ALC_FORMAT_TYPE_SOFT:
            case ALC_AMBISONIC_LAYOUT_SOFT:
            case ALC_AMBISONIC_SCALING_SOFT:
            case ALC_AMBISONIC_ORDER_SOFT:
            case ALC_MAX_AMBISONIC_ORDER_SOFT:
                alcSetError(NULL, ALC_INVALID_DEVICE);
                return 0;

            default:
                alcSetError(NULL, ALC_INVALID_ENUM);
                return 0;
        }
        return 0;
    }

    if(device->Type == Capture)
    {
        switch(param)
        {
            case ALC_ATTRIBUTES_SIZE:
                values[0] = NumAttrsForDevice(device);
                return 1;

            case ALC_ALL_ATTRIBUTES:
                if(size < NumAttrsForDevice(device))
                {
                    alcSetError(device, ALC_INVALID_VALUE);
                    return 0;
                }

                i = 0;
                almtx_lock(&device->BackendLock);
                values[i++] = ALC_MAJOR_VERSION;
                values[i++] = alcMajorVersion;
                values[i++] = ALC_MINOR_VERSION;
                values[i++] = alcMinorVersion;
                values[i++] = ALC_CAPTURE_SAMPLES;
                values[i++] = V0(device->Backend,availableSamples)();
                values[i++] = ALC_CONNECTED;
                values[i++] = ATOMIC_LOAD(&device->Connected, almemory_order_relaxed);
                almtx_unlock(&device->BackendLock);

                values[i++] = 0;
                return i;

            case ALC_MAJOR_VERSION:
                values[0] = alcMajorVersion;
                return 1;
            case ALC_MINOR_VERSION:
                values[0] = alcMinorVersion;
                return 1;

            case ALC_CAPTURE_SAMPLES:
                almtx_lock(&device->BackendLock);
                values[0] = V0(device->Backend,availableSamples)();
                almtx_unlock(&device->BackendLock);
                return 1;

            case ALC_CONNECTED:
                values[0] = ATOMIC_LOAD(&device->Connected, almemory_order_acquire);
                return 1;

            default:
                alcSetError(device, ALC_INVALID_ENUM);
                return 0;
        }
        return 0;
    }

    /* render device */
    switch(param)
    {
        case ALC_ATTRIBUTES_SIZE:
            values[0] = NumAttrsForDevice(device);
            return 1;

        case ALC_ALL_ATTRIBUTES:
            if(size < NumAttrsForDevice(device))
            {
                alcSetError(device, ALC_INVALID_VALUE);
                return 0;
            }

            i = 0;
            almtx_lock(&device->BackendLock);
            values[i++] = ALC_MAJOR_VERSION;
            values[i++] = alcMajorVersion;
            values[i++] = ALC_MINOR_VERSION;
            values[i++] = alcMinorVersion;
            values[i++] = ALC_EFX_MAJOR_VERSION;
            values[i++] = alcEFXMajorVersion;
            values[i++] = ALC_EFX_MINOR_VERSION;
            values[i++] = alcEFXMinorVersion;

            values[i++] = ALC_FREQUENCY;
            values[i++] = device->Frequency;
            if(device->Type != Loopback)
            {
                values[i++] = ALC_REFRESH;
                values[i++] = device->Frequency / device->UpdateSize;

                values[i++] = ALC_SYNC;
                values[i++] = ALC_FALSE;
            }
            else
            {
                if(device->FmtChans == DevFmtAmbi3D)
                {
                    values[i++] = ALC_AMBISONIC_LAYOUT_SOFT;
                    values[i++] = device->AmbiLayout;

                    values[i++] = ALC_AMBISONIC_SCALING_SOFT;
                    values[i++] = device->AmbiScale;

                    values[i++] = ALC_AMBISONIC_ORDER_SOFT;
                    values[i++] = device->AmbiOrder;
                }

                values[i++] = ALC_FORMAT_CHANNELS_SOFT;
                values[i++] = device->FmtChans;

                values[i++] = ALC_FORMAT_TYPE_SOFT;
                values[i++] = device->FmtType;
            }

            values[i++] = ALC_MONO_SOURCES;
            values[i++] = device->NumMonoSources;

            values[i++] = ALC_STEREO_SOURCES;
            values[i++] = device->NumStereoSources;

            values[i++] = ALC_MAX_AUXILIARY_SENDS;
            values[i++] = device->NumAuxSends;

            values[i++] = ALC_HRTF_SOFT;
            values[i++] = (device->HrtfHandle ? ALC_TRUE : ALC_FALSE);

            values[i++] = ALC_HRTF_STATUS_SOFT;
            values[i++] = device->HrtfStatus;

            values[i++] = ALC_OUTPUT_LIMITER_SOFT;
            values[i++] = device->Limiter ? ALC_TRUE : ALC_FALSE;

            values[i++] = ALC_MAX_AMBISONIC_ORDER_SOFT;
            values[i++] = MAX_AMBI_ORDER;
            almtx_unlock(&device->BackendLock);

            values[i++] = 0;
            return i;

        case ALC_MAJOR_VERSION:
            values[0] = alcMajorVersion;
            return 1;

        case ALC_MINOR_VERSION:
            values[0] = alcMinorVersion;
            return 1;

        case ALC_EFX_MAJOR_VERSION:
            values[0] = alcEFXMajorVersion;
            return 1;

        case ALC_EFX_MINOR_VERSION:
            values[0] = alcEFXMinorVersion;
            return 1;

        case ALC_FREQUENCY:
            values[0] = device->Frequency;
            return 1;

        case ALC_REFRESH:
            if(device->Type == Loopback)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            almtx_lock(&device->BackendLock);
            values[0] = device->Frequency / device->UpdateSize;
            almtx_unlock(&device->BackendLock);
            return 1;

        case ALC_SYNC:
            if(device->Type == Loopback)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = ALC_FALSE;
            return 1;

        case ALC_FORMAT_CHANNELS_SOFT:
            if(device->Type != Loopback)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = device->FmtChans;
            return 1;

        case ALC_FORMAT_TYPE_SOFT:
            if(device->Type != Loopback)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = device->FmtType;
            return 1;

        case ALC_AMBISONIC_LAYOUT_SOFT:
            if(device->Type != Loopback || device->FmtChans != DevFmtAmbi3D)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = device->AmbiLayout;
            return 1;

        case ALC_AMBISONIC_SCALING_SOFT:
            if(device->Type != Loopback || device->FmtChans != DevFmtAmbi3D)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = device->AmbiScale;
            return 1;

        case ALC_AMBISONIC_ORDER_SOFT:
            if(device->Type != Loopback || device->FmtChans != DevFmtAmbi3D)
            {
                alcSetError(device, ALC_INVALID_DEVICE);
                return 0;
            }
            values[0] = device->AmbiOrder;
            return 1;

        case ALC_MONO_SOURCES:
            values[0] = device->NumMonoSources;
            return 1;

        case ALC_STEREO_SOURCES:
            values[0] = device->NumStereoSources;
            return 1;

        case ALC_MAX_AUXILIARY_SENDS:
            values[0] = device->NumAuxSends;
            return 1;

        case ALC_CONNECTED:
            values[0] = ATOMIC_LOAD(&device->Connected, almemory_order_acquire);
            return 1;

        case ALC_HRTF_SOFT:
            values[0] = (device->HrtfHandle ? ALC_TRUE : ALC_FALSE);
            return 1;

        case ALC_HRTF_STATUS_SOFT:
            values[0] = device->HrtfStatus;
            return 1;

        case ALC_NUM_HRTF_SPECIFIERS_SOFT:
            almtx_lock(&device->BackendLock);
            FreeHrtfList(&device->HrtfList);
            device->HrtfList = EnumerateHrtf(device->DeviceName);
            values[0] = (ALCint)VECTOR_SIZE(device->HrtfList);
            almtx_unlock(&device->BackendLock);
            return 1;

        case ALC_OUTPUT_LIMITER_SOFT:
            values[0] = device->Limiter ? ALC_TRUE : ALC_FALSE;
            return 1;

        case ALC_MAX_AMBISONIC_ORDER_SOFT:
            values[0] = MAX_AMBI_ORDER;
            return 1;

        default:
            alcSetError(device, ALC_INVALID_ENUM);
            return 0;
    }
    return 0;
}

/* alcGetIntegerv
 *
 * Returns information about the device and the version of OpenAL
 */
ALC_API void ALC_APIENTRY alcGetIntegerv(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values)
{
    VerifyDevice(&device);
    if(size <= 0 || values == NULL)
        alcSetError(device, ALC_INVALID_VALUE);
    else
        GetIntegerv(device, param, size, values);
    if(device) ALCdevice_DecRef(device);
}

ALC_API void ALC_APIENTRY alcGetInteger64vSOFT(ALCdevice *device, ALCenum pname, ALCsizei size, ALCint64SOFT *values)
{
    ALCint *ivals;
    ALsizei i;

    VerifyDevice(&device);
    if(size <= 0 || values == NULL)
        alcSetError(device, ALC_INVALID_VALUE);
    else if(!device || device->Type == Capture)
    {
        ivals = malloc(size * sizeof(ALCint));
        size = GetIntegerv(device, pname, size, ivals);
        for(i = 0;i < size;i++)
            values[i] = ivals[i];
        free(ivals);
    }
    else /* render device */
    {
        ClockLatency clock;
        ALuint64 basecount;
        ALuint samplecount;
        ALuint refcount;

        switch(pname)
        {
            case ALC_ATTRIBUTES_SIZE:
                *values = NumAttrsForDevice(device)+4;
                break;

            case ALC_ALL_ATTRIBUTES:
                if(size < NumAttrsForDevice(device)+4)
                    alcSetError(device, ALC_INVALID_VALUE);
                else
                {
                    i = 0;
                    almtx_lock(&device->BackendLock);
                    values[i++] = ALC_FREQUENCY;
                    values[i++] = device->Frequency;

                    if(device->Type != Loopback)
                    {
                        values[i++] = ALC_REFRESH;
                        values[i++] = device->Frequency / device->UpdateSize;

                        values[i++] = ALC_SYNC;
                        values[i++] = ALC_FALSE;
                    }
                    else
                    {
                        if(device->FmtChans == DevFmtAmbi3D)
                        {
                            values[i++] = ALC_AMBISONIC_LAYOUT_SOFT;
                            values[i++] = device->AmbiLayout;

                            values[i++] = ALC_AMBISONIC_SCALING_SOFT;
                            values[i++] = device->AmbiScale;

                            values[i++] = ALC_AMBISONIC_ORDER_SOFT;
                            values[i++] = device->AmbiOrder;
                        }

                        values[i++] = ALC_FORMAT_CHANNELS_SOFT;
                        values[i++] = device->FmtChans;

                        values[i++] = ALC_FORMAT_TYPE_SOFT;
                        values[i++] = device->FmtType;
                    }

                    values[i++] = ALC_MONO_SOURCES;
                    values[i++] = device->NumMonoSources;

                    values[i++] = ALC_STEREO_SOURCES;
                    values[i++] = device->NumStereoSources;

                    values[i++] = ALC_MAX_AUXILIARY_SENDS;
                    values[i++] = device->NumAuxSends;

                    values[i++] = ALC_HRTF_SOFT;
                    values[i++] = (device->HrtfHandle ? ALC_TRUE : ALC_FALSE);

                    values[i++] = ALC_HRTF_STATUS_SOFT;
                    values[i++] = device->HrtfStatus;

                    values[i++] = ALC_OUTPUT_LIMITER_SOFT;
                    values[i++] = device->Limiter ? ALC_TRUE : ALC_FALSE;

                    clock = V0(device->Backend,getClockLatency)();
                    values[i++] = ALC_DEVICE_CLOCK_SOFT;
                    values[i++] = clock.ClockTime;

                    values[i++] = ALC_DEVICE_LATENCY_SOFT;
                    values[i++] = clock.Latency;
                    almtx_unlock(&device->BackendLock);

                    values[i++] = 0;
                }
                break;

            case ALC_DEVICE_CLOCK_SOFT:
                almtx_lock(&device->BackendLock);
                do {
                    while(((refcount=ReadRef(&device->MixCount))&1) != 0)
                        althrd_yield();
                    basecount = device->ClockBase;
                    samplecount = device->SamplesDone;
                } while(refcount != ReadRef(&device->MixCount));
                *values = basecount + (samplecount*DEVICE_CLOCK_RES/device->Frequency);
                almtx_unlock(&device->BackendLock);
                break;

            case ALC_DEVICE_LATENCY_SOFT:
                almtx_lock(&device->BackendLock);
                clock = V0(device->Backend,getClockLatency)();
                almtx_unlock(&device->BackendLock);
                *values = clock.Latency;
                break;

            case ALC_DEVICE_CLOCK_LATENCY_SOFT:
                if(size < 2)
                    alcSetError(device, ALC_INVALID_VALUE);
                else
                {
                    almtx_lock(&device->BackendLock);
                    clock = V0(device->Backend,getClockLatency)();
                    almtx_unlock(&device->BackendLock);
                    values[0] = clock.ClockTime;
                    values[1] = clock.Latency;
                }
                break;

            default:
                ivals = malloc(size * sizeof(ALCint));
                size = GetIntegerv(device, pname, size, ivals);
                for(i = 0;i < size;i++)
                    values[i] = ivals[i];
                free(ivals);
                break;
        }
    }
    if(device)
        ALCdevice_DecRef(device);
}


/* alcIsExtensionPresent
 *
 * Determines if there is support for a particular extension
 */
ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice *device, const ALCchar *extName)
{
    ALCboolean bResult = ALC_FALSE;

    VerifyDevice(&device);

    if(!extName)
        alcSetError(device, ALC_INVALID_VALUE);
    else
    {
        size_t len = strlen(extName);
        const char *ptr = (device ? alcExtensionList : alcNoDeviceExtList);
        while(ptr && *ptr)
        {
            if(strncasecmp(ptr, extName, len) == 0 &&
               (ptr[len] == '\0' || isspace(ptr[len])))
            {
                bResult = ALC_TRUE;
                break;
            }
            if((ptr=strchr(ptr, ' ')) != NULL)
            {
                do {
                    ++ptr;
                } while(isspace(*ptr));
            }
        }
    }
    if(device)
        ALCdevice_DecRef(device);
    return bResult;
}


/* alcGetProcAddress
 *
 * Retrieves the function address for a particular extension function
 */
ALC_API ALCvoid* ALC_APIENTRY alcGetProcAddress(ALCdevice *device, const ALCchar *funcName)
{
    ALCvoid *ptr = NULL;

    if(!funcName)
    {
        VerifyDevice(&device);
        alcSetError(device, ALC_INVALID_VALUE);
        if(device) ALCdevice_DecRef(device);
    }
    else
    {
        size_t i = 0;
        for(i = 0;i < COUNTOF(alcFunctions);i++)
        {
            if(strcmp(alcFunctions[i].funcName, funcName) == 0)
            {
                ptr = alcFunctions[i].address;
                break;
            }
        }
    }

    return ptr;
}


/* alcGetEnumValue
 *
 * Get the value for a particular ALC enumeration name
 */
ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice *device, const ALCchar *enumName)
{
    ALCenum val = 0;

    if(!enumName)
    {
        VerifyDevice(&device);
        alcSetError(device, ALC_INVALID_VALUE);
        if(device) ALCdevice_DecRef(device);
    }
    else
    {
        size_t i = 0;
        for(i = 0;i < COUNTOF(alcEnumerations);i++)
        {
            if(strcmp(alcEnumerations[i].enumName, enumName) == 0)
            {
                val = alcEnumerations[i].value;
                break;
            }
        }
    }

    return val;
}


/* alcCreateContext
 *
 * Create and attach a context to the given device.
 */
ALC_API ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice *device, const ALCint *attrList)
{
    ALCcontext *ALContext;
    ALfloat valf;
    ALCenum err;

    /* Explicitly hold the list lock while taking the BackendLock in case the
     * device is asynchronously destropyed, to ensure this new context is
     * properly cleaned up after being made.
     */
    LockLists();
    if(!VerifyDevice(&device) || device->Type == Capture ||
       !ATOMIC_LOAD(&device->Connected, almemory_order_relaxed))
    {
        UnlockLists();
        alcSetError(device, ALC_INVALID_DEVICE);
        if(device) ALCdevice_DecRef(device);
        return NULL;
    }
    almtx_lock(&device->BackendLock);
    UnlockLists();

    ATOMIC_STORE_SEQ(&device->LastError, ALC_NO_ERROR);

    if(device->Type == Playback && DefaultEffect.type != AL_EFFECT_NULL)
        ALContext = al_calloc(16, sizeof(ALCcontext)+sizeof(ALlistener)+sizeof(ALeffectslot));
    else
        ALContext = al_calloc(16, sizeof(ALCcontext)+sizeof(ALlistener));
    if(!ALContext)
    {
        almtx_unlock(&device->BackendLock);

        alcSetError(device, ALC_OUT_OF_MEMORY);
        ALCdevice_DecRef(device);
        return NULL;
    }

    InitRef(&ALContext->ref, 1);
    ALContext->Listener = (ALlistener*)ALContext->_listener_mem;
    ALContext->DefaultSlot = NULL;

    ALContext->Voices = NULL;
    ALContext->VoiceCount = 0;
    ALContext->MaxVoices = 0;
    ATOMIC_INIT(&ALContext->ActiveAuxSlots, NULL);
    ALContext->Device = device;
    ATOMIC_INIT(&ALContext->next, NULL);

    if((err=UpdateDeviceParams(device, attrList)) != ALC_NO_ERROR)
    {
        almtx_unlock(&device->BackendLock);

        al_free(ALContext);
        ALContext = NULL;

        alcSetError(device, err);
        if(err == ALC_INVALID_DEVICE)
        {
            V0(device->Backend,lock)();
            aluHandleDisconnect(device, "Device update failure");
            V0(device->Backend,unlock)();
        }
        ALCdevice_DecRef(device);
        return NULL;
    }
    AllocateVoices(ALContext, 256, device->NumAuxSends);

    if(DefaultEffect.type != AL_EFFECT_NULL && device->Type == Playback)
    {
        ALContext->DefaultSlot = (ALeffectslot*)(ALContext->_listener_mem + sizeof(ALlistener));
        if(InitEffectSlot(ALContext->DefaultSlot) == AL_NO_ERROR)
            aluInitEffectPanning(ALContext->DefaultSlot);
        else
        {
            ALContext->DefaultSlot = NULL;
            ERR("Failed to initialize the default effect slot\n");
        }
    }

    ALCdevice_IncRef(ALContext->Device);
    InitContext(ALContext);

    if(ConfigValueFloat(alstr_get_cstr(device->DeviceName), NULL, "volume-adjust", &valf))
    {
        if(!isfinite(valf))
            ERR("volume-adjust must be finite: %f\n", valf);
        else
        {
            ALfloat db = clampf(valf, -24.0f, 24.0f);
            if(db != valf)
                WARN("volume-adjust clamped: %f, range: +/-%f\n", valf, 24.0f);
            ALContext->GainBoost = powf(10.0f, db/20.0f);
            TRACE("volume-adjust gain: %f\n", ALContext->GainBoost);
        }
    }
    UpdateListenerProps(ALContext);

    {
        ALCcontext *head = ATOMIC_LOAD_SEQ(&device->ContextList);
        do {
            ATOMIC_STORE(&ALContext->next, head, almemory_order_relaxed);
        } while(ATOMIC_COMPARE_EXCHANGE_PTR_WEAK_SEQ(&device->ContextList, &head,
                                                     ALContext) == 0);
    }
    almtx_unlock(&device->BackendLock);

    if(ALContext->DefaultSlot)
    {
        if(InitializeEffect(ALContext, ALContext->DefaultSlot, &DefaultEffect) == AL_NO_ERROR)
            UpdateEffectSlotProps(ALContext->DefaultSlot, ALContext);
        else
            ERR("Failed to initialize the default effect\n");
    }

    ALCdevice_DecRef(device);

    TRACE("Created context %p\n", ALContext);
    return ALContext;
}

/* alcDestroyContext
 *
 * Remove a context from its device
 */
ALC_API ALCvoid ALC_APIENTRY alcDestroyContext(ALCcontext *context)
{
    ALCdevice *Device;

    LockLists();
    if(!VerifyContext(&context))
    {
        UnlockLists();
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        return;
    }

    Device = context->Device;
    if(Device)
    {
        almtx_lock(&Device->BackendLock);
        if(!ReleaseContext(context, Device))
        {
            V0(Device->Backend,stop)();
            Device->Flags &= ~DEVICE_RUNNING;
        }
        almtx_unlock(&Device->BackendLock);
    }
    UnlockLists();

    ALCcontext_DecRef(context);
}


/* alcGetCurrentContext
 *
 * Returns the currently active context on the calling thread
 */
ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext(void)
{
    ALCcontext *Context = altss_get(LocalContext);
    if(!Context) Context = ATOMIC_LOAD_SEQ(&GlobalContext);
    return Context;
}

/* alcGetThreadContext
 *
 * Returns the currently active thread-local context
 */
ALC_API ALCcontext* ALC_APIENTRY alcGetThreadContext(void)
{
    return altss_get(LocalContext);
}


/* alcMakeContextCurrent
 *
 * Makes the given context the active process-wide context, and removes the
 * thread-local context for the calling thread.
 */
ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext *context)
{
    /* context must be valid or NULL */
    if(context && !VerifyContext(&context))
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        return ALC_FALSE;
    }
    /* context's reference count is already incremented */
    context = ATOMIC_EXCHANGE_PTR_SEQ(&GlobalContext, context);
    if(context) ALCcontext_DecRef(context);

    if((context=altss_get(LocalContext)) != NULL)
    {
        altss_set(LocalContext, NULL);
        ALCcontext_DecRef(context);
    }

    return ALC_TRUE;
}

/* alcSetThreadContext
 *
 * Makes the given context the active context for the current thread
 */
ALC_API ALCboolean ALC_APIENTRY alcSetThreadContext(ALCcontext *context)
{
    ALCcontext *old;

    /* context must be valid or NULL */
    if(context && !VerifyContext(&context))
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        return ALC_FALSE;
    }
    /* context's reference count is already incremented */
    old = altss_get(LocalContext);
    altss_set(LocalContext, context);
    if(old) ALCcontext_DecRef(old);

    return ALC_TRUE;
}


/* alcGetContextsDevice
 *
 * Returns the device that a particular context is attached to
 */
ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext *Context)
{
    ALCdevice *Device;

    if(!VerifyContext(&Context))
    {
        alcSetError(NULL, ALC_INVALID_CONTEXT);
        return NULL;
    }
    Device = Context->Device;
    ALCcontext_DecRef(Context);

    return Device;
}


/* alcOpenDevice
 *
 * Opens the named device.
 */
ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar *deviceName)
{
    ALCbackendFactory *factory;
    const ALCchar *fmt;
    ALCdevice *device;
    ALCenum err;

    DO_INITCONFIG();

    if(!PlaybackBackend.name)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        return NULL;
    }

    if(deviceName && (!deviceName[0] || strcasecmp(deviceName, alcDefaultName) == 0 || strcasecmp(deviceName, "openal-soft") == 0
#ifdef _WIN32
        /* Some old Windows apps hardcode these expecting OpenAL to use a
         * specific audio API, even when they're not enumerated. Creative's
         * router effectively ignores them too.
         */
        || strcasecmp(deviceName, "DirectSound3D") == 0 || strcasecmp(deviceName, "DirectSound") == 0
        || strcasecmp(deviceName, "MMSYSTEM") == 0
#endif
    ))
        deviceName = NULL;

    device = al_calloc(16, sizeof(ALCdevice));
    if(!device)
    {
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    //Validate device
    InitDevice(device, Playback);

    //Set output format
    device->FmtChans = DevFmtChannelsDefault;
    device->FmtType = DevFmtTypeDefault;
    device->Frequency = DEFAULT_OUTPUT_RATE;
    device->IsHeadphones = AL_FALSE;
    device->AmbiLayout = AmbiLayout_Default;
    device->AmbiScale = AmbiNorm_Default;
    device->NumUpdates = 3;
    device->UpdateSize = 1024;

    device->SourcesMax = 256;
    device->AuxiliaryEffectSlotMax = 64;
    device->NumAuxSends = DEFAULT_SENDS;

    if(ConfigValueStr(deviceName, NULL, "channels", &fmt))
    {
        static const struct {
            const char name[16];
            enum DevFmtChannels chans;
            ALsizei order;
        } chanlist[] = {
            { "mono",       DevFmtMono,   0 },
            { "stereo",     DevFmtStereo, 0 },
            { "quad",       DevFmtQuad,   0 },
            { "surround51", DevFmtX51,    0 },
            { "surround61", DevFmtX61,    0 },
            { "surround71", DevFmtX71,    0 },
            { "surround51rear", DevFmtX51Rear, 0 },
            { "ambi1", DevFmtAmbi3D, 1 },
            { "ambi2", DevFmtAmbi3D, 2 },
            { "ambi3", DevFmtAmbi3D, 3 },
        };
        size_t i;

        for(i = 0;i < COUNTOF(chanlist);i++)
        {
            if(strcasecmp(chanlist[i].name, fmt) == 0)
            {
                device->FmtChans = chanlist[i].chans;
                device->AmbiOrder = chanlist[i].order;
                device->Flags |= DEVICE_CHANNELS_REQUEST;
                break;
            }
        }
        if(i == COUNTOF(chanlist))
            ERR("Unsupported channels: %s\n", fmt);
    }
    if(ConfigValueStr(deviceName, NULL, "sample-type", &fmt))
    {
        static const struct {
            const char name[16];
            enum DevFmtType type;
        } typelist[] = {
            { "int8",    DevFmtByte   },
            { "uint8",   DevFmtUByte  },
            { "int16",   DevFmtShort  },
            { "uint16",  DevFmtUShort },
            { "int32",   DevFmtInt    },
            { "uint32",  DevFmtUInt   },
            { "float32", DevFmtFloat  },
        };
        size_t i;

        for(i = 0;i < COUNTOF(typelist);i++)
        {
            if(strcasecmp(typelist[i].name, fmt) == 0)
            {
                device->FmtType = typelist[i].type;
                device->Flags |= DEVICE_SAMPLE_TYPE_REQUEST;
                break;
            }
        }
        if(i == COUNTOF(typelist))
            ERR("Unsupported sample-type: %s\n", fmt);
    }

    if(ConfigValueUInt(deviceName, NULL, "frequency", &device->Frequency))
    {
        device->Flags |= DEVICE_FREQUENCY_REQUEST;
        if(device->Frequency < MIN_OUTPUT_RATE)
            ERR("%uhz request clamped to %uhz minimum\n", device->Frequency, MIN_OUTPUT_RATE);
        device->Frequency = maxu(device->Frequency, MIN_OUTPUT_RATE);
    }

    ConfigValueUInt(deviceName, NULL, "periods", &device->NumUpdates);
    device->NumUpdates = clampu(device->NumUpdates, 2, 16);

    ConfigValueUInt(deviceName, NULL, "period_size", &device->UpdateSize);
    device->UpdateSize = clampu(device->UpdateSize, 64, 8192);
    if((CPUCapFlags&(CPU_CAP_SSE|CPU_CAP_NEON)) != 0)
        device->UpdateSize = (device->UpdateSize+3)&~3;

    ConfigValueUInt(deviceName, NULL, "sources", &device->SourcesMax);
    if(device->SourcesMax == 0) device->SourcesMax = 256;

    ConfigValueUInt(deviceName, NULL, "slots", &device->AuxiliaryEffectSlotMax);
    if(device->AuxiliaryEffectSlotMax == 0) device->AuxiliaryEffectSlotMax = 64;
    else device->AuxiliaryEffectSlotMax = minu(device->AuxiliaryEffectSlotMax, INT_MAX);

    if(ConfigValueInt(deviceName, NULL, "sends", &device->NumAuxSends))
        device->NumAuxSends = clampi(
            DEFAULT_SENDS, 0, clampi(device->NumAuxSends, 0, MAX_SENDS)
        );

    device->NumStereoSources = 1;
    device->NumMonoSources = device->SourcesMax - device->NumStereoSources;

    factory = PlaybackBackend.getFactory();
    device->Backend = V(factory,createBackend)(device, ALCbackend_Playback);
    if(!device->Backend)
    {
        FreeDevice(device);
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    // Find a playback device to open
    if((err=V(device->Backend,open)(deviceName)) != ALC_NO_ERROR)
    {
        FreeDevice(device);
        alcSetError(NULL, err);
        return NULL;
    }

    if(ConfigValueStr(alstr_get_cstr(device->DeviceName), NULL, "ambi-format", &fmt))
    {
        if(strcasecmp(fmt, "fuma") == 0)
        {
            device->AmbiLayout = AmbiLayout_FuMa;
            device->AmbiScale = AmbiNorm_FuMa;
        }
        else if(strcasecmp(fmt, "acn+sn3d") == 0)
        {
            device->AmbiLayout = AmbiLayout_ACN;
            device->AmbiScale = AmbiNorm_SN3D;
        }
        else if(strcasecmp(fmt, "acn+n3d") == 0)
        {
            device->AmbiLayout = AmbiLayout_ACN;
            device->AmbiScale = AmbiNorm_N3D;
        }
        else
            ERR("Unsupported ambi-format: %s\n", fmt);
    }

    device->Limiter = CreateDeviceLimiter(device);

    {
        ALCdevice *head = ATOMIC_LOAD_SEQ(&DeviceList);
        do {
            ATOMIC_STORE(&device->next, head, almemory_order_relaxed);
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_WEAK_SEQ(&DeviceList, &head, device));
    }

    TRACE("Created device %p, \"%s\"\n", device, alstr_get_cstr(device->DeviceName));
    return device;
}

/* alcCloseDevice
 *
 * Closes the given device.
 */
ALC_API ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice *device)
{
    ALCdevice *iter, *origdev, *nextdev;
    ALCcontext *ctx;

    LockLists();
    iter = ATOMIC_LOAD_SEQ(&DeviceList);
    do {
        if(iter == device)
            break;
        iter = ATOMIC_LOAD(&iter->next, almemory_order_relaxed);
    } while(iter != NULL);
    if(!iter || iter->Type == Capture)
    {
        alcSetError(iter, ALC_INVALID_DEVICE);
        UnlockLists();
        return ALC_FALSE;
    }
    almtx_lock(&device->BackendLock);

    origdev = device;
    nextdev = ATOMIC_LOAD(&device->next, almemory_order_relaxed);
    if(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&DeviceList, &origdev, nextdev))
    {
        ALCdevice *list;
        do {
            list = origdev;
            origdev = device;
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&list->next, &origdev, nextdev));
    }
    UnlockLists();

    ctx = ATOMIC_LOAD_SEQ(&device->ContextList);
    while(ctx != NULL)
    {
        ALCcontext *next = ATOMIC_LOAD(&ctx->next, almemory_order_relaxed);
        WARN("Releasing context %p\n", ctx);
        ReleaseContext(ctx, device);
        ctx = next;
    }
    if((device->Flags&DEVICE_RUNNING))
        V0(device->Backend,stop)();
    device->Flags &= ~DEVICE_RUNNING;
    almtx_unlock(&device->BackendLock);

    ALCdevice_DecRef(device);

    return ALC_TRUE;
}


/************************************************
 * ALC capture functions
 ************************************************/
ALC_API ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(const ALCchar *deviceName, ALCuint frequency, ALCenum format, ALCsizei samples)
{
    ALCbackendFactory *factory;
    ALCdevice *device = NULL;
    ALCenum err;

    DO_INITCONFIG();

    if(!CaptureBackend.name)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        return NULL;
    }

    if(samples <= 0)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        return NULL;
    }

    if(deviceName && (!deviceName[0] || strcasecmp(deviceName, alcDefaultName) == 0 || strcasecmp(deviceName, "openal-soft") == 0))
        deviceName = NULL;

    device = al_calloc(16, sizeof(ALCdevice));
    if(!device)
    {
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    //Validate device
    InitDevice(device, Capture);

    device->Frequency = frequency;
    device->Flags |= DEVICE_FREQUENCY_REQUEST;

    if(DecomposeDevFormat(format, &device->FmtChans, &device->FmtType) == AL_FALSE)
    {
        FreeDevice(device);
        alcSetError(NULL, ALC_INVALID_ENUM);
        return NULL;
    }
    device->Flags |= DEVICE_CHANNELS_REQUEST | DEVICE_SAMPLE_TYPE_REQUEST;
    device->IsHeadphones = AL_FALSE;
    device->AmbiOrder = 0;
    device->AmbiLayout = AmbiLayout_Default;
    device->AmbiScale = AmbiNorm_Default;

    device->UpdateSize = samples;
    device->NumUpdates = 1;

    factory = CaptureBackend.getFactory();
    device->Backend = V(factory,createBackend)(device, ALCbackend_Capture);
    if(!device->Backend)
    {
        FreeDevice(device);
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    TRACE("Capture format: %s, %s, %uhz, %u update size x%d\n",
        DevFmtChannelsString(device->FmtChans), DevFmtTypeString(device->FmtType),
        device->Frequency, device->UpdateSize, device->NumUpdates
    );
    if((err=V(device->Backend,open)(deviceName)) != ALC_NO_ERROR)
    {
        FreeDevice(device);
        alcSetError(NULL, err);
        return NULL;
    }

    {
        ALCdevice *head = ATOMIC_LOAD_SEQ(&DeviceList);
        do {
            ATOMIC_STORE(&device->next, head, almemory_order_relaxed);
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_WEAK_SEQ(&DeviceList, &head, device));
    }

    TRACE("Created device %p, \"%s\"\n", device, alstr_get_cstr(device->DeviceName));
    return device;
}

ALC_API ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice *device)
{
    ALCdevice *iter, *origdev, *nextdev;

    LockLists();
    iter = ATOMIC_LOAD_SEQ(&DeviceList);
    do {
        if(iter == device)
            break;
        iter = ATOMIC_LOAD(&iter->next, almemory_order_relaxed);
    } while(iter != NULL);
    if(!iter || iter->Type != Capture)
    {
        alcSetError(iter, ALC_INVALID_DEVICE);
        UnlockLists();
        return ALC_FALSE;
    }

    origdev = device;
    nextdev = ATOMIC_LOAD(&device->next, almemory_order_relaxed);
    if(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&DeviceList, &origdev, nextdev))
    {
        ALCdevice *list;
        do {
            list = origdev;
            origdev = device;
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(&list->next, &origdev, nextdev));
    }
    UnlockLists();

    ALCdevice_DecRef(device);

    return ALC_TRUE;
}

ALC_API void ALC_APIENTRY alcCaptureStart(ALCdevice *device)
{
    if(!VerifyDevice(&device) || device->Type != Capture)
        alcSetError(device, ALC_INVALID_DEVICE);
    else
    {
        almtx_lock(&device->BackendLock);
        if(!ATOMIC_LOAD(&device->Connected, almemory_order_acquire))
            alcSetError(device, ALC_INVALID_DEVICE);
        else if(!(device->Flags&DEVICE_RUNNING))
        {
            if(V0(device->Backend,start)())
                device->Flags |= DEVICE_RUNNING;
            else
            {
                aluHandleDisconnect(device, "Device start failure");
                alcSetError(device, ALC_INVALID_DEVICE);
            }
        }
        almtx_unlock(&device->BackendLock);
    }

    if(device) ALCdevice_DecRef(device);
}

ALC_API void ALC_APIENTRY alcCaptureStop(ALCdevice *device)
{
    if(!VerifyDevice(&device) || device->Type != Capture)
        alcSetError(device, ALC_INVALID_DEVICE);
    else
    {
        almtx_lock(&device->BackendLock);
        if((device->Flags&DEVICE_RUNNING))
            V0(device->Backend,stop)();
        device->Flags &= ~DEVICE_RUNNING;
        almtx_unlock(&device->BackendLock);
    }

    if(device) ALCdevice_DecRef(device);
}

ALC_API void ALC_APIENTRY alcCaptureSamples(ALCdevice *device, ALCvoid *buffer, ALCsizei samples)
{
    if(!VerifyDevice(&device) || device->Type != Capture)
        alcSetError(device, ALC_INVALID_DEVICE);
    else
    {
        ALCenum err = ALC_INVALID_VALUE;

        almtx_lock(&device->BackendLock);
        if(samples >= 0 && V0(device->Backend,availableSamples)() >= (ALCuint)samples)
            err = V(device->Backend,captureSamples)(buffer, samples);
        almtx_unlock(&device->BackendLock);

        if(err != ALC_NO_ERROR)
            alcSetError(device, err);
    }
    if(device) ALCdevice_DecRef(device);
}


/************************************************
 * ALC loopback functions
 ************************************************/

/* alcLoopbackOpenDeviceSOFT
 *
 * Open a loopback device, for manual rendering.
 */
ALC_API ALCdevice* ALC_APIENTRY alcLoopbackOpenDeviceSOFT(const ALCchar *deviceName)
{
    ALCbackendFactory *factory;
    ALCdevice *device;

    DO_INITCONFIG();

    /* Make sure the device name, if specified, is us. */
    if(deviceName && strcmp(deviceName, alcDefaultName) != 0)
    {
        alcSetError(NULL, ALC_INVALID_VALUE);
        return NULL;
    }

    device = al_calloc(16, sizeof(ALCdevice));
    if(!device)
    {
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    //Validate device
    InitDevice(device, Loopback);

    device->SourcesMax = 256;
    device->AuxiliaryEffectSlotMax = 64;
    device->NumAuxSends = DEFAULT_SENDS;

    //Set output format
    device->NumUpdates = 0;
    device->UpdateSize = 0;

    device->Frequency = DEFAULT_OUTPUT_RATE;
    device->FmtChans = DevFmtChannelsDefault;
    device->FmtType = DevFmtTypeDefault;
    device->IsHeadphones = AL_FALSE;
    device->AmbiLayout = AmbiLayout_Default;
    device->AmbiScale = AmbiNorm_Default;

    ConfigValueUInt(NULL, NULL, "sources", &device->SourcesMax);
    if(device->SourcesMax == 0) device->SourcesMax = 256;

    ConfigValueUInt(NULL, NULL, "slots", &device->AuxiliaryEffectSlotMax);
    if(device->AuxiliaryEffectSlotMax == 0) device->AuxiliaryEffectSlotMax = 64;
    else device->AuxiliaryEffectSlotMax = minu(device->AuxiliaryEffectSlotMax, INT_MAX);

    if(ConfigValueInt(NULL, NULL, "sends", &device->NumAuxSends))
        device->NumAuxSends = clampi(
            DEFAULT_SENDS, 0, clampi(device->NumAuxSends, 0, MAX_SENDS)
        );

    device->NumStereoSources = 1;
    device->NumMonoSources = device->SourcesMax - device->NumStereoSources;

    factory = ALCloopbackFactory_getFactory();
    device->Backend = V(factory,createBackend)(device, ALCbackend_Loopback);
    if(!device->Backend)
    {
        al_free(device);
        alcSetError(NULL, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    // Open the "backend"
    V(device->Backend,open)("Loopback");

    device->Limiter = CreateDeviceLimiter(device);

    {
        ALCdevice *head = ATOMIC_LOAD_SEQ(&DeviceList);
        do {
            ATOMIC_STORE(&device->next, head, almemory_order_relaxed);
        } while(!ATOMIC_COMPARE_EXCHANGE_PTR_WEAK_SEQ(&DeviceList, &head, device));
    }

    TRACE("Created device %p\n", device);
    return device;
}

/* alcIsRenderFormatSupportedSOFT
 *
 * Determines if the loopback device supports the given format for rendering.
 */
ALC_API ALCboolean ALC_APIENTRY alcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type)
{
    ALCboolean ret = ALC_FALSE;

    if(!VerifyDevice(&device) || device->Type != Loopback)
        alcSetError(device, ALC_INVALID_DEVICE);
    else if(freq <= 0)
        alcSetError(device, ALC_INVALID_VALUE);
    else
    {
        if(IsValidALCType(type) && IsValidALCChannels(channels) && freq >= MIN_OUTPUT_RATE)
            ret = ALC_TRUE;
    }
    if(device) ALCdevice_DecRef(device);

    return ret;
}

/* alcRenderSamplesSOFT
 *
 * Renders some samples into a buffer, using the format last set by the
 * attributes given to alcCreateContext.
 */
FORCE_ALIGN ALC_API void ALC_APIENTRY alcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples)
{
    if(!VerifyDevice(&device) || device->Type != Loopback)
        alcSetError(device, ALC_INVALID_DEVICE);
    else if(samples < 0 || (samples > 0 && buffer == NULL))
        alcSetError(device, ALC_INVALID_VALUE);
    else
    {
        V0(device->Backend,lock)();
        aluMixData(device, buffer, samples);
        V0(device->Backend,unlock)();
    }
    if(device) ALCdevice_DecRef(device);
}


/************************************************
 * ALC DSP pause/resume functions
 ************************************************/

/* alcDevicePauseSOFT
 *
 * Pause the DSP to stop audio processing.
 */
ALC_API void ALC_APIENTRY alcDevicePauseSOFT(ALCdevice *device)
{
    if(!VerifyDevice(&device) || device->Type != Playback)
        alcSetError(device, ALC_INVALID_DEVICE);
    else
    {
        almtx_lock(&device->BackendLock);
        if((device->Flags&DEVICE_RUNNING))
            V0(device->Backend,stop)();
        device->Flags &= ~DEVICE_RUNNING;
        device->Flags |= DEVICE_PAUSED;
        almtx_unlock(&device->BackendLock);
    }
    if(device) ALCdevice_DecRef(device);
}

/* alcDeviceResumeSOFT
 *
 * Resume the DSP to restart audio processing.
 */
ALC_API void ALC_APIENTRY alcDeviceResumeSOFT(ALCdevice *device)
{
    if(!VerifyDevice(&device) || device->Type != Playback)
        alcSetError(device, ALC_INVALID_DEVICE);
    else
    {
        almtx_lock(&device->BackendLock);
        if((device->Flags&DEVICE_PAUSED))
        {
            device->Flags &= ~DEVICE_PAUSED;
            if(ATOMIC_LOAD_SEQ(&device->ContextList) != NULL)
            {
                if(V0(device->Backend,start)() != ALC_FALSE)
                    device->Flags |= DEVICE_RUNNING;
                else
                {
                    V0(device->Backend,lock)();
                    aluHandleDisconnect(device, "Device start failure");
                    V0(device->Backend,unlock)();
                    alcSetError(device, ALC_INVALID_DEVICE);
                }
            }
        }
        almtx_unlock(&device->BackendLock);
    }
    if(device) ALCdevice_DecRef(device);
}


/************************************************
 * ALC HRTF functions
 ************************************************/

/* alcGetStringiSOFT
 *
 * Gets a string parameter at the given index.
 */
ALC_API const ALCchar* ALC_APIENTRY alcGetStringiSOFT(ALCdevice *device, ALCenum paramName, ALCsizei index)
{
    const ALCchar *str = NULL;

    if(!VerifyDevice(&device) || device->Type == Capture)
        alcSetError(device, ALC_INVALID_DEVICE);
    else switch(paramName)
    {
        case ALC_HRTF_SPECIFIER_SOFT:
            if(index >= 0 && (size_t)index < VECTOR_SIZE(device->HrtfList))
                str = alstr_get_cstr(VECTOR_ELEM(device->HrtfList, index).name);
            else
                alcSetError(device, ALC_INVALID_VALUE);
            break;

        default:
            alcSetError(device, ALC_INVALID_ENUM);
            break;
    }
    if(device) ALCdevice_DecRef(device);

    return str;
}

/* alcResetDeviceSOFT
 *
 * Resets the given device output, using the specified attribute list.
 */
ALC_API ALCboolean ALC_APIENTRY alcResetDeviceSOFT(ALCdevice *device, const ALCint *attribs)
{
    ALCenum err;

    LockLists();
    if(!VerifyDevice(&device) || device->Type == Capture ||
       !ATOMIC_LOAD(&device->Connected, almemory_order_relaxed))
    {
        UnlockLists();
        alcSetError(device, ALC_INVALID_DEVICE);
        if(device) ALCdevice_DecRef(device);
        return ALC_FALSE;
    }
    almtx_lock(&device->BackendLock);
    UnlockLists();

    err = UpdateDeviceParams(device, attribs);
    almtx_unlock(&device->BackendLock);

    if(err != ALC_NO_ERROR)
    {
        alcSetError(device, err);
        if(err == ALC_INVALID_DEVICE)
        {
            V0(device->Backend,lock)();
            aluHandleDisconnect(device, "Device start failure");
            V0(device->Backend,unlock)();
        }
        ALCdevice_DecRef(device);
        return ALC_FALSE;
    }
    ALCdevice_DecRef(device);

    return ALC_TRUE;
}
