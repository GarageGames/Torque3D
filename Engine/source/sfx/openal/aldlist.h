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
	int				iMajorVersion;
	int				iMinorVersion;
	unsigned int	uiSourceCount;
	int iCapsFlags;
	bool			bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

class ALDeviceList
{
private:
	OPENALFNTABLE	ALFunction;
	Vector<ALDEVICEINFO> vDeviceInfo;
	int defaultDeviceIndex;
	int filterIndex;

public:
	ALDeviceList ( const OPENALFNTABLE &oalft );
	~ALDeviceList ();
	int GetNumDevices();
	const char *GetDeviceName(int index);
	void GetDeviceVersion(int index, int *major, int *minor);
	unsigned int GetMaxNumSources(int index);
	bool IsExtensionSupported(int index, SFXALCaps caps);
	int GetDefaultDevice();
	void FilterDevicesMinVer(int major, int minor);
	void FilterDevicesMaxVer(int major, int minor);
	void FilterDevicesExtension(SFXALCaps caps);
	void ResetFilters();
	int GetFirstFilteredDevice();
	int GetNextFilteredDevice();

private:
	unsigned int GetMaxNumSources();
};

#endif // ALDEVICELIST_H
