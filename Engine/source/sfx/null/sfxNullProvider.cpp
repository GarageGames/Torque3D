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

#include "sfx/sfxProvider.h"
#include "sfx/null/sfxNullDevice.h"
#include "core/strings/stringFunctions.h"
#include "core/module.h"


class SFXNullProvider : public SFXProvider
{
public:

   SFXNullProvider()
      : SFXProvider( "Null" ) {}
   virtual ~SFXNullProvider();

protected:
   void addDeviceDesc( const String& name, const String& desc );
   void init();

public:

   SFXDevice* createDevice( const String& deviceName, bool useHardware, S32 maxBuffers );

};

MODULE_BEGIN( SFXNull )

   MODULE_INIT_BEFORE( SFX )
   MODULE_SHUTDOWN_AFTER( SFX )
   
   SFXNullProvider* mProvider;

   MODULE_INIT
   {
      mProvider = new SFXNullProvider;
   }
   
   MODULE_SHUTDOWN
   {
      delete mProvider;
   }

MODULE_END;

void SFXNullProvider::init()
{
   regProvider( this );
   addDeviceDesc( "Null", "SFX Null Device" );
}

SFXNullProvider::~SFXNullProvider()
{
}


void SFXNullProvider::addDeviceDesc( const String& name, const String& desc )
{
   SFXDeviceInfo* info = new SFXDeviceInfo;
   info->name = desc;
   info->driver = name;
   info->hasHardware = false;
   info->maxBuffers = 8;

   mDeviceInfo.push_back( info );
}

SFXDevice* SFXNullProvider::createDevice( const String& deviceName, bool useHardware, S32 maxBuffers )
{
   SFXDeviceInfo* info = _findDeviceInfo( deviceName );

   // Do we find one to create?
   if ( info )
      return new SFXNullDevice( this, info->name, useHardware, maxBuffers );

   return NULL;
}
