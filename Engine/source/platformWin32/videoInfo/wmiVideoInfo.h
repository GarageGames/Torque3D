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

#ifndef _WMI_CARDINFO_H_
#define _WMI_CARDINFO_H_

#include "platform/platformVideoInfo.h"

struct IWbemLocator;
struct IWbemServices;

struct IDXGIFactory;
struct IDxDiagProvider;

class WMIVideoInfo : public PlatformVideoInfo
{
private:
   IWbemLocator *mLocator;
   IWbemServices *mServices;
   bool mComInitialized;

   void*             mDXGIModule;
   IDXGIFactory*     mDXGIFactory;
   IDxDiagProvider*  mDxDiagProvider;

   bool _initializeDXGI();
   bool _initializeDxDiag();
   bool _initializeWMI();

   bool _queryPropertyDXGI( const PVIQueryType queryType, const U32 adapterId, String *outValue );
   bool _queryPropertyDxDiag( const PVIQueryType queryType, const U32 adapterId, String *outValue );
   bool _queryPropertyWMI( const PVIQueryType queryType, const U32 adapterId, String *outValue );

protected:
   static WCHAR *smPVIQueryTypeToWMIString [];
   bool _queryProperty( const PVIQueryType queryType, const U32 adapterId, String *outValue );
   bool _initialize();

public:
   WMIVideoInfo();
   ~WMIVideoInfo();
};

#endif