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

#include <dlfcn.h>
#include <err.h>
#include <string.h>
#include "sfx/openal/LoadOAL.h"
#include "console/console.h"

void* openal_library = NULL;

ALboolean LoadOAL10Library(char *szOALFullPathName, LPOPENALFNTABLE lpOALFnTable)
{
   if (!lpOALFnTable)
      return AL_FALSE;
    
   if (szOALFullPathName)
      openal_library = dlopen(szOALFullPathName, RTLD_LAZY);
   else
   #ifdef TORQUE_OS_MAC
      openal_library = dlopen("libopenal.dylib", RTLD_LAZY);
   #else
      openal_library = dlopen("libopenal.so", RTLD_LAZY);
   #endif
    
   if (openal_library == NULL)
   {
      Con::errorf("Failed to load OpenAL shared library. Sound will not be available");
      return AL_FALSE;
   }
    
   memset(lpOALFnTable, 0, sizeof(OPENALFNTABLE));
    
   lpOALFnTable->alEnable = (LPALENABLE)dlsym(openal_library,"alEnable");
   if (lpOALFnTable->alEnable == NULL)
   {
      Con::errorf("Failed to retrieve 'alEnable' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDisable = (LPALDISABLE)dlsym(openal_library,"alDisable");
   if (lpOALFnTable->alDisable == NULL)
   {
      Con::errorf("Failed to retrieve 'alDisable' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsEnabled = (LPALISENABLED)dlsym(openal_library,"alIsEnabled");
   if (lpOALFnTable->alIsEnabled == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsEnabled' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetBoolean = (LPALGETBOOLEAN)dlsym(openal_library,"alGetBoolean");
   if (lpOALFnTable->alGetBoolean == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetBoolean' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetInteger = (LPALGETINTEGER)dlsym(openal_library,"alGetInteger");
   if (lpOALFnTable->alGetInteger == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetInteger' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetFloat = (LPALGETFLOAT)dlsym(openal_library,"alGetFloat");
   if (lpOALFnTable->alGetFloat == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetFloat' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetDouble = (LPALGETDOUBLE)dlsym(openal_library,"alGetDouble");
   if (lpOALFnTable->alGetDouble == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetDouble' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetBooleanv = (LPALGETBOOLEANV)dlsym(openal_library,"alGetBooleanv");
   if (lpOALFnTable->alGetBooleanv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetBooleanv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetIntegerv = (LPALGETINTEGERV)dlsym(openal_library,"alGetIntegerv");
   if (lpOALFnTable->alGetIntegerv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetIntegerv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetFloatv = (LPALGETFLOATV)dlsym(openal_library,"alGetFloatv");
   if (lpOALFnTable->alGetFloatv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetFloatv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetDoublev = (LPALGETDOUBLEV)dlsym(openal_library,"alGetDoublev");
   if (lpOALFnTable->alGetDoublev == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetDoublev' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetString = (LPALGETSTRING)dlsym(openal_library,"alGetString");
   if (lpOALFnTable->alGetString == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetString' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetError = (LPALGETERROR)dlsym(openal_library,"alGetError");
   if (lpOALFnTable->alGetError == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetError' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsExtensionPresent = (LPALISEXTENSIONPRESENT)dlsym(openal_library,"alIsExtensionPresent");
   if (lpOALFnTable->alIsExtensionPresent == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsExtensionPresent' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetProcAddress = (LPALGETPROCADDRESS)dlsym(openal_library,"alGetProcAddress");
   if (lpOALFnTable->alGetProcAddress == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetProcAddress' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetEnumValue = (LPALGETENUMVALUE)dlsym(openal_library,"alGetEnumValue");
   if (lpOALFnTable->alGetEnumValue == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetEnumValue' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alListeneri = (LPALLISTENERI)dlsym(openal_library,"alListeneri");
   if (lpOALFnTable->alListeneri == NULL)
   {
      Con::errorf("Failed to retrieve 'alListeneri' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alListenerf = (LPALLISTENERF)dlsym(openal_library,"alListenerf");
   if (lpOALFnTable->alListenerf == NULL)
   {
      Con::errorf("Failed to retrieve 'alListenerf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alListener3f = (LPALLISTENER3F)dlsym(openal_library,"alListener3f");
   if (lpOALFnTable->alListener3f == NULL)
   {
      Con::errorf("Failed to retrieve 'alListener3f' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alListenerfv = (LPALLISTENERFV)dlsym(openal_library,"alListenerfv");
   if (lpOALFnTable->alListenerfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alListenerfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetListeneri = (LPALGETLISTENERI)dlsym(openal_library,"alGetListeneri");
   if (lpOALFnTable->alGetListeneri == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetListeneri' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetListenerf =(LPALGETLISTENERF)dlsym(openal_library,"alGetListenerf");
   if (lpOALFnTable->alGetListenerf == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetListenerf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetListener3f = (LPALGETLISTENER3F)dlsym(openal_library,"alGetListener3f");
   if (lpOALFnTable->alGetListener3f == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetListener3f' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetListenerfv = (LPALGETLISTENERFV)dlsym(openal_library,"alGetListenerfv");
   if (lpOALFnTable->alGetListenerfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetListenerfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGenSources = (LPALGENSOURCES)dlsym(openal_library,"alGenSources");
   if (lpOALFnTable->alGenSources == NULL)
   {
      Con::errorf("Failed to retrieve 'alGenSources' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDeleteSources = (LPALDELETESOURCES)dlsym(openal_library,"alDeleteSources");
   if (lpOALFnTable->alDeleteSources == NULL)
   {
      Con::errorf("Failed to retrieve 'alDeleteSources' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsSource = (LPALISSOURCE)dlsym(openal_library,"alIsSource");
   if (lpOALFnTable->alIsSource == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsSource' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcei = (LPALSOURCEI)dlsym(openal_library,"alSourcei");
   if (lpOALFnTable->alSourcei == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcei' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcef = (LPALSOURCEF)dlsym(openal_library,"alSourcef");
   if (lpOALFnTable->alSourcef == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcef' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSource3f = (LPALSOURCE3F)dlsym(openal_library,"alSource3f");
   if (lpOALFnTable->alSource3f == NULL)
   {
      Con::errorf("Failed to retrieve 'alSource3f' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcefv = (LPALSOURCEFV)dlsym(openal_library,"alSourcefv");
   if (lpOALFnTable->alSourcefv == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcefv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetSourcei = (LPALGETSOURCEI)dlsym(openal_library,"alGetSourcei");
   if (lpOALFnTable->alGetSourcei == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetSourcei' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetSourcef = (LPALGETSOURCEF)dlsym(openal_library,"alGetSourcef");
   if (lpOALFnTable->alGetSourcef == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetSourcef' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetSourcefv = (LPALGETSOURCEFV)dlsym(openal_library,"alGetSourcefv");
   if (lpOALFnTable->alGetSourcefv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetSourcefv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcePlayv = (LPALSOURCEPLAYV)dlsym(openal_library,"alSourcePlayv");
   if (lpOALFnTable->alSourcePlayv == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcePlayv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourceStopv = (LPALSOURCESTOPV)dlsym(openal_library,"alSourceStopv");
   if (lpOALFnTable->alSourceStopv == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourceStopv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcePlay = (LPALSOURCEPLAY)dlsym(openal_library,"alSourcePlay");
   if (lpOALFnTable->alSourcePlay == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcePlay' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourcePause = (LPALSOURCEPAUSE)dlsym(openal_library,"alSourcePause");
   if (lpOALFnTable->alSourcePause == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourcePause' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourceStop = (LPALSOURCESTOP)dlsym(openal_library,"alSourceStop");
   if (lpOALFnTable->alSourceStop == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourceStop' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourceRewind = (LPALSOURCEREWIND)dlsym(openal_library,"alSourceRewind");
   if (lpOALFnTable->alSourceRewind == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourceRewind' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGenBuffers = (LPALGENBUFFERS)dlsym(openal_library,"alGenBuffers");
   if (lpOALFnTable->alGenBuffers == NULL)
   {
      Con::errorf("Failed to retrieve 'alGenBuffers' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDeleteBuffers = (LPALDELETEBUFFERS)dlsym(openal_library,"alDeleteBuffers");
   if (lpOALFnTable->alDeleteBuffers == NULL)
   {
      Con::errorf("Failed to retrieve 'alDeleteBuffers' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsBuffer = (LPALISBUFFER)dlsym(openal_library,"alIsBuffer");
   if (lpOALFnTable->alIsBuffer == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsBuffer' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alBufferData = (LPALBUFFERDATA)dlsym(openal_library,"alBufferData");
   if (lpOALFnTable->alBufferData == NULL)
   {
      Con::errorf("Failed to retrieve 'alBufferData' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetBufferi = (LPALGETBUFFERI)dlsym(openal_library,"alGetBufferi");
   if (lpOALFnTable->alGetBufferi == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetBufferi' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetBufferf = (LPALGETBUFFERF)dlsym(openal_library,"alGetBufferf");
   if (lpOALFnTable->alGetBufferf == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetBufferf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)dlsym(openal_library,"alSourceQueueBuffers");
   if (lpOALFnTable->alSourceQueueBuffers == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourceQueueBuffers' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)dlsym(openal_library,"alSourceUnqueueBuffers");
   if (lpOALFnTable->alSourceUnqueueBuffers == NULL)
   {
      Con::errorf("Failed to retrieve 'alSourceUnqueueBuffers' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDistanceModel = (LPALDISTANCEMODEL)dlsym(openal_library,"alDistanceModel");
   if (lpOALFnTable->alDistanceModel == NULL)
   {
      Con::errorf("Failed to retrieve 'alDistanceModel' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDopplerFactor = (LPALDOPPLERFACTOR)dlsym(openal_library,"alDopplerFactor");
   if (lpOALFnTable->alDopplerFactor == NULL)
   {
      Con::errorf("Failed to retrieve 'alDopplerFactor' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDopplerVelocity = (LPALDOPPLERVELOCITY)dlsym(openal_library,"alDopplerVelocity");
   if (lpOALFnTable->alDopplerVelocity == NULL)
   {
      Con::errorf("Failed to retrieve 'alDopplerVelocity' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetString = (LPALCGETSTRING)dlsym(openal_library,"alcGetString");
   if (lpOALFnTable->alcGetString == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetString' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetIntegerv = (LPALCGETINTEGERV)dlsym(openal_library,"alcGetIntegerv");
   if (lpOALFnTable->alcGetIntegerv == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetIntegerv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcOpenDevice = (LPALCOPENDEVICE)dlsym(openal_library,"alcOpenDevice");
   if (lpOALFnTable->alcOpenDevice == NULL)
   {
      Con::errorf("Failed to retrieve 'alcOpenDevice' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcCloseDevice = (LPALCCLOSEDEVICE)dlsym(openal_library,"alcCloseDevice");
   if (lpOALFnTable->alcCloseDevice == NULL)
   {
      Con::errorf("Failed to retrieve 'alcCloseDevice' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcCreateContext = (LPALCCREATECONTEXT)dlsym(openal_library,"alcCreateContext");
   if (lpOALFnTable->alcCreateContext == NULL)
   {
      Con::errorf("Failed to retrieve 'alcCreateContext' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)dlsym(openal_library,"alcMakeContextCurrent");
   if (lpOALFnTable->alcMakeContextCurrent == NULL)
   {
      Con::errorf("Failed to retrieve 'alcMakeContextCurrent' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcProcessContext = (LPALCPROCESSCONTEXT)dlsym(openal_library,"alcProcessContext");
   if (lpOALFnTable->alcProcessContext == NULL)
   {
      Con::errorf("Failed to retrieve 'alcProcessContext' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetCurrentContext = (LPALCGETCURRENTCONTEXT)dlsym(openal_library,"alcGetCurrentContext");
   if (lpOALFnTable->alcGetCurrentContext == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetCurrentContext' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)dlsym(openal_library,"alcGetContextsDevice");
   if (lpOALFnTable->alcGetContextsDevice == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetContextsDevice' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcSuspendContext = (LPALCSUSPENDCONTEXT)dlsym(openal_library,"alcSuspendContext");
   if (lpOALFnTable->alcSuspendContext == NULL)
   {
      Con::errorf("Failed to retrieve 'alcSuspendContext' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcDestroyContext = (LPALCDESTROYCONTEXT)dlsym(openal_library,"alcDestroyContext");
   if (lpOALFnTable->alcDestroyContext == NULL)
   {
      Con::errorf("Failed to retrieve 'alcDestroyContext' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetError = (LPALCGETERROR)dlsym(openal_library,"alcGetError");
   if (lpOALFnTable->alcGetError == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetError' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)dlsym(openal_library,"alcIsExtensionPresent");
   if (lpOALFnTable->alcIsExtensionPresent == NULL)
   {
      Con::errorf("Failed to retrieve 'alcIsExtensionPresent' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetProcAddress = (LPALCGETPROCADDRESS)dlsym(openal_library,"alcGetProcAddress");
   if (lpOALFnTable->alcGetProcAddress == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetProcAddress' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alcGetEnumValue = (LPALCGETENUMVALUE)dlsym(openal_library,"alcGetEnumValue");
   if (lpOALFnTable->alcGetEnumValue == NULL)
   {
      Con::errorf("Failed to retrieve 'alcGetEnumValue' function address");
      return AL_FALSE;
   }
   //efx
   lpOALFnTable->alGenEffects = (LPALGENEFFECTS)dlsym(openal_library, "alGenEffects");
   if (lpOALFnTable->alGenEffects == NULL)
   {
      Con::errorf("Failed to retrieve 'alGenEffects' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alEffecti = (LPALEFFECTI)dlsym(openal_library, "alEffecti");
   if (lpOALFnTable->alEffecti == NULL)
   {
      Con::errorf("Failed to retrieve 'alEffecti' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alEffectiv = (LPALEFFECTIV)dlsym(openal_library, "alEffectiv");
   if (lpOALFnTable->alEffectiv == NULL)
   {
      Con::errorf("Failed to retrieve 'alEffectiv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alEffectf = (LPALEFFECTF)dlsym(openal_library, "alEffectf");
   if (lpOALFnTable->alEffectf == NULL)
   {
      Con::errorf("Failed to retrieve 'alEffectf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alEffectfv = (LPALEFFECTFV)dlsym(openal_library, "alEffectfv");
   if (lpOALFnTable->alEffectfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alEffectfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetEffecti = (LPALGETEFFECTI)dlsym(openal_library, "alGetEffecti");
   if (lpOALFnTable->alGetEffecti == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetEffecti' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetEffectiv = (LPALGETEFFECTIV)dlsym(openal_library, "alGetEffectiv");
   if (lpOALFnTable->alGetEffectiv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetEffectiv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetEffectf = (LPALGETEFFECTF)dlsym(openal_library, "alGetEffectf");
   if (lpOALFnTable->alGetEffectf == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetEffectf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetEffectfv = (LPALGETEFFECTFV)dlsym(openal_library, "alGetEffectfv");
   if (lpOALFnTable->alGetEffectfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetEffectfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDeleteEffects = (LPALDELETEEFFECTS)dlsym(openal_library, "alDeleteEffects");
   if (lpOALFnTable->alDeleteEffects == NULL)
   {
      Con::errorf("Failed to retrieve 'alDeleteEffects' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsEffect = (LPALISEFFECT)dlsym(openal_library, "alIsEffect");
   if (lpOALFnTable->alIsEffect == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsEffect' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)dlsym(openal_library, "alAuxiliaryEffectSlotf");
   if (lpOALFnTable->alAuxiliaryEffectSlotf == NULL)
   {
      Con::errorf("Failed to retrieve 'alAuxiliaryEffectSlotf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)dlsym(openal_library, "alAuxiliaryEffectSlotfv");
   if (lpOALFnTable->alAuxiliaryEffectSlotfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alAuxiliaryEffectSlotfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)dlsym(openal_library, "alAuxiliaryEffectSloti");
   if (lpOALFnTable->alAuxiliaryEffectSloti == NULL)
   {
      Con::errorf("Failed to retrieve 'alAuxiliaryEffectSloti' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)dlsym(openal_library, "alAuxiliaryEffectSlotiv");
   if (lpOALFnTable->alAuxiliaryEffectSlotiv == NULL)
   {
      Con::errorf("Failed to retrieve 'alAuxiliaryEffectSlotiv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)dlsym(openal_library, "alIsAuxiliaryEffectSlot");
   if (lpOALFnTable->alIsAuxiliaryEffectSlot == NULL)
   {
      Con::errorf("Failed to retrieve 'alIsAuxiliaryEffectSlot' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)dlsym(openal_library, "alGenAuxiliaryEffectSlots");
   if (lpOALFnTable->alGenAuxiliaryEffectSlots == NULL)
   {
      Con::errorf("Failed to retrieve 'alGenAuxiliaryEffectSlots' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)dlsym(openal_library, "alDeleteAuxiliaryEffectSlots");
   if (lpOALFnTable->alDeleteAuxiliaryEffectSlots == NULL)
   {
      Con::errorf("Failed to retrieve 'alDeleteAuxiliaryEffectSlots' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)dlsym(openal_library, "alGetAuxiliaryEffectSlotf");
   if (lpOALFnTable->alGetAuxiliaryEffectSlotf == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetAuxiliaryEffectSlotf' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)dlsym(openal_library, "alGetAuxiliaryEffectSlotfv");
   if (lpOALFnTable->alGetAuxiliaryEffectSlotfv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetAuxiliaryEffectSlotfv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)dlsym(openal_library, "alGetAuxiliaryEffectSloti");
   if (lpOALFnTable->alGetAuxiliaryEffectSloti == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetAuxiliaryEffectSloti' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)dlsym(openal_library, "alGetAuxiliaryEffectSlotiv");
   if (lpOALFnTable->alGetAuxiliaryEffectSlotiv == NULL)
   {
      Con::errorf("Failed to retrieve 'alGetAuxiliaryEffectSlotiv' function address");
      return AL_FALSE;
   }
   lpOALFnTable->alSource3i = (LPALSOURCE3I)dlsym(openal_library, "alSource3i");
   if (lpOALFnTable->alSource3i == NULL)
   {
      Con::errorf("Failed to retrieve 'alSource3i' function address");
      return AL_FALSE;
   }
    
   return AL_TRUE;
}

ALvoid UnloadOAL10Library()
{
   if (openal_library != NULL)
      dlclose(openal_library);
}
