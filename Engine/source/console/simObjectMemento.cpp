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
#include "console/simObjectMemento.h"

#include "console/simObject.h"
#include "console/simDatablock.h"
#include "core/stream/memStream.h"


SimObjectMemento::SimObjectMemento()
   : mState( NULL ),
      mIsDatablock( false )
{
}

SimObjectMemento::~SimObjectMemento()
{
   dFree( mState );
}

void SimObjectMemento::save( SimObject *object )
{
   // Cleanup any existing state data.
   dFree( mState );
   mObjectName = String::EmptyString;

   // Use a stream to save the state.
   MemStream stream( 256 );

   U32 writeFlags = 0;
   SimDataBlock* db = dynamic_cast<SimDataBlock*>(object);
   if( !db )
      stream.write( sizeof( "return " ) - 1, "return " );
   else
   {
      mIsDatablock = true;

      // Cull the datablock name from the output so that
      // we can easily replace it in case the datablock's name
      // is already taken when we call restore().  We can't use the same
      // setup as with non-datablock classes as the return semantics
      // are not the same.

      writeFlags |= SimObject::NoName;
   }
   
   object->write( stream, 0, writeFlags );
   stream.write( (UTF8)0 );

   // Steal the data away from the stream.
   mState = (UTF8*)stream.takeBuffer();
   mObjectName = object->getName();
}

SimObject *SimObjectMemento::restore() const
{
   // Make sure we have data to restore.
   if ( !mState )
      return NULL;

   // TODO: We could potentially make this faster by
   // caching the CodeBlock generated from the string

   SimObject* object;
   if( !mIsDatablock )
   {
      // Set the redefine behavior to automatically giving
      // the new objects unique names.  This will restore the
      // old names if they are still available or give reasonable
      // approximations if not.

      const char* oldRedefineBehavior = Con::getVariable( "$Con::redefineBehavior" );
      Con::setVariable( "$Con::redefineBehavior", "renameNew" );

      // Read the object.

      const UTF8* result = Con::evaluate( mState );

      // Restore the redefine behavior.

      Con::setVariable( "$Con::redefineBehavior", oldRedefineBehavior );

      if ( !result || !result[ 0 ] )
         return NULL;

      // Look up the object.

      U32 objectId = dAtoi( result );
      object = Sim::findObject( objectId );
   }
   else
   {
      String objectName = mObjectName;

      // For datablocks, it's getting a little complicated.  Datablock definitions cannot be used
      // as expressions and thus we can't get to the datablock object we create by using the
      // Con::evaluate() return value.  Instead, we need to rely on the object name.  However, if
      // the name is already taken and needs to be changed, we need to manually do that.  To complicate
      // this further, we cannot rely on automatic renaming since then we don't know by what name
      // the newly created object actually goes.  So what we do is we alter the source text snapshot
      // and substitute a name in case the old object name is actually taken now.

      char* tempBuffer;
      if( !Sim::findObject( objectName ) )
         tempBuffer = mState;
      else
      {
         String uniqueName = Sim::getUniqueName( objectName );
         U32 uniqueNameLen = uniqueName.length();

         char* pLeftParen = dStrchr( mState, '(' );
         if( pLeftParen == NULL )
            return NULL;
         U32 numCharsToLeftParen = pLeftParen - mState;

         tempBuffer = ( char* ) dMalloc( dStrlen( mState ) + uniqueNameLen + 1 );
         dMemcpy( tempBuffer, mState, numCharsToLeftParen );
         dMemcpy( &tempBuffer[ numCharsToLeftParen ], uniqueName, uniqueNameLen );
         dStrcpy( &tempBuffer[ numCharsToLeftParen + uniqueNameLen ], &mState[ numCharsToLeftParen ] );
      }

      Con::evaluate( tempBuffer );

      if( tempBuffer != mState )
         dFree( tempBuffer );

      if( objectName == String::EmptyString )
         return NULL;

      object = Sim::findObject( objectName );
   }

   return object;
}
