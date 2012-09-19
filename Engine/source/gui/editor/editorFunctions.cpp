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
#include "gui/editor/editorFunctions.h"

#include "console/simObject.h"


bool validateObjectName( const char *data, const SimObject *object )
{
   if( !data || !data[ 0 ] )
      return true;

   bool isValidId = true;
   if( !dIsalpha( data[ 0 ] ) && data[ 0 ] != '_' )
      isValidId = false;
   else
   {
      for( U32 i = 1; data[ i ] != '\0'; ++ i )
      {
         if( !dIsalnum( data[ i ] ) && data[ i ] != '_' )
         {
            isValidId = false;
            break;
         }
      }
   }

   if( !isValidId )
   {
      Platform::AlertOK( "Invalid Object Name", avar( "'%s' is not a valid "
         "object name.  Please choose a name that begins with a letter or "
         "underscore and is otherwise comprised exclusively of letters, "
         "digits, and/or underscores.", data ) );
      return false;
   }

   SimObject *pTemp = NULL;
   if ( Sim::findObject( data, pTemp ) && pTemp != object )
   {
      const char* filename = pTemp->getFilename();
      if ( !filename || !filename[0] )
         filename = "an unknown file";

      Platform::AlertOK( "Invalid Object Name", avar( "Object names must be unique, "
         "and there is an existing %s object with the name '%s' (defined in %s).  "
         "Please choose another name.", pTemp->getClassName(), data, filename ) );
      return false;
   }

   if ( AbstractClassRep::findClassRep( data ) )
   {
      Platform::AlertOK( "Invalid Object Name", avar( "'%s' is the name of an "
         "existing TorqueScript class.  Please choose another name.", data ) );
      return false;
   }

   return true;
}