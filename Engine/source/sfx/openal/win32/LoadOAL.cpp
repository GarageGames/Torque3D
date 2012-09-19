/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>
#include "sfx/openal/LoadOAL.h"

HINSTANCE g_hOpenALDLL = NULL;

ALboolean LoadOAL10Library(char *szOALFullPathName, LPOPENALFNTABLE lpOALFnTable)
{
	if (!lpOALFnTable)
		return AL_FALSE;

	if (szOALFullPathName)
		g_hOpenALDLL = LoadLibraryA(szOALFullPathName);
	else
		g_hOpenALDLL = LoadLibraryA("openal32.dll");
	
	if (!g_hOpenALDLL)
		return AL_FALSE;

	memset(lpOALFnTable, 0, sizeof(OPENALFNTABLE));

	// Get function pointers
	lpOALFnTable->alEnable = (LPALENABLE)GetProcAddress(g_hOpenALDLL, "alEnable");
	if (lpOALFnTable->alEnable == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alEnable' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDisable = (LPALDISABLE)GetProcAddress(g_hOpenALDLL, "alDisable");
	if (lpOALFnTable->alDisable == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDisable' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsEnabled = (LPALISENABLED)GetProcAddress(g_hOpenALDLL, "alIsEnabled");
	if (lpOALFnTable->alIsEnabled == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alIsEnabled' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBoolean = (LPALGETBOOLEAN)GetProcAddress(g_hOpenALDLL, "alGetBoolean");
	if (lpOALFnTable->alGetBoolean == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetBoolean' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetInteger = (LPALGETINTEGER)GetProcAddress(g_hOpenALDLL, "alGetInteger");
	if (lpOALFnTable->alGetInteger == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetInteger' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetFloat = (LPALGETFLOAT)GetProcAddress(g_hOpenALDLL, "alGetFloat");
	if (lpOALFnTable->alGetFloat == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetFloat' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetDouble = (LPALGETDOUBLE)GetProcAddress(g_hOpenALDLL, "alGetDouble");
	if (lpOALFnTable->alGetDouble == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetDouble' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBooleanv = (LPALGETBOOLEANV)GetProcAddress(g_hOpenALDLL, "alGetBooleanv");
	if (lpOALFnTable->alGetBooleanv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetBooleanv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetIntegerv = (LPALGETINTEGERV)GetProcAddress(g_hOpenALDLL, "alGetIntegerv");
	if (lpOALFnTable->alGetIntegerv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetIntegerv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetFloatv = (LPALGETFLOATV)GetProcAddress(g_hOpenALDLL, "alGetFloatv");
	if (lpOALFnTable->alGetFloatv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetFloatv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetDoublev = (LPALGETDOUBLEV)GetProcAddress(g_hOpenALDLL, "alGetDoublev");
	if (lpOALFnTable->alGetDoublev == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetDoublev' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetString = (LPALGETSTRING)GetProcAddress(g_hOpenALDLL, "alGetString");
	if (lpOALFnTable->alGetString == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetString' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetError = (LPALGETERROR)GetProcAddress(g_hOpenALDLL, "alGetError");
	if (lpOALFnTable->alGetError == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetError' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsExtensionPresent = (LPALISEXTENSIONPRESENT)GetProcAddress(g_hOpenALDLL, "alIsExtensionPresent");
	if (lpOALFnTable->alIsExtensionPresent == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alIsExtensionPresent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetProcAddress = (LPALGETPROCADDRESS)GetProcAddress(g_hOpenALDLL, "alGetProcAddress");
	if (lpOALFnTable->alGetProcAddress == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetProcAddress' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetEnumValue = (LPALGETENUMVALUE)GetProcAddress(g_hOpenALDLL, "alGetEnumValue");
	if (lpOALFnTable->alGetEnumValue == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetEnumValue' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListeneri = (LPALLISTENERI)GetProcAddress(g_hOpenALDLL, "alListeneri");
	if (lpOALFnTable->alListeneri == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alListeneri' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListenerf = (LPALLISTENERF)GetProcAddress(g_hOpenALDLL, "alListenerf");
	if (lpOALFnTable->alListenerf == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alListenerf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListener3f = (LPALLISTENER3F)GetProcAddress(g_hOpenALDLL, "alListener3f");
	if (lpOALFnTable->alListener3f == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alListener3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alListenerfv = (LPALLISTENERFV)GetProcAddress(g_hOpenALDLL, "alListenerfv");
	if (lpOALFnTable->alListenerfv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alListenerfv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListeneri = (LPALGETLISTENERI)GetProcAddress(g_hOpenALDLL, "alGetListeneri");
	if (lpOALFnTable->alGetListeneri == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetListeneri' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListenerf =(LPALGETLISTENERF)GetProcAddress(g_hOpenALDLL, "alGetListenerf");
	if (lpOALFnTable->alGetListenerf == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetListenerf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListener3f = (LPALGETLISTENER3F)GetProcAddress(g_hOpenALDLL, "alGetListener3f");
	if (lpOALFnTable->alGetListener3f == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetListener3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetListenerfv = (LPALGETLISTENERFV)GetProcAddress(g_hOpenALDLL, "alGetListenerfv");
	if (lpOALFnTable->alGetListenerfv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetListenerfv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGenSources = (LPALGENSOURCES)GetProcAddress(g_hOpenALDLL, "alGenSources");
	if (lpOALFnTable->alGenSources == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGenSources' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDeleteSources = (LPALDELETESOURCES)GetProcAddress(g_hOpenALDLL, "alDeleteSources");
	if (lpOALFnTable->alDeleteSources == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDeleteSources' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsSource = (LPALISSOURCE)GetProcAddress(g_hOpenALDLL, "alIsSource");
	if (lpOALFnTable->alIsSource == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alIsSource' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcei = (LPALSOURCEI)GetProcAddress(g_hOpenALDLL, "alSourcei");
	if (lpOALFnTable->alSourcei == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcei' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcef = (LPALSOURCEF)GetProcAddress(g_hOpenALDLL, "alSourcef");
	if (lpOALFnTable->alSourcef == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcef' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSource3f = (LPALSOURCE3F)GetProcAddress(g_hOpenALDLL, "alSource3f");
	if (lpOALFnTable->alSource3f == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSource3f' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcefv = (LPALSOURCEFV)GetProcAddress(g_hOpenALDLL, "alSourcefv");
	if (lpOALFnTable->alSourcefv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcefv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcei = (LPALGETSOURCEI)GetProcAddress(g_hOpenALDLL, "alGetSourcei");
	if (lpOALFnTable->alGetSourcei == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetSourcei' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcef = (LPALGETSOURCEF)GetProcAddress(g_hOpenALDLL, "alGetSourcef");
	if (lpOALFnTable->alGetSourcef == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetSourcef' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetSourcefv = (LPALGETSOURCEFV)GetProcAddress(g_hOpenALDLL, "alGetSourcefv");
	if (lpOALFnTable->alGetSourcefv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetSourcefv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePlayv = (LPALSOURCEPLAYV)GetProcAddress(g_hOpenALDLL, "alSourcePlayv");
	if (lpOALFnTable->alSourcePlayv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcePlayv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceStopv = (LPALSOURCESTOPV)GetProcAddress(g_hOpenALDLL, "alSourceStopv");
	if (lpOALFnTable->alSourceStopv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourceStopv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePlay = (LPALSOURCEPLAY)GetProcAddress(g_hOpenALDLL, "alSourcePlay");
	if (lpOALFnTable->alSourcePlay == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcePlay' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourcePause = (LPALSOURCEPAUSE)GetProcAddress(g_hOpenALDLL, "alSourcePause");
	if (lpOALFnTable->alSourcePause == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourcePause' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceStop = (LPALSOURCESTOP)GetProcAddress(g_hOpenALDLL, "alSourceStop");
	if (lpOALFnTable->alSourceStop == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourceStop' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceRewind = (LPALSOURCEREWIND)GetProcAddress(g_hOpenALDLL, "alSourceRewind");
	if (lpOALFnTable->alSourceRewind == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourceRewind' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGenBuffers = (LPALGENBUFFERS)GetProcAddress(g_hOpenALDLL, "alGenBuffers");
	if (lpOALFnTable->alGenBuffers == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGenBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDeleteBuffers = (LPALDELETEBUFFERS)GetProcAddress(g_hOpenALDLL, "alDeleteBuffers");
	if (lpOALFnTable->alDeleteBuffers == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDeleteBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alIsBuffer = (LPALISBUFFER)GetProcAddress(g_hOpenALDLL, "alIsBuffer");
	if (lpOALFnTable->alIsBuffer == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alIsBuffer' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alBufferData = (LPALBUFFERDATA)GetProcAddress(g_hOpenALDLL, "alBufferData");
	if (lpOALFnTable->alBufferData == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alBufferData' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBufferi = (LPALGETBUFFERI)GetProcAddress(g_hOpenALDLL, "alGetBufferi");
	if (lpOALFnTable->alGetBufferi == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetBufferi' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alGetBufferf = (LPALGETBUFFERF)GetProcAddress(g_hOpenALDLL, "alGetBufferf");
	if (lpOALFnTable->alGetBufferf == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alGetBufferf' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)GetProcAddress(g_hOpenALDLL, "alSourceQueueBuffers");
	if (lpOALFnTable->alSourceQueueBuffers == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourceQueueBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)GetProcAddress(g_hOpenALDLL, "alSourceUnqueueBuffers");
	if (lpOALFnTable->alSourceUnqueueBuffers == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alSourceUnqueueBuffers' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDistanceModel = (LPALDISTANCEMODEL)GetProcAddress(g_hOpenALDLL, "alDistanceModel");
	if (lpOALFnTable->alDistanceModel == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDistanceModel' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDopplerFactor = (LPALDOPPLERFACTOR)GetProcAddress(g_hOpenALDLL, "alDopplerFactor");
	if (lpOALFnTable->alDopplerFactor == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDopplerFactor' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alDopplerVelocity = (LPALDOPPLERVELOCITY)GetProcAddress(g_hOpenALDLL, "alDopplerVelocity");
	if (lpOALFnTable->alDopplerVelocity == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alDopplerVelocity' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetString = (LPALCGETSTRING)GetProcAddress(g_hOpenALDLL, "alcGetString");
	if (lpOALFnTable->alcGetString == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetString' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetIntegerv = (LPALCGETINTEGERV)GetProcAddress(g_hOpenALDLL, "alcGetIntegerv");
	if (lpOALFnTable->alcGetIntegerv == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetIntegerv' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcOpenDevice = (LPALCOPENDEVICE)GetProcAddress(g_hOpenALDLL, "alcOpenDevice");
	if (lpOALFnTable->alcOpenDevice == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcOpenDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcCloseDevice = (LPALCCLOSEDEVICE)GetProcAddress(g_hOpenALDLL, "alcCloseDevice");
	if (lpOALFnTable->alcCloseDevice == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcCloseDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcCreateContext = (LPALCCREATECONTEXT)GetProcAddress(g_hOpenALDLL, "alcCreateContext");
	if (lpOALFnTable->alcCreateContext == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcCreateContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)GetProcAddress(g_hOpenALDLL, "alcMakeContextCurrent");
	if (lpOALFnTable->alcMakeContextCurrent == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcMakeContextCurrent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcProcessContext = (LPALCPROCESSCONTEXT)GetProcAddress(g_hOpenALDLL, "alcProcessContext");
	if (lpOALFnTable->alcProcessContext == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcProcessContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetCurrentContext = (LPALCGETCURRENTCONTEXT)GetProcAddress(g_hOpenALDLL, "alcGetCurrentContext");
	if (lpOALFnTable->alcGetCurrentContext == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetCurrentContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)GetProcAddress(g_hOpenALDLL, "alcGetContextsDevice");
	if (lpOALFnTable->alcGetContextsDevice == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetContextsDevice' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcSuspendContext = (LPALCSUSPENDCONTEXT)GetProcAddress(g_hOpenALDLL, "alcSuspendContext");
	if (lpOALFnTable->alcSuspendContext == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcSuspendContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcDestroyContext = (LPALCDESTROYCONTEXT)GetProcAddress(g_hOpenALDLL, "alcDestroyContext");
	if (lpOALFnTable->alcDestroyContext == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcDestroyContext' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetError = (LPALCGETERROR)GetProcAddress(g_hOpenALDLL, "alcGetError");
	if (lpOALFnTable->alcGetError == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetError' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)GetProcAddress(g_hOpenALDLL, "alcIsExtensionPresent");
	if (lpOALFnTable->alcIsExtensionPresent == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcIsExtensionPresent' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetProcAddress = (LPALCGETPROCADDRESS)GetProcAddress(g_hOpenALDLL, "alcGetProcAddress");
	if (lpOALFnTable->alcGetProcAddress == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetProcAddress' function address\n");
		return AL_FALSE;
	}
	lpOALFnTable->alcGetEnumValue = (LPALCGETENUMVALUE)GetProcAddress(g_hOpenALDLL, "alcGetEnumValue");
	if (lpOALFnTable->alcGetEnumValue == NULL)
	{
		OutputDebugStringA("Failed to retrieve 'alcGetEnumValue' function address\n");
		return AL_FALSE;
	}

	return AL_TRUE;
}

ALvoid UnloadOAL10Library()
{
	// Unload the dll
	if (g_hOpenALDLL)
	{
		FreeLibrary(g_hOpenALDLL);
		g_hOpenALDLL = NULL;
	}
}