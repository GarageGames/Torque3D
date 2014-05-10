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

#ifndef ALDEVICELIST_H
#define ALDEVICELIST_H

#pragma warning(disable: 4786)  //disable warning "identifier was truncated to '255' characters in the browser information"
#include "core/util/tVector.h"
#include "core/stringTable.h"
#include "sfx/openal/sfxALCaps.h"
#include "LoadOAL.h"

typedef struct
{
	char           strDeviceName[256];
	S32				iMajorVersion;
	S32				iMinorVersion;
   U32	uiSourceCount;
	S32 iCapsFlags;
	bool			bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

class ALDeviceList
{
private:
	OPENALFNTABLE	ALFunction;
	Vector<ALDEVICEINFO> vDeviceInfo;
	S32 defaultDeviceIndex;
	S32 filterIndex;

public:
	ALDeviceList ( const OPENALFNTABLE &oalft );
	~ALDeviceList ();
	S32 GetNumDevices();
	const char *GetDeviceName(S32 index);
	void GetDeviceVersion(S32 index, S32 *major, S32 *minor);
   U32 GetMaxNumSources(S32 index);
	bool IsExtensionSupported(S32 index, SFXALCaps caps);
	S32 GetDefaultDevice();
	void FilterDevicesMinVer(S32 major, S32 minor);
	void FilterDevicesMaxVer(S32 major, S32 minor);
	void FilterDevicesExtension(SFXALCaps caps);
	void ResetFilters();
	S32 GetFirstFilteredDevice();
	S32 GetNextFilteredDevice();

private:
	U32 GetMaxNumSources();
};

#endif // ALDEVICELIST_H
