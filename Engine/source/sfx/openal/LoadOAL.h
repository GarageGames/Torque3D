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

#ifndef _LOADOAL_H_
#define _LOADOAL_H_

#ifndef _PLATFORM_H_
#  include "platform/platform.h"
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx-presets.h>

#ifndef ALAPIENTRY
#define ALAPIENTRY
#endif

#ifndef ALCAPIENTRY
#define ALCAPIENTRY
#endif

// Open AL Function table definition

#ifndef _OPENALFNTABLE
#define _OPENALFNTABLE

// AL 1.0 did not define the ALchar and ALCchar types, so define them here
// if they don't exist

#ifndef ALchar
#define ALchar char
#endif

#ifndef ALCchar
#define ALCchar char
#endif

// Complete list of functions available in AL 1.0 implementations

typedef void           (ALAPIENTRY *LPALENABLE)( ALenum capability );
typedef void           (ALAPIENTRY *LPALDISABLE)( ALenum capability ); 
typedef ALboolean      (ALAPIENTRY *LPALISENABLED)( ALenum capability ); 
typedef const ALchar*  (ALAPIENTRY *LPALGETSTRING)( ALenum param );
typedef void           (ALAPIENTRY *LPALGETBOOLEANV)( ALenum param, ALboolean* data );
typedef void           (ALAPIENTRY *LPALGETINTEGERV)( ALenum param, ALint* data );
typedef void           (ALAPIENTRY *LPALGETFLOATV)( ALenum param, ALfloat* data );
typedef void           (ALAPIENTRY *LPALGETDOUBLEV)( ALenum param, ALdouble* data );
typedef ALboolean      (ALAPIENTRY *LPALGETBOOLEAN)( ALenum param );
typedef ALint          (ALAPIENTRY *LPALGETINTEGER)( ALenum param );
typedef ALfloat        (ALAPIENTRY *LPALGETFLOAT)( ALenum param );
typedef ALdouble       (ALAPIENTRY *LPALGETDOUBLE)( ALenum param );
typedef ALenum         (ALAPIENTRY *LPALGETERROR)( void );
typedef ALboolean      (ALAPIENTRY *LPALISEXTENSIONPRESENT)(const ALchar* extname );
typedef void*          (ALAPIENTRY *LPALGETPROCADDRESS)( const ALchar* fname );
typedef ALenum         (ALAPIENTRY *LPALGETENUMVALUE)( const ALchar* ename );
typedef void           (ALAPIENTRY *LPALLISTENERF)( ALenum param, ALfloat value );
typedef void           (ALAPIENTRY *LPALLISTENER3F)( ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void           (ALAPIENTRY *LPALLISTENERFV)( ALenum param, const ALfloat* values );
typedef void           (ALAPIENTRY *LPALLISTENERI)( ALenum param, ALint value );
typedef void           (ALAPIENTRY *LPALGETLISTENERF)( ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETLISTENER3F)( ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3 );
typedef void           (ALAPIENTRY *LPALGETLISTENERFV)( ALenum param, ALfloat* values );
typedef void           (ALAPIENTRY *LPALGETLISTENERI)( ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALGENSOURCES)( ALsizei n, ALuint* sources ); 
typedef void           (ALAPIENTRY *LPALDELETESOURCES)( ALsizei n, const ALuint* sources );
typedef ALboolean      (ALAPIENTRY *LPALISSOURCE)( ALuint sid ); 
typedef void           (ALAPIENTRY *LPALSOURCEF)( ALuint sid, ALenum param, ALfloat value); 
typedef void           (ALAPIENTRY *LPALSOURCE3F)( ALuint sid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void           (ALAPIENTRY *LPALSOURCEFV)( ALuint sid, ALenum param, const ALfloat* values );
typedef void           (ALAPIENTRY *LPALSOURCEI)( ALuint sid, ALenum param, ALint value); 
typedef void           (ALAPIENTRY *LPALGETSOURCEF)( ALuint sid, ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETSOURCE3F)( ALuint sid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
typedef void           (ALAPIENTRY *LPALGETSOURCEFV)( ALuint sid, ALenum param, ALfloat* values );
typedef void           (ALAPIENTRY *LPALGETSOURCEI)( ALuint sid, ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALSOURCEPLAYV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCESTOPV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEREWINDV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEPAUSEV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEPLAY)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCESTOP)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEREWIND)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEPAUSE)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, const ALuint *bids );
typedef void           (ALAPIENTRY *LPALSOURCEUNQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, ALuint *bids );
typedef void           (ALAPIENTRY *LPALGENBUFFERS)( ALsizei n, ALuint* buffers );
typedef void           (ALAPIENTRY *LPALDELETEBUFFERS)( ALsizei n, const ALuint* buffers );
typedef ALboolean      (ALAPIENTRY *LPALISBUFFER)( ALuint bid );
typedef void           (ALAPIENTRY *LPALBUFFERDATA)( ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq );
typedef void           (ALAPIENTRY *LPALGETBUFFERF)( ALuint bid, ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETBUFFERI)( ALuint bid, ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALDOPPLERFACTOR)( ALfloat value );
typedef void           (ALAPIENTRY *LPALDOPPLERVELOCITY)( ALfloat value );
typedef void           (ALAPIENTRY *LPALDISTANCEMODEL)( ALenum distanceModel );

