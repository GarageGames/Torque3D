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

#include "core/strings/stringFunctions.h"

#include "aldlist.h"
#if defined(TORQUE_OS_MAC)
#include <OpenAL/alc.h>
#elif defined(TORQUE_OS_LINUX)
#include <AL/alc.h>
#else
#include <al/alc.h>
#endif

/* 
 * Init call
 */
ALDeviceList::ALDeviceList( const OPENALFNTABLE &oalft )
{
   VECTOR_SET_ASSOCIATION( vDeviceInfo );

	ALDEVICEINFO	ALDeviceInfo;
	char *devices;
	int index;
	const char *defaultDeviceName;
   const char *actualDeviceName;

   dMemcpy( &ALFunction, &oalft, sizeof( OPENALFNTABLE ) );

   // DeviceInfo vector stores, for each enumerated device, it's device name, selection status, spec version #, and extension support
   vDeviceInfo.clear();
   vDeviceInfo.reserve(10);

   defaultDeviceIndex = 0;

   // grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
   if (ALFunction.alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
      devices = (char *)ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
      defaultDeviceName = (char *)ALFunction.alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      index = 0;
      // go through device list (each device terminated with a single NULL, list terminated with double NULL)
      while (*devices != 0) {
         if (dStrcmp(defaultDeviceName, devices) == 0) {
            defaultDeviceIndex = index;
         }
         ALCdevice *device = ALFunction.alcOpenDevice(devices);
         if (device) {
            ALCcontext *context = ALFunction.alcCreateContext(device, NULL);
            if (context) {
               ALFunction.alcMakeContextCurrent(context);
               // if new actual device name isn't already in the list, then add it...
               actualDeviceName = ALFunction.alcGetString(device, ALC_DEVICE_SPECIFIER);
               bool bNewName = true;
               for (int i = 0; i < GetNumDevices(); i++) {
                  if (dStrcmp(GetDeviceName(i), actualDeviceName) == 0) {
                     bNewName = false;
                  }
               }
               if ((bNewName) && (actualDeviceName != NULL) && (dStrlen(actualDeviceName) > 0)) {
                  dMemset(&ALDeviceInfo, 0, sizeof(ALDEVICEINFO));
                  ALDeviceInfo.bSelected = true;
                  dStrncpy(ALDeviceInfo.strDeviceName, actualDeviceName, sizeof(ALDeviceInfo.strDeviceName));
                  ALFunction.alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &ALDeviceInfo.iMajorVersion);
                  ALFunction.alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &ALDeviceInfo.iMinorVersion);

                  ALDeviceInfo.iCapsFlags = 0;

                  // Check for ALC Extensions
                  if (ALFunction.alcIsExtensionPresent(device, "ALC_EXT_CAPTURE") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALCapture;
                  if (ALFunction.alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEFX;

                  // Check for AL Extensions
                  if (ALFunction.alIsExtensionPresent("AL_EXT_OFFSET") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALOffset;

                  if (ALFunction.alIsExtensionPresent("AL_EXT_LINEAR_DISTANCE") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALLinearDistance;
                  if (ALFunction.alIsExtensionPresent("AL_EXT_EXPONENT_DISTANCE") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALExponentDistance;

                  if (ALFunction.alIsExtensionPresent("EAX2.0") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEAX2;
                  if (ALFunction.alIsExtensionPresent("EAX3.0") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEAX3;
                  if (ALFunction.alIsExtensionPresent("EAX4.0") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEAX4;
                  if (ALFunction.alIsExtensionPresent("EAX5.0") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEAX5;

                  if (ALFunction.alIsExtensionPresent("EAX-RAM") == AL_TRUE)
                     ALDeviceInfo.iCapsFlags |= SFXALEAXRAM;

                  // Get Source Count
                  ALDeviceInfo.uiSourceCount = GetMaxNumSources();

                  vDeviceInfo.push_back(ALDeviceInfo);
               }
               ALFunction.alcMakeContextCurrent(NULL);
               ALFunction.alcDestroyContext(context);
            }
            ALFunction.alcCloseDevice(device);
         }
         devices += dStrlen(devices) + 1;
         index += 1;
      }
   }

	ResetFilters();
}

/* 
 * Exit call
 */
ALDeviceList::~ALDeviceList()
{
}

/*
 * Returns the number of devices in the complete device list
 */
int ALDeviceList::GetNumDevices()
{
	return (int)vDeviceInfo.size();	
}

/* 
 * Returns the device name at an index in the complete device list
 */
const char *ALDeviceList::GetDeviceName(int index)
{
	if (index < GetNumDevices())
		return vDeviceInfo[index].strDeviceName;
	else
		return NULL;
}

/*
 * Returns the major and minor version numbers for a device at a specified index in the complete list
 */
void ALDeviceList::GetDeviceVersion(int index, int *major, int *minor)
{
	if (index < GetNumDevices()) {
		if (major)
			*major = vDeviceInfo[index].iMajorVersion;
		if (minor)
			*minor = vDeviceInfo[index].iMinorVersion;
	}
	return;
}

/*
 * Returns the maximum number of Sources that can be generate on the given device
 */
U32 ALDeviceList::GetMaxNumSources(S32 index)
{
	if (index < GetNumDevices())
		return vDeviceInfo[index].uiSourceCount;
	else
		return 0;
}

/*
 * Checks if the extension is supported on the given device
 */
bool ALDeviceList::IsExtensionSupported(int index, SFXALCaps cap)
{
	bool bReturn = false;

	if (index < GetNumDevices())
      bReturn = vDeviceInfo[index].iCapsFlags & cap;

	return bReturn;
}

/*
 * returns the index of the default device in the complete device list
 */
int ALDeviceList::GetDefaultDevice()
{
	return defaultDeviceIndex;
}

/* 
 * Deselects devices which don't have the specified minimum version
 */
void ALDeviceList::FilterDevicesMinVer(S32 major, S32 minor)
{
	int dMajor, dMinor;
	for (U32 i = 0; i < vDeviceInfo.size(); i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor < major) || ((dMajor == major) && (dMinor < minor))) {
			vDeviceInfo[i].bSelected = false;
		}
	}
}

/* 
 * Deselects devices which don't have the specified maximum version
 */
void ALDeviceList::FilterDevicesMaxVer(S32 major, S32 minor)
{
	S32 dMajor, dMinor;
	for (U32 i = 0; i < vDeviceInfo.size(); i++) {
		GetDeviceVersion(i, &dMajor, &dMinor);
		if ((dMajor > major) || ((dMajor == major) && (dMinor > minor))) {
			vDeviceInfo[i].bSelected = false;
		}
	}
}

/*
 * Deselects device which don't support the given extension name
 */
void ALDeviceList::FilterDevicesExtension(SFXALCaps cap)
{
	for (U32 i = 0; i < vDeviceInfo.size(); i++)
		vDeviceInfo[i].bSelected = vDeviceInfo[i].iCapsFlags & cap;
}

/*
 * Resets all filtering, such that all devices are in the list
 */
void ALDeviceList::ResetFilters()
{
	for (S32 i = 0; i < GetNumDevices(); i++) {
		vDeviceInfo[i].bSelected = true;
	}
	filterIndex = 0;
}

/*
 * Gets index of first filtered device
 */
int ALDeviceList::GetFirstFilteredDevice()
{
	int i;

	for (i = 0; i < GetNumDevices(); i++) {
		if (vDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Gets index of next filtered device
 */
int ALDeviceList::GetNextFilteredDevice()
{
	int i;

	for (i = filterIndex; i < GetNumDevices(); i++) {
		if (vDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	filterIndex = i + 1;
	return i;
}

/*
 * Internal function to detemine max number of Sources that can be generated
 */
unsigned int ALDeviceList::GetMaxNumSources()
{
	ALuint uiSources[256];
	U32 iSourceCount = 0;

	// Clear AL Error Code
	ALFunction.alGetError();

	// Generate up to 256 Sources, checking for any errors
	for (iSourceCount = 0; iSourceCount < 256; iSourceCount++)
	{
		ALFunction.alGenSources(1, &uiSources[iSourceCount]);
		if (ALFunction.alGetError() != AL_NO_ERROR)
			break;
	}

	// Release the Sources
	ALFunction.alDeleteSources(iSourceCount, uiSources);
	if (ALFunction.alGetError() != AL_NO_ERROR)
	{
		for (U32 i = 0; i < 256; i++)
		{
			ALFunction.alDeleteSources(1, &uiSources[i]);
		}
	}

	return iSourceCount;
}