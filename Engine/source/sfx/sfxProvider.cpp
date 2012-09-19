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

#include "core/strings/stringFunctions.h"
#include "sfx/sfxProvider.h"

SFXProvider* SFXProvider::smProviders = NULL;
Vector<SFXProvider*> SFXProvider::sAllProviders( __FILE__, __LINE__ );

SFXProvider* SFXProvider::findProvider( String providerName )
{
   if( providerName.isEmpty() )
      return NULL;

   SFXProvider* curr = smProviders;
   for ( ; curr != NULL; curr = curr->mNextProvider )
   {
      if( curr->getName().equal( providerName, String::NoCase ) )
         return curr;
   }

   return NULL;
}

void SFXProvider::regProvider( SFXProvider* provider )
{
   AssertFatal( provider, "Got null provider!" );
   AssertFatal( findProvider( provider->getName() ) == NULL, "Can't register provider twice!" );
   AssertFatal( provider->mNextProvider == NULL, "Can't register provider twice!" );

   SFXProvider* oldHead = smProviders;
   smProviders = provider;
   provider->mNextProvider = oldHead;
}

SFXProvider::SFXProvider( const String& name )
   :  mName( name ),
      mNextProvider( NULL )
{
   VECTOR_SET_ASSOCIATION( mDeviceInfo );

   sAllProviders.push_back( this );
}

void SFXProvider::initializeAllProviders()
{

   for (U32 i = 0; i < sAllProviders.size(); i++)
      sAllProviders[i]->init();

}

SFXProvider::~SFXProvider()
{
   SFXDeviceInfoVector::iterator iter = mDeviceInfo.begin();
   for ( ; iter != mDeviceInfo.end(); iter++ )
      delete *iter;
}

SFXDeviceInfo* SFXProvider::_findDeviceInfo( const String& deviceName )
{
   SFXDeviceInfoVector::iterator iter = mDeviceInfo.begin();
   for ( ; iter != mDeviceInfo.end(); iter++ )
   {
      if( deviceName.equal( ( *iter )->name, String::NoCase ) )
         return *iter;
   }

   // If not found and deviceName is empty,
   // return first (default) device.

   if( deviceName.isEmpty() && mDeviceInfo.size() > 0 )
      return mDeviceInfo[ 0 ];

   return NULL;
}
