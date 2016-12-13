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

#ifndef _PHYSICS_PHYSICSUSERDATA_H_
#define _PHYSICS_PHYSICSUSERDATA_H_

#ifndef _SIGNAL_H_
#include "core/util/tSignal.h"
#endif

class PhysicsUserData;
class SceneObject;
class Point3F;
class PhysicsBody;


/// Signal used for contact reports.
///
/// @param us The physics user data for the signaling object.
/// @param them The other physics user data involved in the contact.
/// @param hitPoint The approximate position of the impact.
/// @param hitForce
///
/// @see PhysicsUserData
///
typedef Signal<void( PhysicsUserData *us,
                     PhysicsUserData *them,
                     const Point3F &hitPoint,
                     const Point3F &hitForce )> PhysicsContactSignal;


/// The base class for physics user data.
class PhysicsUserData
{
public:

   /// The constructor.
   PhysicsUserData()
      :
      #ifdef TORQUE_DEBUG
          mTypeId( smTypeName ),
      #endif
         mObject( NULL ),
         mBody( NULL )
      {}

   /// The destructor.
   virtual ~PhysicsUserData() {}

   ///
   void setObject( SceneObject *object ) { mObject = object; }
   SceneObject* getObject() const { return mObject; }

   void setBody( PhysicsBody *body ) { mBody = body; }
   PhysicsBody* getBody() const { return mBody; }

   /// Helper method for casting a void pointer to a userdata pointer.
   static inline SceneObject* getObject( void *data )
   {
      PhysicsUserData *result = cast( data );
      return result ? result->getObject() : NULL;
   }

   PhysicsContactSignal& getContactSignal() { return mContactSignal; }

   /// Helper method for casting a void pointer to a userdata pointer.
   static inline PhysicsUserData* cast( void *data )
   {
      PhysicsUserData *result = (PhysicsUserData*)data;
      
      // If the typeid doesn't equal the value we assigned to it at
      // construction then this isn't a PhysicsUserData object.
      #ifdef TORQUE_DEBUG
      AssertFatal( !result || result->mTypeId == smTypeName,
          "PhysicsUserData::cast - The pointer is the wrong type!" );
      #endif

      return result;
   }

protected:

   #ifdef TORQUE_DEBUG

   /// The type string used to validate the void* cast.
   /// @see cast
   static const char *smTypeName;

   /// The type string assigned at construction used to
   /// validate the void* cast.
   /// @see cast
   const char *mTypeId;
   #endif

   PhysicsContactSignal mContactSignal;

   SceneObject *mObject;

   PhysicsBody *mBody;
};

#endif // _PHYSICS_PHYSICSUSERDATA_H_   
