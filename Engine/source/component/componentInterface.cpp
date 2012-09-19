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

#include "component/simComponent.h"
#include "component/componentInterface.h"
#include "core/strings/findMatch.h"
#include "core/stringTable.h"

bool ComponentInterfaceCache::add( const char *type, const char *name, const SimComponent *owner, ComponentInterface *cinterface )
{
   if( ( mInterfaceList.size() == 0 ) || ( enumerate( NULL, type, name, owner ) == 0 ) )
   {
      mInterfaceList.increment();
      // CodeReview [tom, 3/9/2007] Seems silly to keep calling last(), why not cache the var? Yes, I know I am pedantic.
      mInterfaceList.last().type = ( type == NULL ? NULL : StringTable->insert( type ) );
      mInterfaceList.last().name = ( name == NULL ? NULL : StringTable->insert( name ) );
      mInterfaceList.last().owner = owner;
      mInterfaceList.last().iface = cinterface;

      return true;
   }

   return false;
}

//------------------------------------------------------------------------------

void ComponentInterfaceCache::clear()
{
   mInterfaceList.clear();
}

//------------------------------------------------------------------------------

U32 ComponentInterfaceCache::enumerate( ComponentInterfaceList *list, const char *type /* = NULL */, 
                                       const char *name /* = NULL  */, const SimComponent *owner /* = NULL */, bool notOwner /* = false */ ) const
{
   U32 numMatches = 0;

   for( _InterfaceEntryItr i = mInterfaceList.begin(); i != mInterfaceList.end(); i++ )
   {
      // Early out if limiting results by component owner
      if( owner != NULL && (
         ( (*i).owner == owner && notOwner ) ||
         ( (*i).owner != owner && !notOwner ) ) )
         continue;

      // Match the type, short circuit if type == NULL
      if( type == NULL || FindMatch::isMatch( type, (*i).type ) )
      {
         // Match the name
         if( name == NULL || FindMatch::isMatch( name, (*i).name ) )
         {
            numMatches++;

            if( list != NULL )
               list->push_back( (*i).iface );
         }
      }
   }

   return numMatches;
}