//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#include "gfx/D3D11/gfxD3D11CardProfiler.h"
#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "platformWin32/videoInfo/wmiVideoInfo.h"
#include "console/console.h"
#include "gfx/primBuilder.h"


GFXD3D11CardProfiler::GFXD3D11CardProfiler() : GFXCardProfiler()
{
}

GFXD3D11CardProfiler::~GFXD3D11CardProfiler()
{

}

void GFXD3D11CardProfiler::init()
{
   U32 adapterIndex = D3D11->getAdaterIndex();
   WMIVideoInfo wmiVidInfo;
   if (wmiVidInfo.profileAdapters())
   {
      const PlatformVideoInfo::PVIAdapter &adapter = wmiVidInfo.getAdapterInformation(adapterIndex);

      mCardDescription = adapter.description;
      mChipSet = adapter.chipSet;
      mVersionString = _getFeatureLevelStr();
      mVideoMemory = adapter.vram;
   }
   Parent::init();
}

String GFXD3D11CardProfiler::_getFeatureLevelStr()
{
   switch (D3D11->getFeatureLevel())
   {
   case D3D_FEATURE_LEVEL_11_0:
      return String("Feature level 11.0");
   case D3D_FEATURE_LEVEL_10_1:
      return String("Feature level 10.1");
   case D3D_FEATURE_LEVEL_10_0:
      return String("Feature level 10.0");
   default:
      return String("Unknown feature level");
   }
}

void GFXD3D11CardProfiler::setupCardCapabilities()
{
   setCapability("maxTextureWidth", D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
   setCapability("maxTextureHeight", D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
   setCapability("maxTextureSize", D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
}

bool GFXD3D11CardProfiler::_queryCardCap(const String &query, U32 &foundResult)
{
	return false;
}

bool GFXD3D11CardProfiler::_queryFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips )
{
   // D3D11 feature level should guarantee that any format is valid!
   return GFXD3D11TextureFormat[fmt] != DXGI_FORMAT_UNKNOWN;
}