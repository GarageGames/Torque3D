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

// TODO: Implement OpenAL loading code which is currently stubbed out.

#if defined(__MACOSX__) && !defined(TORQUE_OS_MAC)
#define TORQUE_OS_MAC
#endif

#include <err.h>
#include <string.h>
#include "sfx/openal/LoadOAL.h"

ALboolean LoadOAL10Library(char *szOALFullPathName, LPOPENALFNTABLE lpOALFnTable)
{
   // TODO: Implement this.
	if (!lpOALFnTable)
		return AL_FALSE;

	memset(lpOALFnTable, 0, sizeof(OPENALFNTABLE));

   lpOALFnTable->alEnable = (LPALENABLE)alEnable;
	if (lpOALFnTable->alEnable == NULL)
	{
		warn("Failed to retrieve 'alEnable' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDisable = (LPALDISABLE)alDisable;
	if (lpOALFnTable->alDisable == NULL)
	{
		warn("Failed to retrieve 'alDisable' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsEnabled = (LPALISENABLED)alIsEnabled;
	if (lpOALFnTable->alIsEnabled == NULL)
	{
		warn("Failed to retrieve 'alIsEnabled' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBoolean = (LPALGETBOOLEAN)alGetBoolean;
	if (lpOALFnTable->alGetBoolean == NULL)
	{
		warn("Failed to retrieve 'alGetBoolean' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetInteger = (LPALGETINTEGER)alGetInteger;
	if (lpOALFnTable->alGetInteger == NULL)
	{
		warn("Failed to retrieve 'alGetInteger' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetFloat = (LPALGETFLOAT)alGetFloat;
	if (lpOALFnTable->alGetFloat == NULL)
	{
		warn("Failed to retrieve 'alGetFloat' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetDouble = (LPALGETDOUBLE)alGetDouble;
	if (lpOALFnTable->alGetDouble == NULL)
	{
		warn("Failed to retrieve 'alGetDouble' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBooleanv = (LPALGETBOOLEANV)alGetBooleanv;
	if (lpOALFnTable->alGetBooleanv == NULL)
	{
		warn("Failed to retrieve 'alGetBooleanv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetIntegerv = (LPALGETINTEGERV)alGetIntegerv;
	if (lpOALFnTable->alGetIntegerv == NULL)
	{
		warn("Failed to retrieve 'alGetIntegerv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetFloatv = (LPALGETFLOATV)alGetFloatv;
	if (lpOALFnTable->alGetFloatv == NULL)
	{
		warn("Failed to retrieve 'alGetFloatv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetDoublev = (LPALGETDOUBLEV)alGetDoublev;
	if (lpOALFnTable->alGetDoublev == NULL)
	{
		warn("Failed to retrieve 'alGetDoublev' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetString = (LPALGETSTRING)alGetString;
	if (lpOALFnTable->alGetString == NULL)
	{
		warn("Failed to retrieve 'alGetString' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetError = (LPALGETERROR)alGetError;
	if (lpOALFnTable->alGetError == NULL)
	{
		warn("Failed to retrieve 'alGetError' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsExtensionPresent = (LPALISEXTENSIONPRESENT)alIsExtensionPresent;
	if (lpOALFnTable->alIsExtensionPresent == NULL)
	{
		warn("Failed to retrieve 'alIsExtensionPresent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetProcAddress = (LPALGETPROCADDRESS)alGetProcAddress;
	if (lpOALFnTable->alGetProcAddress == NULL)
	{
		warn("Failed to retrieve 'alGetProcAddress' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetEnumValue = (LPALGETENUMVALUE)alGetEnumValue;
	if (lpOALFnTable->alGetEnumValue == NULL)
	{
		warn("Failed to retrieve 'alGetEnumValue' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListeneri = (LPALLISTENERI)alListeneri;
	if (lpOALFnTable->alListeneri == NULL)
	{
		warn("Failed to retrieve 'alListeneri' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListenerf = (LPALLISTENERF)alListenerf;
	if (lpOALFnTable->alListenerf == NULL)
	{
		warn("Failed to retrieve 'alListenerf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListener3f = (LPALLISTENER3F)alListener3f;
	if (lpOALFnTable->alListener3f == NULL)
	{
		warn("Failed to retrieve 'alListener3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListenerfv = (LPALLISTENERFV)alListenerfv;
	if (lpOALFnTable->alListenerfv == NULL)
	{
		warn("Failed to retrieve 'alListenerfv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListeneri = (LPALGETLISTENERI)alGetListeneri;
	if (lpOALFnTable->alGetListeneri == NULL)
	{
		warn("Failed to retrieve 'alGetListeneri' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListenerf =(LPALGETLISTENERF)alGetListenerf;
	if (lpOALFnTable->alGetListenerf == NULL)
	{
		warn("Failed to retrieve 'alGetListenerf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListener3f = (LPALGETLISTENER3F)alGetListener3f;
	if (lpOALFnTable->alGetListener3f == NULL)
	{
		warn("Failed to retrieve 'alGetListener3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListenerfv = (LPALGETLISTENERFV)alGetListenerfv;
	if (lpOALFnTable->alGetListenerfv == NULL)
	{
		warn("Failed to retrieve 'alGetListenerfv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGenSources = (LPALGENSOURCES)alGenSources;
	if (lpOALFnTable->alGenSources == NULL)
	{
		warn("Failed to retrieve 'alGenSources' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDeleteSources = (LPALDELETESOURCES)alDeleteSources;
	if (lpOALFnTable->alDeleteSources == NULL)
	{
		warn("Failed to retrieve 'alDeleteSources' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsSource = (LPALISSOURCE)alIsSource;
	if (lpOALFnTable->alIsSource == NULL)
	{
		warn("Failed to retrieve 'alIsSource' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcei = (LPALSOURCEI)alSourcei;
	if (lpOALFnTable->alSourcei == NULL)
	{
		warn("Failed to retrieve 'alSourcei' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcef = (LPALSOURCEF)alSourcef;
	if (lpOALFnTable->alSourcef == NULL)
	{
		warn("Failed to retrieve 'alSourcef' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSource3f = (LPALSOURCE3F)alSource3f;
	if (lpOALFnTable->alSource3f == NULL)
	{
		warn("Failed to retrieve 'alSource3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcefv = (LPALSOURCEFV)alSourcefv;
	if (lpOALFnTable->alSourcefv == NULL)
	{
		warn("Failed to retrieve 'alSourcefv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcei = (LPALGETSOURCEI)alGetSourcei;
	if (lpOALFnTable->alGetSourcei == NULL)
	{
		warn("Failed to retrieve 'alGetSourcei' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcef = (LPALGETSOURCEF)alGetSourcef;
	if (lpOALFnTable->alGetSourcef == NULL)
	{
		warn("Failed to retrieve 'alGetSourcef' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcefv = (LPALGETSOURCEFV)alGetSourcefv;
	if (lpOALFnTable->alGetSourcefv == NULL)
	{
		warn("Failed to retrieve 'alGetSourcefv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePlayv = (LPALSOURCEPLAYV)alSourcePlayv;
	if (lpOALFnTable->alSourcePlayv == NULL)
	{
		warn("Failed to retrieve 'alSourcePlayv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceStopv = (LPALSOURCESTOPV)alSourceStopv;
	if (lpOALFnTable->alSourceStopv == NULL)
	{
		warn("Failed to retrieve 'alSourceStopv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePlay = (LPALSOURCEPLAY)alSourcePlay;
	if (lpOALFnTable->alSourcePlay == NULL)
	{
		warn("Failed to retrieve 'alSourcePlay' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePause = (LPALSOURCEPAUSE)alSourcePause;
	if (lpOALFnTable->alSourcePause == NULL)
	{
		warn("Failed to retrieve 'alSourcePause' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceStop = (LPALSOURCESTOP)alSourceStop;
	if (lpOALFnTable->alSourceStop == NULL)
	{
		warn("Failed to retrieve 'alSourceStop' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceRewind = (LPALSOURCEREWIND)alSourceRewind;
	if (lpOALFnTable->alSourceRewind == NULL)
	{
		warn("Failed to retrieve 'alSourceRewind' function address\n");
		return AL_FALSE;
	}
   lpOALFnTable->alGenBuffers = (LPALGENBUFFERS)alGenBuffers;
	if (lpOALFnTable->alGenBuffers == NULL)
	{
		warn("Failed to retrieve 'alGenBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDeleteBuffers = (LPALDELETEBUFFERS)alDeleteBuffers;
	if (lpOALFnTable->alDeleteBuffers == NULL)
	{
		warn("Failed to retrieve 'alDeleteBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsBuffer = (LPALISBUFFER)alIsBuffer;
	if (lpOALFnTable->alIsBuffer == NULL)
	{
		warn("Failed to retrieve 'alIsBuffer' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alBufferData = (LPALBUFFERDATA)alBufferData;
	if (lpOALFnTable->alBufferData == NULL)
	{
		warn("Failed to retrieve 'alBufferData' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBufferi = (LPALGETBUFFERI)alGetBufferi;
	if (lpOALFnTable->alGetBufferi == NULL)
	{
		warn("Failed to retrieve 'alGetBufferi' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBufferf = (LPALGETBUFFERF)alGetBufferf;
	if (lpOALFnTable->alGetBufferf == NULL)
	{
		warn("Failed to retrieve 'alGetBufferf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)alSourceQueueBuffers;
	if (lpOALFnTable->alSourceQueueBuffers == NULL)
	{
		warn("Failed to retrieve 'alSourceQueueBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)alSourceUnqueueBuffers;
	if (lpOALFnTable->alSourceUnqueueBuffers == NULL)
	{
		warn("Failed to retrieve 'alSourceUnqueueBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDistanceModel = (LPALDISTANCEMODEL)alDistanceModel;
	if (lpOALFnTable->alDistanceModel == NULL)
	{
		warn("Failed to retrieve 'alDistanceModel' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDopplerFactor = (LPALDOPPLERFACTOR)alDopplerFactor;
	if (lpOALFnTable->alDopplerFactor == NULL)
	{
		warn("Failed to retrieve 'alDopplerFactor' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDopplerVelocity = (LPALDOPPLERVELOCITY)alDopplerVelocity;
	if (lpOALFnTable->alDopplerVelocity == NULL)
	{
		warn("Failed to retrieve 'alDopplerVelocity' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetString = (LPALCGETSTRING)alcGetString;
	if (lpOALFnTable->alcGetString == NULL)
	{
		warn("Failed to retrieve 'alcGetString' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetIntegerv = (LPALCGETINTEGERV)alcGetIntegerv;
	if (lpOALFnTable->alcGetIntegerv == NULL)
	{
		warn("Failed to retrieve 'alcGetIntegerv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcOpenDevice = (LPALCOPENDEVICE)alcOpenDevice;
	if (lpOALFnTable->alcOpenDevice == NULL)
	{
		warn("Failed to retrieve 'alcOpenDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcCloseDevice = (LPALCCLOSEDEVICE)alcCloseDevice;
	if (lpOALFnTable->alcCloseDevice == NULL)
	{
		warn("Failed to retrieve 'alcCloseDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcCreateContext = (LPALCCREATECONTEXT)alcCreateContext;
	if (lpOALFnTable->alcCreateContext == NULL)
	{
		warn("Failed to retrieve 'alcCreateContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)alcMakeContextCurrent;
	if (lpOALFnTable->alcMakeContextCurrent == NULL)
	{
		warn("Failed to retrieve 'alcMakeContextCurrent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcProcessContext = (LPALCPROCESSCONTEXT)alcProcessContext;
	if (lpOALFnTable->alcProcessContext == NULL)
	{
		warn("Failed to retrieve 'alcProcessContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetCurrentContext = (LPALCGETCURRENTCONTEXT)alcGetCurrentContext;
	if (lpOALFnTable->alcGetCurrentContext == NULL)
	{
		warn("Failed to retrieve 'alcGetCurrentContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)alcGetContextsDevice;
	if (lpOALFnTable->alcGetContextsDevice == NULL)
	{
		warn("Failed to retrieve 'alcGetContextsDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcSuspendContext = (LPALCSUSPENDCONTEXT)alcSuspendContext;
	if (lpOALFnTable->alcSuspendContext == NULL)
	{
		warn("Failed to retrieve 'alcSuspendContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcDestroyContext = (LPALCDESTROYCONTEXT)alcDestroyContext;
	if (lpOALFnTable->alcDestroyContext == NULL)
	{
		warn("Failed to retrieve 'alcDestroyContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetError = (LPALCGETERROR)alcGetError;
	if (lpOALFnTable->alcGetError == NULL)
	{
		warn("Failed to retrieve 'alcGetError' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)alcIsExtensionPresent;
	if (lpOALFnTable->alcIsExtensionPresent == NULL)
	{
		warn("Failed to retrieve 'alcIsExtensionPresent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetProcAddress = (LPALCGETPROCADDRESS)alcGetProcAddress;
	if (lpOALFnTable->alcGetProcAddress == NULL)
	{
		warn("Failed to retrieve 'alcGetProcAddress' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetEnumValue = (LPALCGETENUMVALUE)alcGetEnumValue;
	if (lpOALFnTable->alcGetEnumValue == NULL)
	{
		warn("Failed to retrieve 'alcGetEnumValue' function address\n");
		return AL_FALSE;
	}
   
   
	return AL_TRUE;
}

ALvoid UnloadOAL10Library()
{
// TODO: Implement this.
}