//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMAL_H_
#define _PLATFORMAL_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#define AL_NO_PROTOTYPES
#include <al/al.h>
#include <al/alc.h>
#include <al/alut.h>

// extra enums for win32/miles implementation
enum {
   // error values
   AL_CONTEXT_ALREADY_INSTANTIATED = 0xbaadf00d,
   AL_ENVIRONMENT_ALREADY_INSTANTIATED,
   AL_UNSUPPORTED,
   AL_INVALID_BUFFER,
   AL_ERROR,

   // context extention
   ALC_PROVIDER,
   ALC_PROVIDER_COUNT,
   ALC_PROVIDER_NAME,
   ALC_SPEAKER,
   ALC_SPEAKER_COUNT,
   ALC_SPEAKER_NAME,
   ALC_BUFFER_DYNAMIC_MEMORY_SIZE,
   ALC_BUFFER_DYNAMIC_MEMORY_USAGE,
   ALC_BUFFER_DYNAMIC_COUNT,
   ALC_BUFFER_MEMORY_USAGE,
   ALC_BUFFER_COUNT,
   ALC_BUFFER_LATENCY,

   // misc 3d params
   AL_MIN_DISTANCE,
   AL_MAX_DISTANCE,
   AL_CONE_OUTER_GAIN,
   
   // relative with pos(0,0,0) won't work for ambient sounds with miles
   AL_SOURCE_AMBIENT,
   AL_PAN,
   
   // other extensions
   AL_BUFFER_KEEP_RESIDENT,
   AL_FORMAT_WAVE_EXT,

   // Environment extensions:
   AL_ENV_EFFECT_VOLUME_EXT,
   AL_ENV_FLAGS_EXT,
   AL_ENV_DAMPING_EXT,
   AL_ENV_ENVIRONMENT_SIZE_EXT,
   AL_ENV_ROOM_VOLUME_EXT,
};

enum {
   // sample level environment:
   AL_ENV_SAMPLE_REVERB_MIX_EXT = 0,
   AL_ENV_SAMPLE_DIRECT_EXT,
   AL_ENV_SAMPLE_DIRECT_HF_EXT,
   AL_ENV_SAMPLE_ROOM_EXT,
   AL_ENV_SAMPLE_ROOM_HF_EXT,
   AL_ENV_SAMPLE_OBSTRUCTION_EXT,
   AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT,
   AL_ENV_SAMPLE_OCCLUSION_EXT,
   AL_ENV_SAMPLE_OCCLUSION_LF_RATIO_EXT,
   AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT,
   AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT,
   AL_ENV_SAMPLE_AIR_ABSORPTION_EXT,
   AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT,
   AL_ENV_SAMPLE_FLAGS_EXT,

   AL_ENV_SAMPLE_COUNT,
};

// room types: same as miles/eax
enum {
    AL_ENVIRONMENT_GENERIC = 0,
    AL_ENVIRONMENT_PADDEDCELL,
    AL_ENVIRONMENT_ROOM,
    AL_ENVIRONMENT_BATHROOM,
    AL_ENVIRONMENT_LIVINGROOM,
    AL_ENVIRONMENT_STONEROOM,
    AL_ENVIRONMENT_AUDITORIUM,
    AL_ENVIRONMENT_CONCERTHALL,
    AL_ENVIRONMENT_CAVE,
    AL_ENVIRONMENT_ARENA,
    AL_ENVIRONMENT_HANGAR,
    AL_ENVIRONMENT_CARPETEDHALLWAY,
    AL_ENVIRONMENT_HALLWAY,
    AL_ENVIRONMENT_STONECORRIDOR,
    AL_ENVIRONMENT_ALLEY,
    AL_ENVIRONMENT_FOREST,
    AL_ENVIRONMENT_CITY,
    AL_ENVIRONMENT_MOUNTAINS,
    AL_ENVIRONMENT_QUARRY,
    AL_ENVIRONMENT_PLAIN,
    AL_ENVIRONMENT_PARKINGLOT,
    AL_ENVIRONMENT_SEWERPIPE,
    AL_ENVIRONMENT_UNDERWATER,
    AL_ENVIRONMENT_DRUGGED,
    AL_ENVIRONMENT_DIZZY,
    AL_ENVIRONMENT_PSYCHOTIC,

    AL_ENVIRONMENT_COUNT
};

// declare OpenAL functions
#define AL_EXTENSION(ext_name) extern bool gDoesSupport_##ext_name;
#define AL_FUNCTION(fn_return,fn_name,fn_args) extern fn_return (FN_CDECL *fn_name)fn_args;
#define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) extern fn_return (FN_CDECL *fn_name)fn_args;
#ifndef _OPENALFN_H_
#include <openALFn.h>
#endif

namespace Audio
{

bool libraryInit(const char *library);
void libraryInitExtensions();
void libraryShutdown();

inline bool doesSupportIASIG()
{
   return gDoesSupport_AL_EXT_IASIG;
}   

inline bool doesSupportDynamix()
{
   return gDoesSupport_AL_EXT_DYNAMIX;
}  

// helpers
F32 DBToLinear(F32 value);
F32 linearToDB(F32 value);

}  // end namespace Audio


#endif  // _H_PLATFORMAL_