typedef ALCcontext *   (ALCAPIENTRY *LPALCCREATECONTEXT) (ALCdevice *device, const ALCint *attrlist);
typedef ALCboolean     (ALCAPIENTRY *LPALCMAKECONTEXTCURRENT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCPROCESSCONTEXT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCSUSPENDCONTEXT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCDESTROYCONTEXT)( ALCcontext *context );
typedef ALCcontext *   (ALCAPIENTRY *LPALCGETCURRENTCONTEXT)( void );
typedef ALCdevice *    (ALCAPIENTRY *LPALCGETCONTEXTSDEVICE)( ALCcontext *context );
typedef ALCdevice *    (ALCAPIENTRY *LPALCOPENDEVICE)( const ALCchar *devicename );
typedef ALCboolean     (ALCAPIENTRY *LPALCCLOSEDEVICE)( ALCdevice *device );
typedef ALCenum        (ALCAPIENTRY *LPALCGETERROR)( ALCdevice *device );
typedef ALCboolean     (ALCAPIENTRY *LPALCISEXTENSIONPRESENT)( ALCdevice *device, const ALCchar *extname );
typedef void *         (ALCAPIENTRY *LPALCGETPROCADDRESS)(ALCdevice *device, const ALCchar *funcname );
typedef ALCenum        (ALCAPIENTRY *LPALCGETENUMVALUE)(ALCdevice *device, const ALCchar *enumname );
typedef const ALCchar* (ALCAPIENTRY *LPALCGETSTRING)( ALCdevice *device, ALCenum param );
typedef void           (ALCAPIENTRY *LPALCGETINTEGERV)( ALCdevice *device, ALCenum param, ALCsizei size, ALCint *dest );
///Changes for effects
typedef void			  (ALAPIENTRY *LPALGENEFFECTS)(ALsizei n, ALuint *effects);
typedef void			  (ALAPIENTRY *LPALDELETEEFFECTS)(ALsizei n, const ALuint *effects);
typedef ALboolean      (ALAPIENTRY *LPALISEFFECT)(ALuint effect);
typedef void			  (ALAPIENTRY *LPALEFFECTI)(ALuint effect, ALenum param, ALint value);
typedef void			  (ALAPIENTRY *LPALEFFECTIV)(ALuint effect, ALenum param, const ALint *values);
typedef void			  (ALAPIENTRY *LPALEFFECTF)(ALuint effect, ALenum param, ALfloat value);
typedef void			  (ALAPIENTRY *LPALEFFECTFV)(ALuint effect, ALenum param, const ALfloat *values);
typedef void			  (ALAPIENTRY *LPALGETEFFECTI)(ALuint effect, ALenum param, ALint *value);
typedef void			  (ALAPIENTRY *LPALGETEFFECTIV)(ALuint effect, ALenum param, ALint *values);
typedef void			  (ALAPIENTRY *LPALGETEFFECTF)(ALuint effect, ALenum param, ALfloat *value);
typedef void			  (ALAPIENTRY *LPALGETEFFECTFV)(ALuint effect, ALenum param, ALfloat *values);
typedef void			  (ALAPIENTRY *LPALRELEASEALEFFECTS)(ALCdevice *device);
typedef void			  (ALAPIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei n, ALuint *effectslots);
typedef void			  (ALAPIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei n, const ALuint *effectslots);
typedef ALboolean      (ALAPIENTRY *LPALISAUXILIARYEFFECTSLOT)(ALuint effectslot);
typedef void		     (ALAPIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint effectslot, ALenum param, ALint value);
typedef void			  (ALAPIENTRY *LPALAUXILIARYEFFECTSLOTIV)(ALuint effectslot, ALenum param, const ALint *values);
typedef void			  (ALAPIENTRY *LPALAUXILIARYEFFECTSLOTF)(ALuint effectslot, ALenum param, ALfloat value);
typedef void			  (ALAPIENTRY *LPALAUXILIARYEFFECTSLOTFV)(ALuint effectslot, ALenum param, const ALfloat *values);
typedef void			  (ALAPIENTRY *LPALGETAUXILIARYEFFECTSLOTI)(ALuint effectslot, ALenum param, ALint *value);
typedef void			  (ALAPIENTRY *LPALGETAUXILIARYEFFECTSLOTIV)(ALuint effectslot, ALenum param, ALint *values);
typedef void			  (ALAPIENTRY *LPALGETAUXILIARYEFFECTSLOTF)(ALuint effectslot, ALenum param, ALfloat *value);
typedef void			  (ALAPIENTRY *LPALGETAUXILIARYEFFECTSLOTFV)(ALuint effectslot, ALenum param, ALfloat *values);
typedef void			  (ALAPIENTRY *LPALSOURCE3I)(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3);

