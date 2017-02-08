#ifndef _AL_BUFFER_H_
#define _AL_BUFFER_H_

#include "alMain.h"

#ifdef __cplusplus
extern "C" {
#endif

/* User formats */
enum UserFmtType {
    UserFmtByte   = AL_BYTE_SOFT,
    UserFmtUByte  = AL_UNSIGNED_BYTE_SOFT,
    UserFmtShort  = AL_SHORT_SOFT,
    UserFmtUShort = AL_UNSIGNED_SHORT_SOFT,
    UserFmtInt    = AL_INT_SOFT,
    UserFmtUInt   = AL_UNSIGNED_INT_SOFT,
    UserFmtFloat  = AL_FLOAT_SOFT,
    UserFmtDouble = AL_DOUBLE_SOFT,
    UserFmtByte3  = AL_BYTE3_SOFT,
    UserFmtUByte3 = AL_UNSIGNED_BYTE3_SOFT,
    UserFmtMulaw  = AL_MULAW_SOFT,
    UserFmtAlaw   = 0x10000000,
    UserFmtIMA4,
    UserFmtMSADPCM,
};
enum UserFmtChannels {
    UserFmtMono      = AL_MONO_SOFT,
    UserFmtStereo    = AL_STEREO_SOFT,
    UserFmtRear      = AL_REAR_SOFT,
    UserFmtQuad      = AL_QUAD_SOFT,
    UserFmtX51       = AL_5POINT1_SOFT, /* (WFX order) */
    UserFmtX61       = AL_6POINT1_SOFT, /* (WFX order) */
    UserFmtX71       = AL_7POINT1_SOFT, /* (WFX order) */
    UserFmtBFormat2D = AL_BFORMAT2D_SOFT, /* WXY */
    UserFmtBFormat3D = AL_BFORMAT3D_SOFT, /* WXYZ */
};

ALuint BytesFromUserFmt(enum UserFmtType type);
ALuint ChannelsFromUserFmt(enum UserFmtChannels chans);
inline ALuint FrameSizeFromUserFmt(enum UserFmtChannels chans, enum UserFmtType type)
{
    return ChannelsFromUserFmt(chans) * BytesFromUserFmt(type);
}


/* Storable formats */
enum FmtType {
    FmtByte  = UserFmtByte,
    FmtShort = UserFmtShort,
    FmtFloat = UserFmtFloat,
};
enum FmtChannels {
    FmtMono   = UserFmtMono,
    FmtStereo = UserFmtStereo,
    FmtRear   = UserFmtRear,
    FmtQuad   = UserFmtQuad,
    FmtX51    = UserFmtX51,
    FmtX61    = UserFmtX61,
    FmtX71    = UserFmtX71,
    FmtBFormat2D = UserFmtBFormat2D,
    FmtBFormat3D = UserFmtBFormat3D,
};
#define MAX_INPUT_CHANNELS  (8)

ALuint BytesFromFmt(enum FmtType type);
ALuint ChannelsFromFmt(enum FmtChannels chans);
inline ALuint FrameSizeFromFmt(enum FmtChannels chans, enum FmtType type)
{
    return ChannelsFromFmt(chans) * BytesFromFmt(type);
}


typedef struct ALbuffer {
    ALvoid  *data;

    ALsizei  Frequency;
    ALenum   Format;
    ALsizei  SampleLen;

    enum FmtChannels FmtChannels;
    enum FmtType     FmtType;
    ALuint BytesAlloc;

    enum UserFmtChannels OriginalChannels;
    enum UserFmtType     OriginalType;
    ALsizei              OriginalSize;
    ALsizei              OriginalAlign;

    ALsizei  LoopStart;
    ALsizei  LoopEnd;

    ATOMIC(ALsizei) UnpackAlign;
    ATOMIC(ALsizei) PackAlign;

    /* Number of times buffer was attached to a source (deletion can only occur when 0) */
    RefCount ref;

    RWLock lock;

    /* Self ID */
    ALuint id;
} ALbuffer;

ALbuffer *NewBuffer(ALCcontext *context);
void DeleteBuffer(ALCdevice *device, ALbuffer *buffer);

ALenum LoadData(ALbuffer *buffer, ALuint freq, ALenum NewFormat, ALsizei frames, enum UserFmtChannels SrcChannels, enum UserFmtType SrcType, const ALvoid *data, ALsizei align, ALboolean storesrc);

inline void LockBuffersRead(ALCdevice *device)
{ LockUIntMapRead(&device->BufferMap); }
inline void UnlockBuffersRead(ALCdevice *device)
{ UnlockUIntMapRead(&device->BufferMap); }
inline void LockBuffersWrite(ALCdevice *device)
{ LockUIntMapWrite(&device->BufferMap); }
inline void UnlockBuffersWrite(ALCdevice *device)
{ UnlockUIntMapWrite(&device->BufferMap); }

inline struct ALbuffer *LookupBuffer(ALCdevice *device, ALuint id)
{ return (struct ALbuffer*)LookupUIntMapKeyNoLock(&device->BufferMap, id); }
inline struct ALbuffer *RemoveBuffer(ALCdevice *device, ALuint id)
{ return (struct ALbuffer*)RemoveUIntMapKeyNoLock(&device->BufferMap, id); }

ALvoid ReleaseALBuffers(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
