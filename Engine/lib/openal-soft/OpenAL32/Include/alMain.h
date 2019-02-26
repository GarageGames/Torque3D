#ifndef AL_MAIN_H
#define AL_MAIN_H

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_INTRIN_H
#include <intrin.h>
#endif

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "inprogext.h"
#include "logging.h"
#include "polymorphism.h"
#include "static_assert.h"
#include "align.h"
#include "atomic.h"
#include "vector.h"
#include "alstring.h"
#include "almalloc.h"
#include "threads.h"


#if defined(_WIN64)
#define SZFMT "%I64u"
#elif defined(_WIN32)
#define SZFMT "%u"
#else
#define SZFMT "%zu"
#endif

#ifdef __has_builtin
#define HAS_BUILTIN __has_builtin
#else
#define HAS_BUILTIN(x) (0)
#endif

#ifdef __GNUC__
/* LIKELY optimizes the case where the condition is true. The condition is not
 * required to be true, but it can result in more optimal code for the true
 * path at the expense of a less optimal false path.
 */
#define LIKELY(x) __builtin_expect(!!(x), !0)
/* The opposite of LIKELY, optimizing the case where the condition is false. */
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
/* Unlike LIKELY, ASSUME requires the condition to be true or else it invokes
 * undefined behavior. It's essentially an assert without actually checking the
 * condition at run-time, allowing for stronger optimizations than LIKELY.
 */
#if HAS_BUILTIN(__builtin_assume)
#define ASSUME __builtin_assume
#else
#define ASSUME(x) do { if(!(x)) __builtin_unreachable(); } while(0)
#endif

#else

#define LIKELY(x) (!!(x))
#define UNLIKELY(x) (!!(x))
#ifdef _MSC_VER
#define ASSUME __assume
#else
#define ASSUME(x) ((void)0)
#endif
#endif

#ifndef UINT64_MAX
#define UINT64_MAX U64(18446744073709551615)
#endif

#ifndef UNUSED
#if defined(__cplusplus)
#define UNUSED(x)
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif
#endif

/* Calculates the size of a struct with N elements of a flexible array member.
 * GCC and Clang allow offsetof(Type, fam[N]) for this, but MSVC seems to have
 * trouble, so a bit more verbose workaround is needed.
 */
#define FAM_SIZE(T, M, N)  (offsetof(T, M) + sizeof(((T*)NULL)->M[0])*(N))