typedef struct
{
	LPALENABLE                 alEnable;
	LPALDISABLE                alDisable;
	LPALISENABLED              alIsEnabled;
	LPALGETBOOLEAN             alGetBoolean;
	LPALGETINTEGER             alGetInteger;
	LPALGETFLOAT               alGetFloat;
	LPALGETDOUBLE              alGetDouble;
	LPALGETBOOLEANV				alGetBooleanv;
	LPALGETINTEGERV				alGetIntegerv;
	LPALGETFLOATV              alGetFloatv;
	LPALGETDOUBLEV             alGetDoublev;
	LPALGETSTRING              alGetString;
	LPALGETERROR               alGetError;
	LPALISEXTENSIONPRESENT		alIsExtensionPresent;
	LPALGETPROCADDRESS			alGetProcAddress;
	LPALGETENUMVALUE           alGetEnumValue;
	LPALLISTENERI              alListeneri;
	LPALLISTENERF				   alListenerf;
	LPALLISTENER3F				   alListener3f;
	LPALLISTENERFV				   alListenerfv;
	LPALGETLISTENERI			   alGetListeneri;
	LPALGETLISTENERF			   alGetListenerf;
	LPALGETLISTENER3F			   alGetListener3f;
	LPALGETLISTENERFV			   alGetListenerfv;
	LPALGENSOURCES				   alGenSources;
	LPALDELETESOURCES			   alDeleteSources;
	LPALISSOURCE				   alIsSource;
	LPALSOURCEI					   alSourcei;
	LPALSOURCEF					   alSourcef;
   LPALSOURCE3I				   alSource3i;
	LPALSOURCE3F				   alSource3f;
	LPALSOURCEFV				   alSourcefv;
	LPALGETSOURCEI				   alGetSourcei;
	LPALGETSOURCEF				   alGetSourcef;
	LPALGETSOURCEFV				alGetSourcefv;
	LPALSOURCEPLAYV				alSourcePlayv;
	LPALSOURCESTOPV				alSourceStopv;
	LPALSOURCEPLAY				   alSourcePlay;
	LPALSOURCEPAUSE				alSourcePause;
	LPALSOURCESTOP				   alSourceStop;
   LPALSOURCEREWIND           alSourceRewind;
	LPALGENBUFFERS				   alGenBuffers;
	LPALDELETEBUFFERS			   alDeleteBuffers;
	LPALISBUFFER				   alIsBuffer;
	LPALBUFFERDATA				   alBufferData;
	LPALGETBUFFERI				   alGetBufferi;
	LPALGETBUFFERF				   alGetBufferf;
	LPALSOURCEQUEUEBUFFERS		alSourceQueueBuffers;
	LPALSOURCEUNQUEUEBUFFERS	alSourceUnqueueBuffers;
	LPALDISTANCEMODEL			   alDistanceModel;
	LPALDOPPLERFACTOR			   alDopplerFactor;
	LPALDOPPLERVELOCITY			alDopplerVelocity;
	LPALCGETSTRING				   alcGetString;
	LPALCGETINTEGERV			   alcGetIntegerv;
	LPALCOPENDEVICE				alcOpenDevice;
	LPALCCLOSEDEVICE			   alcCloseDevice;
	LPALCCREATECONTEXT			alcCreateContext;
	LPALCMAKECONTEXTCURRENT		alcMakeContextCurrent;
	LPALCPROCESSCONTEXT			alcProcessContext;
	LPALCGETCURRENTCONTEXT		alcGetCurrentContext;
	LPALCGETCONTEXTSDEVICE		alcGetContextsDevice;
	LPALCSUSPENDCONTEXT			alcSuspendContext;
	LPALCDESTROYCONTEXT			alcDestroyContext;
	LPALCGETERROR				   alcGetError;
	LPALCISEXTENSIONPRESENT		alcIsExtensionPresent;
	LPALCGETPROCADDRESS			alcGetProcAddress;
	LPALCGETENUMVALUE			   alcGetEnumValue;
   LPALGENEFFECTS				   alGenEffects;
   LPALDELETEEFFECTS			   alDeleteEffects;
   LPALISEFFECT				   alIsEffect;
   LPALEFFECTI					   alEffecti;
   LPALEFFECTIV				   alEffectiv;
   LPALEFFECTF					   alEffectf;
   LPALEFFECTFV				   alEffectfv;
   LPALGETEFFECTI 				alGetEffecti;
   LPALGETEFFECTIV				alGetEffectiv;
   LPALGETEFFECTF				   alGetEffectf;
   LPALGETEFFECTFV				alGetEffectfv;
   LPALRELEASEALEFFECTS		   alReleaseEffects;
   LPALGENAUXILIARYEFFECTSLOTS			alGenAuxiliaryEffectSlots;
   LPALDELETEAUXILIARYEFFECTSLOTS		alDeleteAuxiliaryEffectSlots;
   LPALISAUXILIARYEFFECTSLOT				alIsAuxiliaryEffectSlot;
   LPALAUXILIARYEFFECTSLOTI				alAuxiliaryEffectSloti;
   LPALAUXILIARYEFFECTSLOTIV				alAuxiliaryEffectSlotiv;
   LPALAUXILIARYEFFECTSLOTF				alAuxiliaryEffectSlotf;
   LPALAUXILIARYEFFECTSLOTFV				alAuxiliaryEffectSlotfv;
   LPALGETAUXILIARYEFFECTSLOTI			alGetAuxiliaryEffectSloti;
   LPALGETAUXILIARYEFFECTSLOTIV			alGetAuxiliaryEffectSlotiv;
   LPALGETAUXILIARYEFFECTSLOTF			alGetAuxiliaryEffectSlotf;
   LPALGETAUXILIARYEFFECTSLOTFV			alGetAuxiliaryEffectSlotfv;
} OPENALFNTABLE, *LPOPENALFNTABLE;
#endif

ALboolean LoadOAL10Library(char *szOALFullPathName, LPOPENALFNTABLE lpOALFnTable);
ALvoid UnloadOAL10Library();

#endif // _LOADOAL_H_
