#ifndef INPROGEXT_H
#define INPROGEXT_H

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ALC_SOFT_loopback2
#define ALC_SOFT_loopback2 1
#define ALC_AMBISONIC_LAYOUT_SOFT                0xfff0
#define ALC_AMBISONIC_SCALING_SOFT               0xfff1
#define ALC_AMBISONIC_ORDER_SOFT                 0xfff2
#define ALC_MAX_AMBISONIC_ORDER_SOFT             0xfff3

#define ALC_BFORMAT3D_SOFT                       0x1508

/* Ambisonic layouts */
#define ALC_ACN_SOFT                             0xfff4
#define ALC_FUMA_SOFT                            0xfff5

/* Ambisonic scalings (normalization) */
/*#define ALC_FUMA_SOFT*/
#define ALC_SN3D_SOFT                            0xfff6
#define ALC_N3D_SOFT                             0xfff7
#endif

#ifndef AL_SOFT_map_buffer
#define AL_SOFT_map_buffer 1
typedef unsigned int ALbitfieldSOFT;
#define AL_MAP_READ_BIT_SOFT                     0x00000001
#define AL_MAP_WRITE_BIT_SOFT                    0x00000002
#define AL_MAP_PERSISTENT_BIT_SOFT               0x00000004
#define AL_PRESERVE_DATA_BIT_SOFT                0x00000008
typedef void (AL_APIENTRY*LPALBUFFERSTORAGESOFT)(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq, ALbitfieldSOFT flags);
typedef void* (AL_APIENTRY*LPALMAPBUFFERSOFT)(ALuint buffer, ALsizei offset, ALsizei length, ALbitfieldSOFT access);
typedef void (AL_APIENTRY*LPALUNMAPBUFFERSOFT)(ALuint buffer);
typedef void (AL_APIENTRY*LPALFLUSHMAPPEDBUFFERSOFT)(ALuint buffer, ALsizei offset, ALsizei length);
#ifdef AL_ALEXT_PROTOTYPES
AL_API void AL_APIENTRY alBufferStorageSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq, ALbitfieldSOFT flags);
AL_API void* AL_APIENTRY alMapBufferSOFT(ALuint buffer, ALsizei offset, ALsizei length, ALbitfieldSOFT access);
AL_API void AL_APIENTRY alUnmapBufferSOFT(ALuint buffer);
AL_API void AL_APIENTRY alFlushMappedBufferSOFT(ALuint buffer, ALsizei offset, ALsizei length);
#endif
#endif

#ifndef AL_SOFT_events
#define AL_SOFT_events 1
#define AL_EVENT_CALLBACK_FUNCTION_SOFT          0x1220
#define AL_EVENT_CALLBACK_USER_PARAM_SOFT        0x1221
#define AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT      0x1222
#define AL_EVENT_TYPE_SOURCE_STATE_CHANGED_SOFT  0x1223
#define AL_EVENT_TYPE_ERROR_SOFT                 0x1224
#define AL_EVENT_TYPE_PERFORMANCE_SOFT           0x1225
#define AL_EVENT_TYPE_DEPRECATED_SOFT            0x1226
#define AL_EVENT_TYPE_DISCONNECTED_SOFT          0x1227
typedef void (AL_APIENTRY*ALEVENTPROCSOFT)(ALenum eventType, ALuint object, ALuint param,
                                           ALsizei length, const ALchar *message,
                                           void *userParam);
typedef void (AL_APIENTRY*LPALEVENTCONTROLSOFT)(ALsizei count, const ALenum *types, ALboolean enable);
typedef void (AL_APIENTRY*LPALEVENTCALLBACKSOFT)(ALEVENTPROCSOFT callback, void *userParam);
typedef void* (AL_APIENTRY*LPALGETPOINTERSOFT)(ALenum pname);
typedef void (AL_APIENTRY*LPALGETPOINTERVSOFT)(ALenum pname, void **values);
#ifdef AL_ALEXT_PROTOTYPES
AL_API void AL_APIENTRY alEventControlSOFT(ALsizei count, const ALenum *types, ALboolean enable);
AL_API void AL_APIENTRY alEventCallbackSOFT(ALEVENTPROCSOFT callback, void *userParam);
AL_API void* AL_APIENTRY alGetPointerSOFT(ALenum pname);
AL_API void AL_APIENTRY alGetPointervSOFT(ALenum pname, void **values);
#endif
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INPROGEXT_H */
