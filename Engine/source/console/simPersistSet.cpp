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

#include "console/simPersistSet.h"
#include "console/simPersistID.h"
#include "console/consoleTypes.h"


IMPLEMENT_CONOBJECT( SimPersistSet );

ConsoleDocClass( SimPersistSet,
				"@brief A SimSet that can be safely persisted.\n\n"
				"Uses SimPersistIDs to reference objects in the set "
				"while persisted on disk.  This allows the set to resolve "
				"its references no matter whether they are loaded before or "
				"after the set is created.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

//-----------------------------------------------------------------------------

SimPersistSet::SimPersistSet()
   :  mIsResolvingPIDs( false )
{
   VECTOR_SET_ASSOCIATION( mUnresolvedPIDs );
}

//-----------------------------------------------------------------------------

bool SimPersistSet::processArguments( S32 argc, const char** argv )
{
   for( U32 i = 0; i < argc; ++ i )
   {
      // Parse the UUID.
      Torque::UUID uuid;
      if( !uuid.fromString( argv[ i ] ) )
      {
         Con::errorf( "SimPersistSet::processArguments - could not read UUID at index %i: %s", i, argv[ i ] );
         continue;
      }

      // Find or create the respective persistent ID.
      SimPersistID* pid = SimPersistID::findOrCreate( uuid );
      if( pid->getObject() )
      {
         // There's already an object attached to this PID so just
         // add the object to the set.
         addObject( pid->getObject() );
      }
      else
      {
         // There not yet an object attached to the PID so push it
         // onto the stack to resolve it later.
         pid->incRefCount();
         mUnresolvedPIDs.push_back( pid );
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

void SimPersistSet::write( Stream& stream, U32 tabStop, U32 flags )
{
   if( ( flags & SelectedOnly ) && !isSelected() )
      return;
      
   // If the selection is transient, we cannot really save it.
   // Just invoke the default SimObject::write and return.
      
   if( !getCanSave() )
   {
      Con::errorf( "SimPersistSet::write - transient set being saved: %d:%s (%s)",
         getId(), getClassName(), getName() );
      Parent::write( stream, tabStop, flags );
      return;
   }
   
   // If there are unresolved PIDs, give resolving them one last
   // chance before writing out the set.
   
   if( !mUnresolvedPIDs.empty() )
      resolvePIDs();
   
   // Write the set out.

   stream.writeTabs( tabStop );
   
   StringBuilder buffer;
   buffer.format( "new %s(%s", getClassName(), getName() ? getName() : "" );
   
   // Write the persistent IDs of all child objects into the set's
   // object constructor so we see them passed back to us through
   // processArguments when the object gets read in.
   
   const U32 numChildren = size();
   for( U32 i = 0; i < numChildren; ++ i )
   {
      SimObject* child = at( i );
      
      SimPersistID* pid = child->getPersistentId();
      AssertWarn( pid != NULL, "SimPersistSet::write - object without pid in persistent selection!" );
      if( !pid )
         continue;
         
      buffer.append( ',' );
      buffer.append( '"' );
      buffer.append( pid->getUUID().toString() );
      buffer.append( '"' );
   }
   
   buffer.append( ") {\r\n" );

   stream.write( buffer.length(), buffer.data() );
   
   // Write our object fields.
   
   writeFields( stream, tabStop + 1 );
   
   // Close our object definition.

   stream.writeTabs( tabStop );
   stream.write( 4, "};\r\n" );
}

//-----------------------------------------------------------------------------

void SimPersistSet::resolvePIDs()
{
   if( mIsResolvingPIDs )
      return;

   lock();
   mIsResolvingPIDs = true;

   for( U32 i = 0; i < mUnresolvedPIDs.size(); ++ i )
      if( mUnresolvedPIDs[ i ]->getObject() != NULL )
      {
         addObject( mUnresolvedPIDs[ i ]->getObject() );
         mUnresolvedPIDs[i]->decRefCount();
         mUnresolvedPIDs.erase( i );
         -- i;
      }

   mIsResolvingPIDs = false;
   unlock();
}

//-----------------------------------------------------------------------------

void SimPersistSet::addObject( SimObject* object )
{
   // If this set isn't transient, make sure the object has a valid
   // persistent ID.
   
   if( getCanSave() )
      object->getOrCreatePersistentId();
      
   Parent::addObject( object );
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

ConsoleMethod( SimPersistSet, resolvePersistentIds, void, 2, 2, "() - Try to bind unresolved persistent IDs in the set." )
{
   object->resolvePIDs();
}