#ifdef __cplusplus
extern "C" {
#endif

typedef ALint64SOFT ALint64;
typedef ALuint64SOFT ALuint64;

#ifndef U64
#if defined(_MSC_VER)
#define U64(x) ((ALuint64)(x##ui64))
#elif SIZEOF_LONG == 8
#define U64(x) ((ALuint64)(x##ul))
#elif SIZEOF_LONG_LONG == 8
#define U64(x) ((ALuint64)(x##ull))
#endif
#endif

/* Define a CTZ64 macro (count trailing zeros, for 64-bit integers). The result
 * is *UNDEFINED* if the value is 0.
 */
#ifdef __GNUC__

#if SIZEOF_LONG == 8
#define CTZ64(x) __builtin_ctzl(x)
#else
#define CTZ64(x) __builtin_ctzll(x)
#endif

#elif defined(HAVE_BITSCANFORWARD64_INTRINSIC)

inline int msvc64_ctz64(ALuint64 v)
{
    unsigned long idx = 64;
    _BitScanForward64(&idx, v);
    return (int)idx;
}
#define CTZ64(x) msvc64_ctz64(x)

#elif defined(HAVE_BITSCANFORWARD_INTRINSIC)

inline int msvc_ctz64(ALuint64 v)
{
    unsigned long idx = 64;
    if(!_BitScanForward(&idx, v&0xffffffff))
    {
        if(_BitScanForward(&idx, v>>32))
            idx += 32;
    }
    return (int)idx;
}
#define CTZ64(x) msvc_ctz64(x)

#else

/* There be black magics here. The popcnt64 method is derived from
 * https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 * while the ctz-utilizing-popcnt algorithm is shown here
 * http://www.hackersdelight.org/hdcodetxt/ntz.c.txt
 * as the ntz2 variant. These likely aren't the most efficient methods, but
 * they're good enough if the GCC or MSVC intrinsics aren't available.
 */
inline int fallback_popcnt64(ALuint64 v)
{
    v = v - ((v >> 1) & U64(0x5555555555555555));
    v = (v & U64(0x3333333333333333)) + ((v >> 2) & U64(0x3333333333333333));
    v = (v + (v >> 4)) & U64(0x0f0f0f0f0f0f0f0f);
    return (int)((v * U64(0x0101010101010101)) >> 56);
}

inline int fallback_ctz64(ALuint64 value)
{
    return fallback_popcnt64(~value & (value - 1));
}
#define CTZ64(x) fallback_ctz64(x)
#endif

static const union {
    ALuint u;
    ALubyte b[sizeof(ALuint)];
} EndianTest = { 1 };
#define IS_LITTLE_ENDIAN (EndianTest.b[0] == 1)

#define COUNTOF(x) (sizeof(x) / sizeof(0[x]))


struct ll_ringbuffer;
struct Hrtf;
struct HrtfEntry;
struct DirectHrtfState;
struct FrontStablizer;
struct Compressor;
struct ALCbackend;
struct ALbuffer;
struct ALeffect;
struct ALfilter;
struct ALsource;
struct ALcontextProps;
struct ALlistenerProps;
struct ALvoiceProps;
struct ALeffectslotProps;


#define DEFAULT_OUTPUT_RATE  (44100)
#define MIN_OUTPUT_RATE      (8000)


/* Find the next power-of-2 for non-power-of-2 numbers. */
inline ALuint NextPowerOf2(ALuint value)
{
    if(value > 0)
    {
        value--;
        value |= value>>1;
        value |= value>>2;
        value |= value>>4;
        value |= value>>8;
        value |= value>>16;
    }
    return value+1;
}

/** Round up a value to the next multiple. */
inline size_t RoundUp(size_t value, size_t r)
{
    value += r-1;
    return value - (value%r);
}

/* Fast float-to-int conversion. No particular rounding mode is assumed; the
 * IEEE-754 default is round-to-nearest with ties-to-even, though an app could
 * change it on its own threads. On some systems, a truncating conversion may
 * always be the fastest method.
 */
inline ALint fastf2i(ALfloat f)
{
#if defined(HAVE_INTRIN_H) && ((defined(_M_IX86_FP) && (_M_IX86_FP > 0)) || defined(_M_X64))
    return _mm_cvt_ss2si(_mm_set1_ps(f));

#elif defined(_MSC_VER) && defined(_M_IX86_FP)

    ALint i;
    __asm fld f
    __asm fistp i
    return i;

#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))

    ALint i;
#ifdef __SSE_MATH__
    __asm__("cvtss2si %1, %0" : "=r"(i) : "x"(f));
#else
    __asm__("flds %1\n fistps %0" : "=m"(i) : "m"(f));
#endif
    return i;

    /* On GCC when compiling with -fno-math-errno, lrintf can be inlined to
     * some simple instructions. Clang does not inline it, always generating a
     * libc call, while MSVC's implementation is horribly slow, so always fall
     * back to a normal integer conversion for them.
     */
#elif defined(HAVE_LRINTF) && !defined(_MSC_VER) && !defined(__clang__)

    return lrintf(f);

#else

    return (ALint)f;
#endif
}

/* Converts float-to-int using standard behavior (truncation). */
inline int float2int(float f)
{
    /* TODO: Make a more efficient method for x87. */
    return (ALint)f;
}


enum DevProbe {
    ALL_DEVICE_PROBE,
    CAPTURE_DEVICE_PROBE
};


enum DistanceModel {
    InverseDistanceClamped  = AL_INVERSE_DISTANCE_CLAMPED,
    LinearDistanceClamped   = AL_LINEAR_DISTANCE_CLAMPED,
    ExponentDistanceClamped = AL_EXPONENT_DISTANCE_CLAMPED,
    InverseDistance  = AL_INVERSE_DISTANCE,
    LinearDistance   = AL_LINEAR_DISTANCE,
    ExponentDistance = AL_EXPONENT_DISTANCE,
    DisableDistance  = AL_NONE,

    DefaultDistanceModel = InverseDistanceClamped
};

enum Channel {
    FrontLeft = 0,
    FrontRight,
    FrontCenter,
    LFE,
    BackLeft,
    BackRight,
    BackCenter,
    SideLeft,
    SideRight,

    UpperFrontLeft,
    UpperFrontRight,
    UpperBackLeft,
    UpperBackRight,
    LowerFrontLeft,
    LowerFrontRight,
    LowerBackLeft,
    LowerBackRight,

    Aux0,
    Aux1,
    Aux2,
    Aux3,
    Aux4,
    Aux5,
    Aux6,
    Aux7,
    Aux8,
    Aux9,
    Aux10,
    Aux11,
    Aux12,
    Aux13,
    Aux14,
    Aux15,

    InvalidChannel
};


/* Device formats */
enum DevFmtType {
    DevFmtByte   = ALC_BYTE_SOFT,
    DevFmtUByte  = ALC_UNSIGNED_BYTE_SOFT,
    DevFmtShort  = ALC_SHORT_SOFT,
    DevFmtUShort = ALC_UNSIGNED_SHORT_SOFT,
    DevFmtInt    = ALC_INT_SOFT,
    DevFmtUInt   = ALC_UNSIGNED_INT_SOFT,
    DevFmtFloat  = ALC_FLOAT_SOFT,

    DevFmtTypeDefault = DevFmtFloat
};
enum DevFmtChannels {
    DevFmtMono   = ALC_MONO_SOFT,
    DevFmtStereo = ALC_STEREO_SOFT,
    DevFmtQuad   = ALC_QUAD_SOFT,
    DevFmtX51    = ALC_5POINT1_SOFT,
    DevFmtX61    = ALC_6POINT1_SOFT,
    DevFmtX71    = ALC_7POINT1_SOFT,
    DevFmtAmbi3D = ALC_BFORMAT3D_SOFT,

    /* Similar to 5.1, except using rear channels instead of sides */
    DevFmtX51Rear = 0x80000000,

    DevFmtChannelsDefault = DevFmtStereo
};
#define MAX_OUTPUT_CHANNELS  (16)

ALsizei BytesFromDevFmt(enum DevFmtType type);
ALsizei ChannelsFromDevFmt(enum DevFmtChannels chans, ALsizei ambiorder);
inline ALsizei FrameSizeFromDevFmt(enum DevFmtChannels chans, enum DevFmtType type, ALsizei ambiorder)
{
    return ChannelsFromDevFmt(chans, ambiorder) * BytesFromDevFmt(type);
}

enum AmbiLayout {
    AmbiLayout_FuMa = ALC_FUMA_SOFT, /* FuMa channel order */
    AmbiLayout_ACN = ALC_ACN_SOFT,   /* ACN channel order */

    AmbiLayout_Default = AmbiLayout_ACN
};

enum AmbiNorm {
    AmbiNorm_FuMa = ALC_FUMA_SOFT, /* FuMa normalization */
    AmbiNorm_SN3D = ALC_SN3D_SOFT, /* SN3D normalization */
    AmbiNorm_N3D = ALC_N3D_SOFT,   /* N3D normalization */

    AmbiNorm_Default = AmbiNorm_SN3D
};


enum DeviceType {
    Playback,
    Capture,
    Loopback
};


enum RenderMode {
    NormalRender,
    StereoPair,
    HrtfRender
};


/* The maximum number of Ambisonics coefficients. For a given order (o), the
 * size needed will be (o+1)**2, thus zero-order has 1, first-order has 4,
 * second-order has 9, third-order has 16, and fourth-order has 25.
 */
#define MAX_AMBI_ORDER  3
#define MAX_AMBI_COEFFS ((MAX_AMBI_ORDER+1) * (MAX_AMBI_ORDER+1))

/* A bitmask of ambisonic channels with height information. If none of these
 * channels are used/needed, there's no height (e.g. with most surround sound
 * speaker setups). This only specifies up to 4th order, which is the highest
 * order a 32-bit mask value can specify (a 64-bit mask could handle up to 7th
 * order). This is ACN ordering, with bit 0 being ACN 0, etc.
 */
#define AMBI_PERIPHONIC_MASK (0xfe7ce4)

/* The maximum number of Ambisonic coefficients for 2D (non-periphonic)
 * representation. This is 2 per each order above zero-order, plus 1 for zero-
 * order. Or simply, o*2 + 1.
 */
#define MAX_AMBI2D_COEFFS (MAX_AMBI_ORDER*2 + 1)


typedef ALfloat ChannelConfig[MAX_AMBI_COEFFS];
typedef struct BFChannelConfig {
    ALfloat Scale;
    ALsizei Index;
} BFChannelConfig;

typedef union AmbiConfig {
    /* Ambisonic coefficients for mixing to the dry buffer. */
    ChannelConfig Coeffs[MAX_OUTPUT_CHANNELS];
    /* Coefficient channel mapping for mixing to the dry buffer. */
    BFChannelConfig Map[MAX_OUTPUT_CHANNELS];
} AmbiConfig;


typedef struct BufferSubList {
    ALuint64 FreeMask;
    struct ALbuffer *Buffers; /* 64 */
} BufferSubList;
TYPEDEF_VECTOR(BufferSubList, vector_BufferSubList)

typedef struct EffectSubList {
    ALuint64 FreeMask;
    struct ALeffect *Effects; /* 64 */
} EffectSubList;
TYPEDEF_VECTOR(EffectSubList, vector_EffectSubList)

typedef struct FilterSubList {
    ALuint64 FreeMask;
    struct ALfilter *Filters; /* 64 */
} FilterSubList;
TYPEDEF_VECTOR(FilterSubList, vector_FilterSubList)

typedef struct SourceSubList {
    ALuint64 FreeMask;
    struct ALsource *Sources; /* 64 */
} SourceSubList;
TYPEDEF_VECTOR(SourceSubList, vector_SourceSubList)

/* Effect slots are rather large, and apps aren't likely to have more than one
 * or two (let alone 64), so hold them individually.
 */
typedef struct ALeffectslot *ALeffectslotPtr;
TYPEDEF_VECTOR(ALeffectslotPtr, vector_ALeffectslotPtr)


typedef struct EnumeratedHrtf {
    al_string name;

    struct HrtfEntry *hrtf;
} EnumeratedHrtf;
TYPEDEF_VECTOR(EnumeratedHrtf, vector_EnumeratedHrtf)


/* Maximum delay in samples for speaker distance compensation. */
#define MAX_DELAY_LENGTH 1024

typedef struct DistanceComp {
    ALfloat Gain;
    ALsizei Length; /* Valid range is [0...MAX_DELAY_LENGTH). */
    ALfloat *Buffer;
} DistanceComp;

/* Size for temporary storage of buffer data, in ALfloats. Larger values need
 * more memory, while smaller values may need more iterations. The value needs
 * to be a sensible size, however, as it constrains the max stepping value used
 * for mixing, as well as the maximum number of samples per mixing iteration.
 */
#define BUFFERSIZE 2048

typedef struct DryMixParams {
    AmbiConfig Ambi;
    /* Number of coefficients in each Ambi.Coeffs to mix together (4 for first-
     * order, 9 for second-order, etc). If the count is 0, Ambi.Map is used
     * instead to map each output to a coefficient index.
     */
    ALsizei CoeffCount;

    ALfloat (*Buffer)[BUFFERSIZE];
    ALsizei NumChannels;
    ALsizei NumChannelsPerOrder[MAX_AMBI_ORDER+1];
} DryMixParams;

typedef struct BFMixParams {
    AmbiConfig Ambi;
    /* Will only be 4 or 0. */
    ALsizei CoeffCount;

    ALfloat (*Buffer)[BUFFERSIZE];
    ALsizei NumChannels;
} BFMixParams;

typedef struct RealMixParams {
    enum Channel ChannelName[MAX_OUTPUT_CHANNELS];

    ALfloat (*Buffer)[BUFFERSIZE];
    ALsizei NumChannels;
} RealMixParams;

typedef void (*POSTPROCESS)(ALCdevice *device, ALsizei SamplesToDo);

struct ALCdevice_struct {
    RefCount ref;

    ATOMIC(ALenum) Connected;
    enum DeviceType Type;

    ALuint Frequency;
    ALuint UpdateSize;
    ALuint NumUpdates;
    enum DevFmtChannels FmtChans;
    enum DevFmtType     FmtType;
    ALboolean IsHeadphones;
    ALsizei AmbiOrder;
    /* For DevFmtAmbi* output only, specifies the channel order and
     * normalization.
     */
    enum AmbiLayout AmbiLayout;
    enum AmbiNorm   AmbiScale;

    al_string DeviceName;

    ATOMIC(ALCenum) LastError;

    // Maximum number of sources that can be created
    ALuint SourcesMax;
    // Maximum number of slots that can be created
    ALuint AuxiliaryEffectSlotMax;

    ALCuint NumMonoSources;
    ALCuint NumStereoSources;
    ALsizei NumAuxSends;

    // Map of Buffers for this device
    vector_BufferSubList BufferList;
    almtx_t BufferLock;

    // Map of Effects for this device
    vector_EffectSubList EffectList;
    almtx_t EffectLock;

    // Map of Filters for this device
    vector_FilterSubList FilterList;
    almtx_t FilterLock;

    POSTPROCESS PostProcess;

    /* HRTF state and info */
    struct DirectHrtfState *Hrtf;
    al_string HrtfName;
    struct Hrtf *HrtfHandle;
    vector_EnumeratedHrtf HrtfList;
    ALCenum HrtfStatus;

    /* UHJ encoder state */
    struct Uhj2Encoder *Uhj_Encoder;

    /* High quality Ambisonic decoder */
    struct BFormatDec *AmbiDecoder;

    /* Stereo-to-binaural filter */
    struct bs2b *Bs2b;

    /* First-order ambisonic upsampler for higher-order output */
    struct AmbiUpsampler *AmbiUp;

    /* Rendering mode. */
    enum RenderMode Render_Mode;

    // Device flags
    ALuint Flags;

    ALuint64 ClockBase;
    ALuint SamplesDone;

    /* Temp storage used for mixer processing. */
    alignas(16) ALfloat TempBuffer[4][BUFFERSIZE];

    /* The "dry" path corresponds to the main output. */
    DryMixParams Dry;

    /* First-order ambisonics output, to be upsampled to the dry buffer if different. */
    BFMixParams FOAOut;

    /* "Real" output, which will be written to the device buffer. May alias the
     * dry buffer.
     */
    RealMixParams RealOut;

    struct FrontStablizer *Stablizer;

    struct Compressor *Limiter;

    /* The average speaker distance as determined by the ambdec configuration
     * (or alternatively, by the NFC-HOA reference delay). Only used for NFC.
     */
    ALfloat AvgSpeakerDist;

    /* Delay buffers used to compensate for speaker distances. */
    DistanceComp ChannelDelay[MAX_OUTPUT_CHANNELS];

    /* Dithering control. */
    ALfloat DitherDepth;
    ALuint DitherSeed;

    /* Running count of the mixer invocations, in 31.1 fixed point. This
     * actually increments *twice* when mixing, first at the start and then at
     * the end, so the bottom bit indicates if the device is currently mixing
     * and the upper bits indicates how many mixes have been done.
     */
    RefCount MixCount;

    // Contexts created on this device
    ATOMIC(ALCcontext*) ContextList;

    almtx_t BackendLock;
    struct ALCbackend *Backend;

    ATOMIC(ALCdevice*) next;
};

// Frequency was requested by the app or config file
#define DEVICE_FREQUENCY_REQUEST                 (1u<<1)
// Channel configuration was requested by the config file
#define DEVICE_CHANNELS_REQUEST                  (1u<<2)
// Sample type was requested by the config file
#define DEVICE_SAMPLE_TYPE_REQUEST               (1u<<3)

// Specifies if the DSP is paused at user request
#define DEVICE_PAUSED                            (1u<<30)

// Specifies if the device is currently running
#define DEVICE_RUNNING                           (1u<<31)


/* Nanosecond resolution for the device clock time. */
#define DEVICE_CLOCK_RES  U64(1000000000)


/* Must be less than 15 characters (16 including terminating null) for
 * compatibility with pthread_setname_np limitations. */
#define MIXER_THREAD_NAME "alsoft-mixer"

#define RECORD_THREAD_NAME "alsoft-record"


enum {
    EventType_SourceStateChange = 1<<0,
    EventType_BufferCompleted   = 1<<1,
    EventType_Error             = 1<<2,
    EventType_Performance       = 1<<3,
    EventType_Deprecated        = 1<<4,
    EventType_Disconnected      = 1<<5,
};

typedef struct AsyncEvent {
    unsigned int EnumType;
    ALenum Type;
    ALuint ObjectId;
    ALuint Param;
    ALchar Message[1008];
} AsyncEvent;

struct ALCcontext_struct {
    RefCount ref;

    struct ALlistener *Listener;

    vector_SourceSubList SourceList;
    ALuint NumSources;
    almtx_t SourceLock;

    vector_ALeffectslotPtr EffectSlotList;
    almtx_t EffectSlotLock;

    ATOMIC(ALenum) LastError;

    enum DistanceModel DistanceModel;
    ALboolean SourceDistanceModel;

    ALfloat DopplerFactor;
    ALfloat DopplerVelocity;
    ALfloat SpeedOfSound;
    ALfloat MetersPerUnit;

    ATOMIC_FLAG PropsClean;
    ATOMIC(ALenum) DeferUpdates;

    almtx_t PropLock;

    /* Counter for the pre-mixing updates, in 31.1 fixed point (lowest bit
     * indicates if updates are currently happening).
     */
    RefCount UpdateCount;
    ATOMIC(ALenum) HoldUpdates;

    ALfloat GainBoost;

    ATOMIC(struct ALcontextProps*) Update;

    /* Linked lists of unused property containers, free to use for future
     * updates.
     */
    ATOMIC(struct ALcontextProps*) FreeContextProps;
    ATOMIC(struct ALlistenerProps*) FreeListenerProps;
    ATOMIC(struct ALvoiceProps*) FreeVoiceProps;
    ATOMIC(struct ALeffectslotProps*) FreeEffectslotProps;

    struct ALvoice **Voices;
    ALsizei VoiceCount;
    ALsizei MaxVoices;

    ATOMIC(struct ALeffectslotArray*) ActiveAuxSlots;

    almtx_t EventThrdLock;
    althrd_t EventThread;
    alsem_t EventSem;
    struct ll_ringbuffer *AsyncEvents;
    ATOMIC(ALbitfieldSOFT) EnabledEvts;
    almtx_t EventCbLock;
    ALEVENTPROCSOFT EventCb;
    void *EventParam;

    /* Default effect slot */
    struct ALeffectslot *DefaultSlot;

    ALCdevice  *Device;
    const ALCchar *ExtensionList;

    ATOMIC(ALCcontext*) next;

    /* Memory space used by the listener (and possibly default effect slot) */
    alignas(16) ALCbyte _listener_mem[];
};

ALCcontext *GetContextRef(void);

void ALCcontext_DecRef(ALCcontext *context);

void ALCcontext_DeferUpdates(ALCcontext *context);
void ALCcontext_ProcessUpdates(ALCcontext *context);

void AllocateVoices(ALCcontext *context, ALsizei num_voices, ALsizei old_sends);

void AppendAllDevicesList(const ALCchar *name);
void AppendCaptureDeviceList(const ALCchar *name);


extern ALint RTPrioLevel;
void SetRTPriority(void);

void SetDefaultChannelOrder(ALCdevice *device);
void SetDefaultWFXChannelOrder(ALCdevice *device);

const ALCchar *DevFmtTypeString(enum DevFmtType type);
const ALCchar *DevFmtChannelsString(enum DevFmtChannels chans);

inline ALint GetChannelIndex(const enum Channel names[MAX_OUTPUT_CHANNELS], enum Channel chan)
{
    ALint i;
    for(i = 0;i < MAX_OUTPUT_CHANNELS;i++)
    {
        if(names[i] == chan)
            return i;
    }
    return -1;
}
/**
 * GetChannelIdxByName
 *
 * Returns the index for the given channel name (e.g. FrontCenter), or -1 if it
 * doesn't exist.
 */
inline ALint GetChannelIdxByName(const RealMixParams *real, enum Channel chan)
{ return GetChannelIndex(real->ChannelName, chan); }


inline void LockBufferList(ALCdevice *device) { almtx_lock(&device->BufferLock); }
inline void UnlockBufferList(ALCdevice *device) { almtx_unlock(&device->BufferLock); }

inline void LockEffectList(ALCdevice *device) { almtx_lock(&device->EffectLock); }
inline void UnlockEffectList(ALCdevice *device) { almtx_unlock(&device->EffectLock); }

inline void LockFilterList(ALCdevice *device) { almtx_lock(&device->FilterLock); }
inline void UnlockFilterList(ALCdevice *device) { almtx_unlock(&device->FilterLock); }

inline void LockEffectSlotList(ALCcontext *context)
{ almtx_lock(&context->EffectSlotLock); }
inline void UnlockEffectSlotList(ALCcontext *context)
{ almtx_unlock(&context->EffectSlotLock); }


vector_al_string SearchDataFiles(const char *match, const char *subdir);

#ifdef __cplusplus
}
#endif

#endif
