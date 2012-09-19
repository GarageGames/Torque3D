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

#include "platform/platform.h"
#include "materials/baseMatInstance.h"

#include "core/util/safeDelete.h"


BaseMatInstance::~BaseMatInstance()
{
   deleteAllHooks();
}

void BaseMatInstance::addHook( MatInstanceHook *hook )
{
   AssertFatal( hook, "BaseMatInstance::addHook() - Got null hook!" );

   const MatInstanceHookType &type = hook->getType();

   while ( mHooks.size() <= type )
      mHooks.push_back( NULL );

   delete mHooks[type];
   mHooks[type] = hook;
}

MatInstanceHook* BaseMatInstance::getHook( const MatInstanceHookType &type ) const
{
   if ( type >= mHooks.size() )
      return NULL;

   return mHooks[ type ];
}

void BaseMatInstance::deleteHook( const MatInstanceHookType &type )
{
   if ( type >= mHooks.size() )
      return;

   delete mHooks[ type ];
   mHooks[ type ] = NULL;
}

U32 BaseMatInstance::deleteAllHooks()
{
   U32 deleteCount = 0;

   Vector<MatInstanceHook*>::iterator iter = mHooks.begin();
   for ( ; iter != mHooks.end(); iter++ )
   {
      if ( *iter )
      {
         delete (*iter);
         (*iter) = NULL;
         ++deleteCount;
      }
   }

   return deleteCount;
}